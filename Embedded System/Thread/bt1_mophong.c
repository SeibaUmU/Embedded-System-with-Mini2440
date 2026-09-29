#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <termios.h>

static volatile int run = 1;
static int freq = 1000;
pthread_mutex_t lock;

// Hàm đọc phím không cần Enter
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

void print_ui() {
    printf("\r[PWM: (f=%4dHz) ----____----____ ] (w: Tang, s: Giam, q: Thoat)   ", freq);
    fflush(stdout);
}

void* input_thread(void* arg) {
    while(run) {
        char ch = get_keypress();
        if (ch == 'q' || ch == 'Q') run = 0;
        else if (ch == 'w') {
            pthread_mutex_lock(&lock);
            freq += 50; if (freq > 3000) freq = 3000;
            pthread_mutex_unlock(&lock);
            print_ui();
        }
        else if (ch == 's') {
            pthread_mutex_lock(&lock);
            freq -= 50; if (freq < 50) freq = 50;
            pthread_mutex_unlock(&lock);
            print_ui();
        }
        usleep(20000); // Quét phím mỗi 20ms
    }
    return NULL;
}

int main() {
    pthread_t in_tid;
    pthread_mutex_init(&lock, NULL);
    
    printf("\n--- BAI 1: PWM SIMULATOR ---\n");
    print_ui();

    pthread_create(&in_tid, NULL, input_thread, NULL);

    while(run) usleep(100000);

    printf("\n[INFO] Da tat.\n");
    pthread_mutex_destroy(&lock);
    return 0;
}