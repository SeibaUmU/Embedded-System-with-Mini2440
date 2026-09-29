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

// --- BIẾN TOÀN CỤC & MUTEX ---
int btn_fd, led_fd, pwm_fd; //biến lưu file descriptors cho /dev/buttons, /dev/leds, /dev/pwm
pthread_mutex_t lock; // Mutex để bảo vệ truy cập biến chung giữa các luồng

static volatile int run_flag = 1; // Cờ chạy chính, khi K4 được nhấn sẽ đặt = 0 để kết thúc chương trình
static volatile int k4_pressed = 0;

int t = 1000;                // Thời gian thay đổi trạng thái (ms)
int led_mode = 0;            // 0: Sáng dần, 1: Sáng nhị phân
int total_presses = 0;       // Tổng số lần nhấn K1, K2, K3
int led_step = 0;            // Bước hiện tại của LED (0-4 cho sáng dần, 0-15 cho nhị phân)

const char* filename = "xuat.txt";

// --- HÀM THỜI GIAN ---
unsigned long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
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

// --- HÀM ĐIỀU KHIỂN PHẦN CỨNG ---
void turn_off_all_devices() {
    for (int i = 0; i < 4; i++) {
        ioctl(led_fd, OFF, i); // Tắt LED[cite: 4]
    }
    ioctl(pwm_fd, PWM_IOCTL_STOP); // Dừng PWM[cite: 4, 5]
}

// --- LUỒNG ĐỌC NÚT NHẤN TỪ DEVICE ---
void* button_thread(void* arg) {
    char cur_btn[4];
    char old_btn[4] = {'1', '1', '1', '1'}; // Trạng thái mặc định: 1 (Nhả)[cite: 4, 5]
    
    while(run_flag) {
        // Đọc 4 byte trạng thái của 4 nút K1-K4[cite: 4]
        if (read(btn_fd, cur_btn, sizeof(cur_btn)) == sizeof(cur_btn)) { //trả về danh sách số byte button đã đọc được
            for (int i = 0; i < 4; i++) {
                if (old_btn[i] != cur_btn[i]) { // Phát hiện sự thay đổi trạng thái của nút[i]
                    usleep(50000); // Ngủ 50ms để chống dội phím (Debounce)[cite: 4]
                    old_btn[i] = cur_btn[i];
                    
                    // Bắt sự kiện NHẤN (Tín hiệu kéo xuống 0)[cite: 4, 5]
                    if (cur_btn[i] == '0') { 
                        pthread_mutex_lock(&lock); // Bảo vệ truy cập biến chung, tránh race condition
                        
                        if (i == 0) { // K1
                            led_mode = !led_mode; // Đổi chế độ LED
                            led_step = 0;  // Reset bước LED khi đổi chế độ
                            total_presses++; // Tăng số lần nhấn
                            update_file(); // Cập nhật file sau khi thay đổi trạng thái[cite: 4, 5]
                        } 
                        else if (i == 1) { // K2
                            if (t < 2500) t += 100;
                            total_presses++;
                            update_file();
                        } 
                        else if (i == 2) { // K3
                            if (t > 100) t -= 100;
                            total_presses++;
                            update_file();
                        } 
                        else if (i == 3) { // K4
                            run_flag = 0;
                            k4_pressed = 1;
                        }
                        
                        pthread_mutex_unlock(&lock);
                    }
                }
            }
        }
        usleep(20000); // Quét phím mỗi 20ms[cite: 4, 5]
    }
    return NULL;
}

// --- LUỒNG CHÍNH ---
int main() {
    // Mở các file thiết bị[cite: 4, 5]
    btn_fd = open("/dev/buttons", 0);
    led_fd = open("/dev/leds", 0);
    pwm_fd = open("/dev/pwm", 0);

    if (btn_fd < 0 || led_fd < 0 || pwm_fd < 0) {
        perror("Loi mo thiet bi phan cung"); 
        exit(1);
    }

    pthread_mutex_init(&lock, NULL); // Khởi tạo mutex, đảm bảo an toàn khi truy cập biến chung giữa các luồng
    
    // Đảm bảo phần cứng tắt hết lúc khởi động và ghi file
    turn_off_all_devices();
    update_file();

    printf("\n=== HE THONG NHUNG DANG CHAY ===\n");
    printf("Nhan K4 tren board de thoat chuong trinh...\n");

    // Tạo luồng đọc phần cứng
    pthread_t btn_tid; //biến lưu ID của luồng đọc nút nhấn
    pthread_create(&btn_tid, NULL, button_thread, NULL); //tạo luồng đọc nút nhấn, chạy hàm button_thread

    unsigned long last_time = get_time_ms(); // Biến lưu thời điểm lần cuối cùng thay đổi trạng thái LED, dùng để định thời[cite: 4]

    // Vòng lặp định thời cho LED và PWM
    while(run_flag) {
        unsigned long now = get_time_ms();
        
        pthread_mutex_lock(&lock); // khóa lại, để luồng chính có thể đọc giá trị t và led_mode một cách an toàn, tránh bị thay đổi giữa chừng bởi luồng button_thread
        int current_t = t; // Lấy giá trị t hiện tại, đảm bảo không bị thay đổi giữa chừng khi đang sử dụng
        int current_mode = led_mode;
        pthread_mutex_unlock(&lock);

        // Chuyển bước LED khi đủ thời gian t
        if (now - last_time >= current_t) { 
            last_time = now;
            pthread_mutex_lock(&lock);
            if (current_mode == 0) led_step = (led_step + 1) % 5; // Chế độ sáng dần có 5 bước (0-4)
            else led_step = (led_step + 1) % 16; // Chế độ nhị phân có 16 bước (0-15)
            pthread_mutex_unlock(&lock);
        }

        int leds[4] = {0, 0, 0, 0}; // Mảng trạng thái LED, 1 = sáng, 0 = tắt
        int lit_count = 0; // Biến đếm số lượng LED đang sáng, dùng để điều khiển PWM sau này
        
        // Tính toán trạng thái logic cho LED
        if (current_mode == 0) {
            for (int i = 0; i < led_step; i++) {
                leds[i] = 1; // Sáng dần: Bật LED từ trái sang phải theo bước hiện tại
                lit_count++;
            }
        } else {
            for (int i = 0; i < 4; i++) { //i = 0 đến 3 tương ứng với 4 LED
                leds[3-i] = (led_step >> i) & 1; // Nhị phân: Mỗi bit của led_step quyết định trạng thái của LED tương ứng, đảo ngược thứ tự để LED1 là bit thấp nhất
                if (leds[3-i]) lit_count++; // Đếm số lượng LED đang sáng để điều khiển PWM sau này
            }
        }

        // Xuất tín hiệu ra /dev/leds[cite: 4]
        for (int i = 0; i < 4; i++) {
            if (leds[i]) ioctl(led_fd, ON, i);
            else ioctl(led_fd, OFF, i);
        }

        // Điều khiển /dev/pwm dựa trên số chẵn/lẻ của LED sáng
        if (lit_count % 2 == 0) { // Số lượng LED sáng là chẵn
            int pwm_freq = (current_mode == 0) ? 10000 : 12000;
            ioctl(pwm_fd, PWM_IOCTL_SET_FREQ, pwm_freq);
        } else {
            ioctl(pwm_fd, PWM_IOCTL_STOP);
        }

        usleep(10000); // Ngủ 10ms để giảm tải CPU cho luồng chính[cite: 4]
    }

    // Luồng sẽ kết thúc khi biến run_flag = 0 (K4 được nhấn)
    pthread_join(btn_tid, NULL); // Chờ luồng đọc nút nhấn kết thúc trước khi tiếp tục, đảm bảo không có truy cập nào đến thiết bị sau khi đã đóng

    if (k4_pressed) {
        printf("\nThe end\n");
        sleep(2); // Đợi 2 giây
        turn_off_all_devices();
        
        // Đóng file descriptors[cite: 5]
        close(btn_fd);
        close(led_fd);
        close(pwm_fd);
    }

    pthread_mutex_destroy(&lock);
    return 0;
}

/*
2 luồng chạy song song:

Luồng Chính (main): Cứ đủ t mili giây thì đổi trạng thái LED và loa. Nó đọc biến t liên tục.

Luồng Nút: Cứ mỗi 20ms lại kiểm tra xem bạn có nhấn K1, K2, K3 không. Nếu bạn nhấn K2, nó sẽ khóa Mutex, tăng t lên, rồi mở khóa.
*/