#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ON 1
#define OFF 0

#define PWM_IOCTL_SET_FREQ 1
#define PWM_IOCTL_STOP 0

void set_buzzer_freq(int fd, int freq) {
    int ret = ioctl(fd, PWM_IOCTL_SET_FREQ, freq);
    if(ret < 0) {
        perror("Loi set tan so buzzer");
        exit(1);
    }
}

void stop_buzzer(int fd) {
    int ret = ioctl(fd, PWM_IOCTL_STOP);
    if(ret < 0) {
        perror("Loi stop buzzer");
        exit(1);
    }
    close(fd);
}
int main(int argc, char *argv[]) {
    int btn_fd = open("/dev/buttons", 0);
    if (btn_fd < 0) { perror("Loi mo /dev/buttons"); exit(1); }

    int pwm_fd = open("/dev/pwm", 0);
    if (pwm_fd < 0) { perror("Loi mo /dev/pwm"); exit(1); }
    int i;
    char buttons[4] = {'0','0','0','0'}; // Lưu trạng thái nút cũ

    printf("\n--- BAI 3: DIEU KHIEL PWM BANG NUT NHAN K1, K2, K3 ---\n");
    printf("Nhan giu K1/K2/K3 de tao xung. Nha nut de dung.\n");

    while (1) {
        char current_buttons[4];
        if (read(btn_fd, current_buttons, sizeof(current_buttons)) != sizeof(current_buttons)) {
            perror("Loi doc buttons");
            exit(1);
        }

        for (i = 0; i < 4; i++) {
            // Nếu phát hiện trạng thái nút có sự thay đổi
            if (buttons[i] != current_buttons[i]) {
                buttons[i] = current_buttons[i];

                if (buttons[i] == '0') {
                    // Trạng thái nút là '0' (UP) -> Dừng chế độ PWM
                    printf("Nha nut K%d -> DUNG PWM\n", i + 1);
                    ioctl(pwm_fd, PWM_IOCTL_STOP);
                }
                else if (buttons[i] == '1') {
                    // Trạng thái nút là '1' (DOWN)
                    if (i == 0) {
                        printf("Nhan K1 -> 500Hz\n");
                        ioctl(pwm_fd, PWM_IOCTL_SET_FREQ, 500);
                    }
                    else if (i == 1) {
                        printf("Nhan K2 -> 1000Hz\n");
                        ioctl(pwm_fd, PWM_IOCTL_SET_FREQ, 1000);
                    }
                    else if (i == 2) {
                        printf("Nhan K3 -> 1500Hz\n");
                        ioctl(pwm_fd, PWM_IOCTL_SET_FREQ, 1500);
                    }
                }
            }
        }
    }

    close(btn_fd);
    close(pwm_fd);
    return 0;
}