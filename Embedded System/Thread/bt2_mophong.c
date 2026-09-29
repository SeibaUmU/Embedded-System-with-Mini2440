#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <termios.h>

static volatile int run = 1;
static int delay_time = 1000;
pthread_mutex_t lock;

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

void* button_thread(void* arg) {
    while(run) {
        char ch = get_keypress();
        if (ch == 'q' || ch == 'Q') run = 0;
        else if (ch == 'w') {
            pthread_mutex_lock(&lock);
            delay_time += 100;
            pthread_mutex_unlock(&lock);
        }
        else if (ch == 's') {
            pthread_mutex_lock(&lock);
            delay_time -= 100; if (delay_time < 100) delay_time = 100;
            pthread_mutex_unlock(&lock);
        }
        usleep(20000);
    }
    return NULL;
}

int main() {
    pthread_t btn_tid;
    int current_freq = 500;
    pthread_mutex_init(&lock, NULL);
    pthread_create(&btn_tid, NULL, button_thread, NULL);

    printf("\n--- BAI 2: PWM AUTO SCAN SIMULATOR ---\n");

    while (run) {
        pthread_mutex_lock(&lock);
        int current_delay = delay_time;
        pthread_mutex_unlock(&lock);
        
        printf("\r[Delay: %4dms] [PWM: (f=%4dHz) ----____----____ ]   ", current_delay, current_freq);
        fflush(stdout);

        int waited = 0;
        while (run && waited < current_delay) {
            usleep(10000);
            waited += 10;
        }

        if (!run) break;

        current_freq += 100;
        if (current_freq > 3000) current_freq = 500;
    }

    printf("\n[INFO] Da tat.\n");
    pthread_mutex_destroy(&lock);
    return 0;
}