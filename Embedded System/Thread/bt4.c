#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#define ON 1
#define OFF 0

/* Biến toàn cục dùng chung */
static int shared_delay_ms = 100; // Mặc định 100ms
static int led_fd, adc_fd;
pthread_mutex_t lock;

// Hàm quy đổi giá trị ADC ra thời gian delay (ms) theo đúng bảng trong sách
int get_delay_from_adc(int adc_val) {
    if (adc_val >= 0 && adc_val <= 100) return 100;
    if (adc_val >= 101 && adc_val <= 200) return 200;
    if (adc_val >= 201 && adc_val <= 300) return 300;
    if (adc_val >= 301 && adc_val <= 400) return 400;
    if (adc_val >= 401 && adc_val <= 500) return 500;
    if (adc_val >= 501 && adc_val <= 600) return 600;
    if (adc_val >= 601 && adc_val <= 700) return 700;
    if (adc_val >= 701 && adc_val <= 800) return 800;
    if (adc_val >= 801 && adc_val <= 900) return 900;
    if (adc_val >= 901 && adc_val <= 1000) return 1000;
    if (adc_val >= 1001 && adc_val <= 1023) return 1100;
    
    return 100; // Giá trị an toàn mặc định
}

// --------------------------------------------------------
// LUỒNG PHỤ: Đọc ADC và cập nhật thời gian Delay
// --------------------------------------------------------
void* adc_thread(void* arg) {
    char buffer[30];
    int len;
    int adc_value;

    while(1) {
        len = read(adc_fd, buffer, sizeof(buffer) - 1);
        
        if (len > 0) {
            buffer[len] = '\0';
            adc_value = -1;
            sscanf(buffer, "%d", &adc_value);
            
            if (adc_value >= 0) {
                // Lấy thời gian delay từ hàm quy đổi
                int new_delay = get_delay_from_adc(adc_value);
                
                // Khóa Mutex để cập nhật biến toàn cục
                pthread_mutex_lock(&lock);
                if (shared_delay_ms != new_delay) {
                    shared_delay_ms = new_delay;
                    printf("ADC: %d -> Cap nhat Delay: %d ms\n", adc_value, shared_delay_ms);
                }
                pthread_mutex_unlock(&lock);
            }
        }
        usleep(50 * 1000); // Ngủ 50ms giảm tải CPU
    }
    return NULL;
}

// --------------------------------------------------------
// LUỒNG CHÍNH: Điều khiển LED chạy
// --------------------------------------------------------
int main() {
    pthread_t tid;
    int current_delay;
    int i, led_no;

    // 1. Mở thiết bị
    led_fd = open("/dev/leds", 0);
    adc_fd = open("/dev/adc", 0);

    if (led_fd < 0 || adc_fd < 0) {
        perror("Loi mo thiet bi LED hoac ADC");
        exit(1);
    }

    // 2. Khởi tạo Mutex và Thread đọc ADC
    pthread_mutex_init(&lock, NULL);
    if (pthread_create(&tid, NULL, adc_thread, NULL) != 0) {
        perror("Loi tao thread ADC");
        return 1;
    }

    printf("Chuong trinh LED hien thi sang dan theo ADC bat dau...\n");

    // 3. Vòng lặp chính điều khiển LED
    while(1) {
        // Tắt tất cả LED khi bắt đầu một chu kỳ mới
        for(i = 0; i < 4; i++) {
            ioctl(led_fd, OFF, i);
        }

        // Bật tuần tự từng LED từ 0 đến 3 (Hiệu ứng sáng dần)
        for (led_no = 0; led_no < 4; led_no++) {
            
            // Đọc thời gian delay hiện tại một cách an toàn
            pthread_mutex_lock(&lock);
            current_delay = shared_delay_ms;
            pthread_mutex_unlock(&lock);

            // Bật LED số 'led_no' (các LED trước đó vẫn đang sáng)
            ioctl(led_fd, ON, led_no);
            
            // Trễ một khoảng thời gian phụ thuộc vào ADC
            usleep(current_delay * 1000); 
        }
    }

    // Đóng thiết bị (dọn dẹp khi thoát)
    close(led_fd);
    close(adc_fd);
    pthread_mutex_destroy(&lock);

    return 0;
}