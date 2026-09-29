#define _DEFAULT_SOURCE
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
    int fd,i;
    fd = open("/dev/leds", 0);
    if (fd < 0)
    {
        perror("open device leds");
        exit(1);
    }
    while (1)
    {   //4 led chay duoi dang chay tu trai sang phai
        for (i = 0; i <= 3; i++)
        {
            ioctl(fd, ON,i);
            usleep(40000);
            ioctl(fd, OFF, i);
        }
    }
    close(fd);
    return 0;
}
