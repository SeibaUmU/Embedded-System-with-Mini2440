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
    {
        ioctl(fd, ON,0);
        sleep(1);
        ioctl(fd, OFF,0);
        sleep(1);
    }
    close(fd);
    return 0;
}
