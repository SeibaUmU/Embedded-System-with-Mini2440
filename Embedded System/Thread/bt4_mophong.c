#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>

static volatile int run = 1;
static int shared_delay_ms = 100; 
static int sim_adc_val = 500; // Giá trị ADC giả lập ban đầu
pthread_mutex_t lock;
int leds[4] = {0, 0, 0, 0};

int get_delay_from_adc(int adc_val) {
    if (adc_val <= 100) return 100;
    if (adc_val <= 1023) return ((adc_val / 100) + 1) * 100;
    return 100;
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

void print_ui() {
    printf("\r[ADC:%4d -> Delay:%4dms] LED: %c %c %c %c  (a: Giam ADC, d: Tang ADC, q: Thoat) ", 
            sim_adc_val, shared_delay_ms, 
            leds[0]?'*':'_', leds[1]?'*':'_', leds[2]?'*':'_', leds[3]?'*':'_');
    fflush(stdout);
}

void* adc_thread(void* arg) {
    while(run) {
        char ch = get_keypress();
        if (ch == 'q') run = 0;
        else if (ch == 'a' && sim_adc_val > 0) sim_adc_val -= 50;
        else if (ch == 'd' && sim_adc_val < 1023) sim_adc_val += 50;

        if (sim_adc_val < 0) sim_adc_val = 0;
        if (sim_adc_val > 1023) sim_adc_val = 1023;

        pthread_mutex_lock(&lock);
        shared_delay_ms = get_delay_from_adc(sim_adc_val);
        pthread_mutex_unlock(&lock);
        
        usleep(50000);
    }
    return NULL;
}

int main() {
    pthread_t tid;
    pthread_mutex_init(&lock, NULL);
    pthread_create(&tid, NULL, adc_thread, NULL);

    printf("\n--- BAI 4: LED SANG DAN THEO ADC SIMULATOR ---\n");

    while(run) {
        for(int i = 0; i < 4; i++) leds[i] = 0; // Tắt tất cả
        print_ui();

        for (int led_no = 0; led_no < 4; led_no++) {
            if (!run) break;
            
            pthread_mutex_lock(&lock);
            int current_delay = shared_delay_ms;
            pthread_mutex_unlock(&lock);

            leds[led_no] = 1; // Bật LED
            print_ui();
            
            usleep(current_delay * 1000); 
        }
    }

    printf("\n[INFO] Da tat.\n");
    pthread_mutex_destroy(&lock);
    return 0;
}