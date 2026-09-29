#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>

#define ON 1
#define OFF 0

int main(int argc, char** argv)
{
    int fd, i, c;
    int led_status[4] = {0,0,0,0};
    fd = open("/dev/leds", 0);
    if (fd < 0) {
        perror("open device leds");
        exit(1);
    }

    while (1) {
        //s: hien thi trang thai, 1-4: dao trang thai led, q: thoat
        printf("\n--- Led Control Programs ---\n");
        printf("s: status, 1-4: led, q: quit\n");
        printf("Enter a character: ");

        c = getchar();
        // Xóa bộ nhớ đệm
        while (getchar() != '\n' && getchar() != EOF); // Clear input buffer

        switch (c) {
            case 's':
                printf("Trang thai LED: Led0:%s, Led1:%s, Led2:%s, Led3:%s\n",
                        led_status[0] ? "ON" : "OFF",
                        led_status[1] ? "ON" : "OFF",
                        led_status[2] ? "ON" : "OFF",
                        led_status[3] ? "ON" : "OFF");
                break;

            case '1':
                led_status[0] = !led_status[0];
                ioctl(fd, led_status[0] ? ON : OFF, 0);
                printf("Led0 is %s\n", led_status[0] ? "ON" : "OFF");
                break;

            case '2':
                led_status[1] = !led_status[1];
                ioctl(fd, led_status[1] ? ON : OFF, 1);
                printf("Led1 is %s\n", led_status[1] ? "ON" : "OFF");
                break;

            case '3':
                led_status[2] = !led_status[2];
                ioctl(fd, led_status[2] ? ON : OFF, 2);
                printf("Led2 is %s\n", led_status[2] ? "ON" : "OFF");
                break;

            case '4':
                led_status[3] = !led_status[3];
                ioctl(fd, led_status[3] ? ON : OFF, 3);
                printf("Led3 is %s\n", led_status[3] ? "ON" : "OFF");
                break;

            case 'q':
                close(fd);
                return 0;

            default:
                printf("Lua chon khong hop le!\n");
                break;
        }
    }
    return 0;
}