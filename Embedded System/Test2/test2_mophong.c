#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <sys/time.h>

// --- BIẾN TOÀN CỤC & MUTEX ---
pthread_mutex_t lock;
static volatile int run_flag = 1;
static volatile int k4_pressed = 0;

int t = 1000;                // Thời gian thay đổi trạng thái (ms)
int led_mode = 0;            // 0: Sáng dần, 1: Sáng nhị phân
int total_presses = 0;       // Tổng số lần nhấn K1, K2, K3
int led_step = 0;            // Biến chạy các bước của LED

const char* filename = "xuat_mophong.txt";

// --- HÀM THỜI GIAN VÀ BÀN PHÍM ---
unsigned long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

char get_keypress() {
    char buf = 0;
    struct termios old = {0};
    tcgetattr(0, &old);
    old.c_lflag &= ~ICANON; old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 0; old.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &old);
    read(0, &buf, 1);
    old.c_lflag |= ICANON; old.c_lflag |= ECHO;
    tcsetattr(0, TCSADRAIN, &old);
    return buf;
}

// --- HÀM LƯU FILE ---
void update_file() {
    FILE *f = fopen(filename, "w");
    if (f != NULL) {
        fprintf(f, "Tong so lan nhan nut K1,K2,K3: %d\n", total_presses);
        fprintf(f, "Thoi gian t: %d\n", t);
        fprintf(f, "Kieu led: %s\n", led_mode == 0 ? "Sang dan" : "Nhi phan");
        fclose(f);
    }
}

// --- LUỒNG NHẬN INPUT (NÚT NHẤN) ---
void* input_thread(void* arg) {
    while(run_flag) {
        char ch = get_keypress();
        if (ch != 0) {
            pthread_mutex_lock(&lock);
            
            if (ch == '1') { // Nút K1
                led_mode = !led_mode;
                led_step = 0; // Reset lại vòng lặp LED khi chuyển mode
                total_presses++;
                update_file();
            } 
            else if (ch == '2') { // Nút K2
                if (t < 2500) t += 100;
                total_presses++;
                update_file();
            } 
            else if (ch == '3') { // Nút K3
                if (t > 100) t -= 100;
                total_presses++;
                update_file();
            } 
            else if (ch == '4') { // Nút K4
                run_flag = 0;
                k4_pressed = 1;
            }
            
            pthread_mutex_unlock(&lock);
        }
        usleep(20000); // Quét phím mỗi 20ms để tránh tốn CPU
    }
    return NULL;
}

// --- HÀM MAIN ---
int main() {
    pthread_t in_tid;
    pthread_mutex_init(&lock, NULL);
    
    // Khởi tạo file lúc bắt đầu
    update_file();

    printf("\n=== HE THONG MO PHONG NHUNG ===\n");
    printf("Phim map: [1] K1: Mode  |  [2] K2: Tăng t  |  [3] K3: Giảm t  |  [4] K4: Thoát\n\n");

    // Tạo luồng đọc phím
    pthread_create(&in_tid, NULL, input_thread, NULL);

    unsigned long last_time = get_time_ms();

    // Vòng lặp chính xử lý logic LED và màn hình
    while(run_flag) {
        unsigned long now = get_time_ms();
        
        pthread_mutex_lock(&lock);
        int current_t = t;
        int current_mode = led_mode;
        pthread_mutex_unlock(&lock);

        // Logic thay đổi trạng thái LED theo thời gian t
        if (now - last_time >= current_t) {
            last_time = now;
            pthread_mutex_lock(&lock);
            if (current_mode == 0) {
                // Mode sáng dần: 5 trạng thái (0 đến 4 bóng sáng)
                led_step = (led_step + 1) % 5;
            } else {
                // Mode nhị phân: 16 trạng thái (đếm từ 0 đến 15)
                led_step = (led_step + 1) % 16;
            }
            pthread_mutex_unlock(&lock);
        }

        // Tính toán trạng thái 4 LED và đếm số LED sáng
        int leds[4] = {0, 0, 0, 0};
        int lit_count = 0;
        
        if (current_mode == 0) {
            // Sáng dần từ trên xuống (trái qua phải)
            for (int i = 0; i < led_step; i++) {
                leds[i] = 1;
                lit_count++;
            }
        } else {
            // Sáng nhị phân (MSB bên trái, LSB bên phải)
            for (int i = 0; i < 4; i++) {
                leds[3-i] = (led_step >> i) & 1; 
                if (leds[3-i]) lit_count++;
            }
        }

        // Quyết định PWM dựa trên số chẵn/lẻ của LED sáng
        int pwm_freq = 0;
        char pwm_status[10] = "STOP";
        
        if (lit_count % 2 == 0) { // Chẵn LED sáng
            if (current_mode == 0) pwm_freq = 10000;      // f1
            else pwm_freq = 12000;                        // f2
            sprintf(pwm_status, "RUN ");
        }

        // In ra màn hình UI (sử dụng \r để ghi đè dòng[cite: 1, 3])
        printf("\r[%-9s] t=%4dms | LEDs: %c %c %c %c | PWM: %s (f=%5dHz) | Phim: %3d  ", 
               current_mode == 0 ? "Sang Dan" : "Nhi Phan", 
               current_t,
               leds[0]?'*':'_', leds[1]?'*':'_', leds[2]?'*':'_', leds[3]?'*':'_',
               pwm_status, pwm_freq, total_presses);
        fflush(stdout);

        usleep(15000); // Delay nhẹ cho vòng lặp hiển thị
    }

    // Đợi thread input thoát
    pthread_join(in_tid, NULL);

    // Logic xử lý khi nhấn K4
    if (k4_pressed) {
        printf("\n\nThe end\n");
        sleep(2); // Đợi 2 giây
        printf("Đã đóng tất cả các device. Chương trình kết thúc.\n");
    }

    pthread_mutex_destroy(&lock);
    return 0;
}