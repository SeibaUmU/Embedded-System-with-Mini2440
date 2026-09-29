#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>

#define ON 1
#define OFF 0
#define PWM_IOCTL_SET_FREQ 1
#define PWM_IOCTL_STOP 0

// Hàm lấy thời gian hiện tại (tính bằng mili-giây)
unsigned long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

// Hàm tắt toàn bộ LED
void turn_off_all_leds(int led_fd) {
    for (int i = 0; i < 4; i++) {
        ioctl(led_fd, OFF, i);
    }
}

int main(int argc, char *argv[]) {
    int btn_fd = open("/dev/buttons", 0);
    int led_fd = open("/dev/leds", 0);
    int pwm_fd = open("/dev/pwm", 0);

    if (btn_fd < 0 || led_fd < 0 || pwm_fd < 0) {
        perror("Loi: Khong the mo thiet bi");
        exit(1);
    }

    char buttons_prev[4] = {'0', '0', '0', '0'};
    
    int led_mode = 1;        
    int delay_idx = 0;       
    int delay_times[3] = {200, 500, 800}; 

    int is_running = 0;      
    int led_step = 0;        
    int is_ton = 0;          
    
    unsigned long last_toggle_time = 0; 

    turn_off_all_leds(led_fd);
    ioctl(pwm_fd, PWM_IOCTL_STOP);
    
    printf("\n=== HE THONG KHOI TAO THANH CONG ===\n");
    printf("Mac dinh - Kieu LED: %d | Thoi gian delay: %dms\n", led_mode, delay_times[delay_idx]);
    printf("Bam K3 de BAT/TAT hien thi.\n\n");

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(btn_fd, &readfds);
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000; // 10ms

        int ret = select(btn_fd + 1, &readfds, NULL, NULL, &timeout);
        
        // --- XỬ LÝ NÚT NHẤN ---
        if (ret > 0 && FD_ISSET(btn_fd, &readfds)) {
            char current_buttons[4];
            if (read(btn_fd, current_buttons, sizeof(current_buttons)) == 4) {
                for (int i = 0; i < 4; i++) {
                    if (current_buttons[i] == '1' && buttons_prev[i] == '0') {
                        switch (i) {
                            case 0: // K1
                                led_mode++;
                                if (led_mode > 3) led_mode = 1;
                                led_step = 0; 
                                printf("[K1] Da chuyen Kieu LED: %d | Delay: %dms\n", led_mode, delay_times[delay_idx]);
                                break;
                                
                            case 1: // K2
                                delay_idx++;
                                if (delay_idx > 2) delay_idx = 0;
                                printf("[K2] Da chuyen Delay: %dms | Kieu LED: %d\n", delay_times[delay_idx], led_mode);
                                break;
                                
                            case 2: // K3
                                is_running = !is_running; 
                                if (is_running) {
                                    printf("[K3] BAT hien thi (Mode %d - %dms)\n", led_mode, delay_times[delay_idx]);
                                    last_toggle_time = get_time_ms();
                                    is_ton = 1; 
                                    led_step = -1; // Để lần vào Ton đầu tiên step sẽ tăng lên 0
                                } else {
                                    printf("[K3] TAT hien thi\n");
                                    turn_off_all_leds(led_fd);
                                    ioctl(pwm_fd, PWM_IOCTL_STOP);
                                }
                                break;
                                
                            case 3: // K4
                                turn_off_all_leds(led_fd);
                                ioctl(pwm_fd, PWM_IOCTL_STOP);
                                close(btn_fd);
                                close(led_fd);
                                close(pwm_fd);
                                printf("\nDevice closed: successful\n");
                                exit(0);
                        }
                    }
                    buttons_prev[i] = current_buttons[i]; 
                }
            }
        }

        // --- XỬ LÝ LED VÀ PWM THEO THỜI GIAN ---
        if (is_running) {
            unsigned long now = get_time_ms();
            int half_delay = delay_times[delay_idx] / 2; // TOn và TOff bằng một nửa thời gian delay
            
            if (now - last_toggle_time >= half_delay) {
                last_toggle_time = now; 

                if (is_ton) {
                    // --- Đang là pha TON: Đổi sang TOFF ---
                    is_ton = 0;
                    
                    // CHỈ TẮT ÂM THANH THEO ĐÚNG YÊU CẦU ĐỀ BÀI, KHÔNG TẮT LED
                    ioctl(pwm_fd, PWM_IOCTL_STOP);  
                    
                } else {
                    // --- Đang là pha TOFF: Đổi sang TON ---
                    is_ton = 1;
                    ioctl(pwm_fd, PWM_IOCTL_SET_FREQ, 1000); // Phát PWM 1000Hz
                    
                    led_step++; // Tăng bước để đổi trạng thái LED
                    
                    turn_off_all_leds(led_fd); // Xoá nền để vẽ trạng thái LED mới
                    
                    if (led_mode == 1) {        // Kiểu 1: Sáng đuổi
                        ioctl(led_fd, ON, led_step % 4);
                    } 
                    else if (led_mode == 2) {   // Kiểu 2: Chớp tắt
                        // Sáng ở chu kỳ chẵn, tắt ở chu kỳ lẻ
                        if (led_step % 2 == 0) {
                            for (int j = 0; j < 4; j++) ioctl(led_fd, ON, j);
                        }
                    } 
                    else if (led_mode == 3) {   // Kiểu 3: Sáng dần
                        for (int j = 0; j <= (led_step % 4); j++) {
                            ioctl(led_fd, ON, j);
                        }
                    }
                }
            }
        }
    }

    return 0;
}