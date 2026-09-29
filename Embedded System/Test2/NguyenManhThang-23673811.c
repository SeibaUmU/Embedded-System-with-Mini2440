#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/time.h>

#define ON 1 
#define OFF 0
#define PWM_IOCTL_SET_FREQ 1
#define PWM_IOCTL_STOP 0

const int freqs[7] = {262, 294, 330, 349, 392, 440, 494};
const int cycle_options[3] = {2, 3, 4};

// Khai báo biến toàn cục để 2 luồng cùng sử dụng
int btn_fd, led_fd, pwm_fd;
pthread_mutex_t lock;

int note_idx = -1;  
int cycle_idx = -1; 
int play_mode = 0;  
int is_ton = 0;     
int current_cycle = 0;
int target_cycles = 0;
unsigned long last_toggle_time = 0;

unsigned long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void turn_off_all_leds() {
    for (int i = 0; i < 4; i++) ioctl(led_fd, OFF, i);
}

void set_binary_led(int val) {
    for (int i = 0; i < 4; i++) {
        if ((val >> i) & 1) ioctl(led_fd, ON, i);
        else ioctl(led_fd, OFF, i);
    }
}

// ---------------------------------------------------------
// LUỒNG 1: Chuyên đọc sự kiện Nút nhấn (Dùng vòng lặp quét)
// ---------------------------------------------------------
void* button_thread(void* arg) {
    char cur_btn[4];
    char old_btn[4] = {'0', '0', '0', '0'}; // Trạng thái mặc định: 1 (Nhả)
    
    while(1) {
        // Đọc 4 byte trạng thái của 4 nút K1-K4
        if (read(btn_fd, cur_btn, sizeof(cur_btn)) == sizeof(cur_btn)) {
            for (int i = 0; i < 4; i++) {
                // Nếu có sự thay đổi trạng thái
                if (old_btn[i] != cur_btn[i]) {
                    usleep(50000); // Ngủ 50ms để chống dội phím (Debounce)
                    old_btn[i] = cur_btn[i];
                    
                    // Bắt sự kiện NHẤN (Tín hiệu kéo xuống 0)
                    if (cur_btn[i] == '1') { 
                        pthread_mutex_lock(&lock); // Khóa Mutex để cập nhật biến
                        
                        switch (i) {
                            case 0: // K1: Chọn nốt
                                note_idx++; if (note_idx > 6) note_idx = 0;
                                set_binary_led(note_idx + 1); 
                                
                                play_mode = 1;      
                                target_cycles = 1;  // Ép K1 chỉ chạy 1 chu kỳ
                                current_cycle = 0;
                                is_ton = 1;
                                last_toggle_time = get_time_ms();
                                ioctl(pwm_fd, PWM_IOCTL_SET_FREQ, freqs[note_idx]);
                                printf("[K1] Not: %d (Freq: %d Hz)\n", note_idx + 1, freqs[note_idx]);
                                break;
                                
                            case 1: // K2: Chọn chu kỳ
                                cycle_idx++; if (cycle_idx > 2) cycle_idx = 0;
                                set_binary_led(cycle_idx + 1); 
                                play_mode = 0;
                                ioctl(pwm_fd, PWM_IOCTL_STOP);
                                printf("[K2] Chu ky: %d\n", cycle_options[cycle_idx]);
                                break;
                                
                            case 2: // K3: Phát chuỗi PWM
                                if (note_idx == -1) note_idx = 0;   
                                if (cycle_idx == -1) cycle_idx = 0; 
                                play_mode = 3;      
                                target_cycles = cycle_options[cycle_idx];
                                current_cycle = 0;
                                is_ton = 1;
                                last_toggle_time = get_time_ms();
                                ioctl(pwm_fd, PWM_IOCTL_SET_FREQ, freqs[note_idx]);
                                printf("[K3] Phat Not %d | %d Chu ky\n", note_idx + 1, target_cycles);
                                break;
                                
                            case 3: // K4: Tắt toàn bộ
                                turn_off_all_leds();
                                ioctl(pwm_fd, PWM_IOCTL_STOP);
                                printf("\n[K4] Da dong thiet bi. Thoat chuong trinh.\n");
                                exit(0);
                        }
                        pthread_mutex_unlock(&lock); // Mở khóa Mutex
                    }
                }
            }
        }
        usleep(20000); // Luồng quét nút nhấn hoạt động mỗi 20ms
    }
    return NULL;
}

// ---------------------------------------------------------
// LUỒNG CHÍNH (MAIN): Điều khiển nhịp điệu (Ton / Toff)
// ---------------------------------------------------------
int main() {
    btn_fd = open("/dev/buttons", 0);
    led_fd = open("/dev/leds", 0);
    pwm_fd = open("/dev/pwm", 0);

    if (btn_fd < 0 || led_fd < 0 || pwm_fd < 0) {
        perror("Loi mo thiet bi"); 
        exit(1);
    }

    // Khởi tạo Mutex và Luồng nút nhấn
    pthread_mutex_init(&lock, NULL);
    pthread_t btn_tid;
    pthread_create(&btn_tid, NULL, button_thread, NULL);

    turn_off_all_leds();
    ioctl(pwm_fd, PWM_IOCTL_STOP);

    printf("\n=== CHUONG TRINH DIEU KHIEN ===\n");

    // Vòng lặp định thời cho PWM
    while (1) {
        pthread_mutex_lock(&lock);
        
        if (play_mode != 0) {
            unsigned long now = get_time_ms();
            unsigned long wait_time = (play_mode == 1) ? 200 : 300;
            
            if (now - last_toggle_time >= wait_time) {
                last_toggle_time = now; 

                if (is_ton) {
                    is_ton = 0; // Đổi sang pha nghỉ
                    ioctl(pwm_fd, PWM_IOCTL_STOP);  
                } else {
                    current_cycle++;
                    // K3 thì kiểm tra giới hạn chu kỳ, K1 thì lặp liên tục để nghe thử
                    // Cứ đủ chu kỳ yêu cầu (1 chu kỳ đối với K1, hoặc N chu kỳ đối với K3) là ngắt
                    if (current_cycle >= target_cycles) {
                        play_mode = 0; 
                    } else {
                        is_ton = 1; // Đổi sang pha phát tiếng
                        ioctl(pwm_fd, PWM_IOCTL_SET_FREQ, freqs[note_idx]);
                    }
                }
            }
        }
        
        pthread_mutex_unlock(&lock);
        usleep(10000); // Nghỉ 10ms để giảm tải CPU
    }

    return 0;
}