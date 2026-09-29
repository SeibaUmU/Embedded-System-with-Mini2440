#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PWM_IOCTL_SET_FREQ 1
#define PWM_IOCTL_STOP 0

// Khai báo biến toàn cục để lưu trạng thái terminal cũ
struct termios old_tio, new_tio;

// Hàm thiết lập Terminal để nhận phím ngay lập tức (không cần nhấn Enter)
void setup_terminal_for_piano() {
    tcgetattr(STDIN_FILENO, &old_tio); // Lưu lại cấu hình cũ
    new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO); // Tắt chế độ chờ Enter và tắt in ký tự ra màn hình
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio); // Áp dụng cấu hình mới
}

// Hàm khôi phục lại Terminal khi thoát chương trình
void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
}

// Hàm phát âm thanh
void play_note(int fd, int freq, const char* note_name) {
    ioctl(fd, PWM_IOCTL_SET_FREQ, freq);
    printf("\rĐang chơi nốt: %-5s (%d Hz) - Nhấn '0' để ngắt âm... ", note_name, freq);
    fflush(stdout); // Ép in ra terminal ngay lập tức
}

int main(int argc, char **argv) {
    int pwm_fd = open("/dev/pwm", 0);
    if (pwm_fd < 0) {
        perror("Loi mo thiet bi /dev/pwm");
        exit(1);
    }

    printf("\n=========================================\n");
    printf("🎹 CHUONG TRINH MO PHONG PIANO DIEN 🎹\n");
    printf("=========================================\n");
    printf("Huong dan phim bam:\n");
    printf(" [1] Do  (C)  | [2] Re  (D)  | [3] Mi  (E)\n");
    printf(" [4] Fa  (F)  | [5] Sol (G)  | [6] La  (A)\n");
    printf(" [7] Si  (B)  | [8] Do+ (C5)\n");
    printf(" ----------------------------------------\n");
    printf(" [0] Ngat am (Mute)   | [q] Thoat (Quit)\n");
    printf("=========================================\n\n");

    setup_terminal_for_piano(); // Bật chế độ phím đàn
    
    char key;
    while (1) {
        key = getchar(); // Bắt phím ngay lập tức

        switch (key) {
            // Chuyển sang chơi ở Quãng 5 (Octave 5) - Cao hơn, hợp với Buzzer hơn

            case '1': play_note(pwm_fd, 2093, "Do 7"); break;
            case '2': play_note(pwm_fd, 2349, "Re 7"); break;
            case '3': play_note(pwm_fd, 2700, "Mi 7"); break;
            case '4': play_note(pwm_fd, 2794, "Fa 7"); break; // Vượt qua mốc cộng hưởng
            case '5': play_note(pwm_fd, 3136, "Sol 7"); break;
            case '6': play_note(pwm_fd, 3520, "La 7"); break;
            case '7': play_note(pwm_fd, 3951, "Si 7"); break;
            case '8': play_note(pwm_fd, 4186, "Do 8"); break;
            case '0':
                ioctl(pwm_fd, PWM_IOCTL_STOP);
                printf("\r%-40s", "Ngắt âm (Mute)"); // Ghi đè dòng hiện tại cho đẹp
                fflush(stdout);
                break;
            case 'q':
            case 'Q':
                ioctl(pwm_fd, PWM_IOCTL_STOP);
                restore_terminal(); // Trả lại Terminal như bình thường
                close(pwm_fd);
                printf("\n\nĐã thoát chương trình Piano!\n");
                exit(0);
        }
    }

    return 0;
}