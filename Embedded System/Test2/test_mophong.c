#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <termios.h>

// Hàm lấy thời gian hiện tại (ms)
unsigned long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

// Hàm đọc phím không chặn (Non-blocking key read)
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

int main() {
    int leds[4] = {0, 0, 0, 0};
    int led_mode = 1;        
    int delay_idx = 0;       
    int delay_times[3] = {200, 500, 800}; 

    int is_running = 0;      
    int led_step = 0;        
    int is_ton = 0;          
    unsigned long last_toggle_time = 0; 
    unsigned long last_wave_shift_time = 0; // Biến đếm thời gian trôi sóng
    int state_changed = 1; 

    // Chuỗi chứa 4 chu kỳ PWM (để làm bộ đệm dịch vòng)
    // 1 chu kỳ: "----____" (8 ký tự)
    const char base_wave[] = "----____----____----____----____"; 
    int wave_offset = 0;

    printf("\n=== HE THONG KHOI TAO (PHIM 1,2,3,4) ===\n");
    printf("Phim: [1:Mode] [2:Delay] [3:Start/Stop] [4:Thoat]\n");

    while (1) {
        char ch = get_keypress();
        if (ch != 0) {
            state_changed = 1;
            switch (ch) {
                case '1': led_mode++; if (led_mode > 3) led_mode = 1; led_step = 0; break;
                case '2': delay_idx++; if (delay_idx > 2) delay_idx = 0; break;
                case '3': 
                    is_running = !is_running; 
                    if (is_running) { last_toggle_time = get_time_ms(); is_ton = 1; led_step = -1; wave_offset = 0; }
                    else { for(int i=0; i<4; i++) leds[i] = 0; is_ton = 0; }
                    break;
                case '4': case 'q': printf("\n[INFO] Successful Exit.\n"); exit(0);
            }
        }

        unsigned long now = get_time_ms();

        // LOGIC 1: ĐIỀU KHIỂN CHỚP TẮT LED (Giữ nguyên)
        if (is_running) {
            int half_delay = delay_times[delay_idx] / 2;
            if (now - last_toggle_time >= half_delay) {
                last_toggle_time = now; 
                state_changed = 1; 

                if (is_ton) is_ton = 0; 
                else {
                    is_ton = 1; 
                    led_step++; 
                    for(int j=0; j<4; j++) leds[j] = 0;
                    if (led_mode == 1) leds[led_step % 4] = 1;
                    else if (led_mode == 2 && (led_step % 2 == 0)) for (int j=0; j<4; j++) leds[j] = 1;
                    else if (led_mode == 3) for (int j=0; j <= (led_step % 4); j++) leds[j] = 1;
                }
            }

            // LOGIC 2: ĐIỀU KHIỂN SÓNG CHẠY (Mỗi 50ms dịch sóng 1 lần)
            if (is_ton && (now - last_wave_shift_time >= 50)) {
                last_wave_shift_time = now;
                wave_offset = (wave_offset + 1) % 16; // Tăng offset từ 0 đến 15 (độ dài 2 chu kỳ)
                state_changed = 1; // Yêu cầu in lại màn hình
            }
        }

        // IN RA MÀN HÌNH
        if (state_changed) {
            char current_wave[17] = "                "; // Mặc định là chuỗi rỗng
            
            // Nếu hệ thống đang chạy và đang ở pha PWM ON, ta copy 16 ký tự từ base_wave
            if (is_running && is_ton) {
                snprintf(current_wave, sizeof(current_wave), "%.16s", base_wave + wave_offset);
            }

            printf("\r[M:%d | %3dms | %-3s] LED: %c %c %c %c | PWM: %s    ", 
                   led_mode, delay_times[delay_idx], 
                   is_running ? "RUN" : "OFF",
                   leds[0]?'*':'_', leds[1]?'*':'_', leds[2]?'*':'_', leds[3]?'*':'_',
                   current_wave);
            fflush(stdout);
            state_changed = 0;
        }

        usleep(10000); // 10ms loop
    }
    return 0;
}