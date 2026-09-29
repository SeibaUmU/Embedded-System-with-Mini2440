#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define PWM_IOCTL_SET_FREQ 1
#define PWM_IOCTL_STOP 0

static volatile int run = 1; // Cờ điều khiển chạy/dừng
static int freq = 1000;
static int pwm_fd, btn_fd;
pthread_mutex_t lock;

// --- Hàm phụ trợ ---
void set_buzzer_freq(int fd, int f) {
    ioctl(fd, PWM_IOCTL_SET_FREQ, f);
    printf("\rDang phat tan so: %d Hz (Nhan 'q' roi Enter de thoat)   ", f);
    fflush(stdout);
}

void stop_buzzer(int fd) {
    ioctl(fd, PWM_IOCTL_STOP);
    close(fd);
    printf("\n[INFO] Da tat buzzer va dong thiet bi PWM.\n");
}

// --- Luồng nhận phím 'q' ---
void* input_thread(void* arg) {
    char ch;
    while (run) {
        scanf(" %c", &ch);
        if (ch == 'q' || ch == 'Q') {
            run = 0; // Kéo cờ xuống để dừng mọi luồng
        }
    }
    return NULL;
}

// --- Luồng đọc nút nhấn ---
void* button_thread(void* arg) {
    char cur_btn[2];
    char old_btn[2] = {'1', '1'}; // '1' là nhả
    
    while(run) {
        if (read(btn_fd, cur_btn, sizeof(cur_btn)) == sizeof(cur_btn)) {
            // Nút K1 (Tăng)
            if (old_btn[0] != cur_btn[0]) {
                usleep(200000); 
                old_btn[0] = cur_btn[0];
                if (cur_btn[0] == '0') {
                    pthread_mutex_lock(&lock);
                    freq += 50;
                    if (freq > 3000) freq = 3000;
                    int f = freq;
                    pthread_mutex_unlock(&lock);
                    set_buzzer_freq(pwm_fd, f);
                }
            }
            // Nút K2 (Giảm)
            if (old_btn[1] != cur_btn[1]) {
                usleep(200000); 
                old_btn[1] = cur_btn[1];
                if (cur_btn[1] == '0') {
                    pthread_mutex_lock(&lock);
                    freq -= 50;
                    if (freq < 50) freq = 50;
                    int f = freq;
                    pthread_mutex_unlock(&lock);
                    set_buzzer_freq(pwm_fd, f);
                }
            }
        }
        usleep(20000); // 20ms
    }
    return NULL;
}

int main() {
    pthread_t btn_tid, in_tid;

    pwm_fd = open("/dev/pwm", 0);
    btn_fd = open("/dev/buttons", 0);
    if (pwm_fd < 0 || btn_fd < 0) {
        perror("Loi mo thiet bi"); exit(1);
    }

    pthread_mutex_init(&lock, NULL);
    
    printf("\n--- BAI 1: PWM K1/K2 (Nhan 'q' de thoat) ---\n");
    set_buzzer_freq(pwm_fd, freq);

    pthread_create(&btn_tid, NULL, button_thread, NULL);
    pthread_create(&in_tid, NULL, input_thread, NULL);

    // Luồng chính chỉ cần đứng chờ cho đến khi run == 0
    while(run) {
        usleep(100000); // Ngủ 100ms liên tục
    }

    // Dọn dẹp
    stop_buzzer(pwm_fd);
    close(btn_fd);
    pthread_mutex_destroy(&lock);
    return 0;
}