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
int main(int argc, char **argv) {
    int led_fd, pwm_fd,i;
    int count, freq = 1000;

    led_fd = open("/dev/leds", 0);
    if (led_fd < 0) { perror("Loi mo /dev/leds"); exit(1); }

    pwm_fd = open("/dev/pwm", 0);
    if (pwm_fd < 0) { perror("Loi mo /dev/pwm"); exit(1); }

    printf("\n--- BAI 2: 4 LED DEM LEN & PWM TANG DAN ---\n");

    for (count = 0; count <= 15; count++) {
        // Xuất trạng thái đếm ra 4 LED bằng phép toán bitwise (AND bit)
        ioctl(led_fd, (count & 8) ? ON : OFF, 3); // LED 4 (Bit MSB)
        ioctl(led_fd, (count & 4) ? ON : OFF, 2); // LED 3
        ioctl(led_fd, (count & 2) ? ON : OFF, 1); // LED 2
        ioctl(led_fd, (count & 1) ? ON : OFF, 0); // LED 1 (Bit LSB)

        // Phát xung PWM
        ioctl(pwm_fd, PWM_IOCTL_SET_FREQ, freq);
        printf("LED Count: %d (He 10) - PWM Freq: %d Hz\n", count, freq);

        usleep(300000); // Trễ 300ms = 300,000 us
        freq += 200;    // Tăng tần số cho lần lặp tiếp theo
    }

    // Kết thúc vòng lặp: Tắt toàn bộ LED và Dừng PWM
    for(i=0; i<4; i++) ioctl(led_fd, OFF, i);
    ioctl(pwm_fd, PWM_IOCTL_STOP);

    close(led_fd);
    close(pwm_fd);
    return 0;
}