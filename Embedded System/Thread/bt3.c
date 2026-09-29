#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#define PWM_IOCTL_SET_FREQ 1
#define PWM_IOCTL_STOP 0

static volatile int run = 1;
static int shared_adc_value = 0;
static int pwm_fd, adc_fd;
pthread_mutex_t lock;

// --- Hàm phụ trợ ---
void set_buzzer_freq(int fd, int f) {
    ioctl(fd, PWM_IOCTL_SET_FREQ, f);
}
void stop_buzzer(int fd) {
    ioctl(fd, PWM_IOCTL_STOP); close(fd);
    printf("\n[INFO] Da tat PWM va dong ADC.\n");
}

// --- Luồng nhận phím 'q' ---
void* input_thread(void* arg) {
    char ch;
    while (run) {
        scanf(" %c", &ch);
        if (ch == 'q' || ch == 'Q') run = 0;
    }
    return NULL;
}

// --- Luồng đọc dữ liệu từ phần cứng ADC ---
void* adc_thread(void* arg) {
    char buffer[30];
    int len, value;

    while(run) {
        len = read(adc_fd, buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            value = -1;
            sscanf(buffer, "%d", &value);
            
            if (value >= 0) {
                pthread_mutex_lock(&lock);
                shared_adc_value = value;
                pthread_mutex_unlock(&lock);
            }
        }
        usleep(50000); // 50ms đọc 1 lần
    }
    return NULL;
}

int main() {
    pthread_t adc_tid, in_tid;
    int current_pwm_freq = -1;

    pwm_fd = open("/dev/pwm", 0);
    adc_fd = open("/dev/adc", 0);
    if (pwm_fd < 0 || adc_fd < 0) {
        perror("Loi mo thiet bi"); exit(1);
    }

    pthread_mutex_init(&lock, NULL);
    pthread_create(&adc_tid, NULL, adc_thread, NULL);
    pthread_create(&in_tid, NULL, input_thread, NULL);

    printf("\n--- BAI 3: ADC -> PWM (Nhan 'q' de thoat) ---\n");

    while (run) {
        pthread_mutex_lock(&lock);
        int latest_adc = shared_adc_value;
        pthread_mutex_unlock(&lock);

        // Chỉ điều chỉnh tần số nếu có sự thay đổi từ tay vặn biến trở
        if (latest_adc > 0 && latest_adc != current_pwm_freq) {
            set_buzzer_freq(pwm_fd, latest_adc);
            printf("\rGia tri ADC / Tan so PWM: %d Hz       ", latest_adc);
            fflush(stdout);
            current_pwm_freq = latest_adc;
        }

        usleep(20000); // Quét luồng chính mỗi 20ms
    }

    stop_buzzer(pwm_fd);
    close(adc_fd);
    pthread_mutex_destroy(&lock);
    return 0;
}