#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define PWM_IOCTL_SET_FREQ 1
#define PWM_IOCTL_STOP 0

static volatile int run = 1;
static int delay_time = 1000;
static int pwm_fd, btn_fd;
pthread_mutex_t lock;

// --- Hàm phụ trợ ---
void set_buzzer_freq(int fd, int f) {
    ioctl(fd, PWM_IOCTL_SET_FREQ, f);
}
void stop_buzzer(int fd) {
    ioctl(fd, PWM_IOCTL_STOP); close(fd);
    printf("\n[INFO] Da tat PWM.\n");
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

// --- Luồng nút nhấn (K1 tăng delay, K2 giảm delay) ---
void* button_thread(void* arg) {
    char cur_btn[2], old_btn[2] = {'1', '1'};
    while(run) {
        if (read(btn_fd, cur_btn, sizeof(cur_btn)) == sizeof(cur_btn)) {
            if (old_btn[0] != cur_btn[0]) { // K1
                usleep(200000); old_btn[0] = cur_btn[0];
                if (cur_btn[0] == '0') {
                    pthread_mutex_lock(&lock);
                    delay_time += 100;
                    pthread_mutex_unlock(&lock);
                }
            }
            if (old_btn[1] != cur_btn[1]) { // K2
                usleep(200000); old_btn[1] = cur_btn[1];
                if (cur_btn[1] == '0') {
                    pthread_mutex_lock(&lock);
                    delay_time -= 100;
                    if (delay_time < 100) delay_time = 100;
                    pthread_mutex_unlock(&lock);
                }
            }
        }
        usleep(20000);
    }
    return NULL;
}

int main() {
    pthread_t btn_tid, in_tid;
    int current_freq = 500;

    pwm_fd = open("/dev/pwm", 0); btn_fd = open("/dev/buttons", 0);
    if (pwm_fd < 0 || btn_fd < 0) { perror("Loi thiet bi"); exit(1); }

    pthread_mutex_init(&lock, NULL);
    pthread_create(&btn_tid, NULL, button_thread, NULL);
    pthread_create(&in_tid, NULL, input_thread, NULL);

    printf("\n--- BAI 2: PWM AUTO SCAN ---\n");

    while (run) {
        set_buzzer_freq(pwm_fd, current_freq);
        
        pthread_mutex_lock(&lock);
        int current_delay = delay_time;
        pthread_mutex_unlock(&lock);
        
        printf("\rFreq: %d Hz | Delay: %d ms | Nhan 'q' de thoat   ", current_freq, current_delay);
        fflush(stdout);

        // Kỹ thuật "Responsive Sleep": Chia nhỏ thời gian chờ để thoát nhanh khi có lệnh 'q'
        int waited = 0;
        while (run && waited < current_delay) {
            usleep(10000); // Ngủ từng giấc 10ms
            waited += 10;
        }

        if (!run) break; // Thoát ngay nếu 'q' được nhấn giữa chừng

        current_freq += 100;
        if (current_freq > 3000) current_freq = 500;
    }

    stop_buzzer(pwm_fd);
    close(btn_fd);
    pthread_mutex_destroy(&lock);
    return 0;
}