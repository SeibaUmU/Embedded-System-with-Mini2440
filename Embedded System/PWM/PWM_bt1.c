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
    int fd = open("/dev/pwm", 0);
    if (fd < 0) {
        perror("Loi mo thiet bi pwm");
        exit(1);
    }

    int freq = 500; // Tần số ban đầu
    printf("\n--- BAI 1: TEST PWM TU DONG TANG ---\n");

    while (freq <= 2000) {
        printf("Dang phat tan so: %d Hz\n", freq);
        set_buzzer_freq(fd, freq);

        sleep(2); // Dừng 2 giây
        freq += 100; // Tăng thêm 100Hz
    }

    printf("Da dat 2000Hz. Dung PWM.\n");
    stop_buzzer(fd);

    return 0;
}