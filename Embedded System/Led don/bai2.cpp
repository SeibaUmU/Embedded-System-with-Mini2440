//4 led dem len tu 0-15
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
    int fd;
    fd = open("/dev/leds", 0);
    if (fd < 0)
    {
        perror("open device leds");
        exit(1);
    }
    while (1)
    {   //0000-1111
        for (int i = 0; i <= 15; i++)
        {
            // Kiểm tra từng bit của số i để điều khiển 4 LED
            for (int j = 0; j < 4; j++)
            {
                // (i >> j) & 1: Lấy giá trị bit thứ j của số i
                if ((i >> j) & 1) {
                    ioctl(fd, ON, j);
                } else {
                    ioctl(fd, OFF, j);
                }
            }

            usleep(100000);
        }

        for (int j = 0; j < 4; j++) ioctl(fd, OFF, j);
        usleep(300000);
    }
    close(fd);
    return 0;
}
/*
5: 0101 = i
j = 0-3
5 >> 0: 010 1 & 1 = 1
5 >> 1: 001 0 & 1 = 0
*/