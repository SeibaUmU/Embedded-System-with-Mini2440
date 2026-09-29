#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define ON 1
#define OFF 0

int fd;
int mode = 3; // 1: Chạy đuổi, 2: Chớp tắt, 3: STOP (giữ nguyên)

// --- LUỒNG CHUYÊN ĐIỀU KHIỂN LED ---
void* led_thread(void* arg) {
    int i;
    while (1) {
        if (mode == 1) { // Hiệu ứng chạy đuổi
            for (i = 0; i < 4; i++) {
                if (mode != 1) break; // Thoát ngay nếu đổi mode
                ioctl(fd, ON, i);
                usleep(50000);
                ioctl(fd, OFF, i);
            }
        }
        else if (mode == 2) { // Hiệu ứng chớp tắt
            for (i = 0; i < 4; i++) ioctl(fd, ON, i);
            usleep(200000);
            if (mode != 2) continue;
            for (i = 0; i < 4; i++) ioctl(fd, OFF, i);
            usleep(200000);
        }
        else if (mode == 3) {
            // STOP: Không làm gì cả, không có lệnh OFF nên LED giữ nguyên trạng thái
            usleep(100000);
        }
    }
    return NULL;
}

int main() {
    pthread_t tid;
    fd = open("/dev/leds", 0);
    if (fd < 0) { perror("open"); exit(1); }

    // Tạo luồng LED chạy song song với main
    pthread_create(&tid, NULL, led_thread, NULL); // Luồng này sẽ liên tục kiểm tra mode và điều khiển LED

    while (1) {
        printf("\n--- MENU DA LUONG ---\n");
        printf("1: Chay lien tuc, 2: Chop tat, 3: STOP tai cho, q: Thoat\n");
        printf("Nhap: ");

        char c = getchar();
        while(getchar() != '\n'); // Xóa bộ nhớ đệm

        if (c == 'q') break;
        if (c == '1') mode = 1;
        else if (c == '2') mode = 2;
        else if (c == '3') mode = 3; // Nhấn 3 là biến mode đổi, luồng kia sẽ ngừng tác động vào LED
    }

    close(fd);
    return 0;
}