//4 led chop tat
#include <stdlib.h>
#include <unistd.h> //usleep,close
#include <sys/stat.h>
#include <fcntl.h> //open
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>

#define ON 1
#define OFF 0
int main(int argc, char** argv) //argc: so luong doi so truyen vao, argv: mang chua cac doi so truyen vao
{
    int fd;
    fd = open("/dev/leds", 0); //tra ve -1 neu mo that bai, tra ve 0 neu mo thanh cong, 0 la chi doc, 1 la chi ghi, 2 la doc va ghi
    if (fd < 0)
    {
        perror("open device leds"); //in ra loi neu mo that bai
        exit(1);
    }
    while (1)
    {
        ioctl(fd, ON,0); //cau truc cua ioctl: int ioctl(int fd, unsigned long request, ...); fd: file descriptor, request: yeu cau, ...: tham so truyen vao
        //ON: yeu cau bat den, OFF: yeu cau tat den, 0: den 1, 1: den 2, 2: den 3, 3: den 4
        ioctl(fd, ON,1);
        ioctl(fd, ON,2);
        ioctl(fd, ON,3);
        usleep(500000); //usleep: us micro giay, sleep: giay, 500000 us micro giay = 0.5 giay = 500 ms
        ioctl(fd, OFF,0);
        ioctl(fd, OFF,1);
        ioctl(fd, OFF,2);
        ioctl(fd, OFF,3);
        usleep(500000); 
    }
    close(fd); //tra ve 0 neu dong thanh cong, tra ve -1 neu dong that bai
    return 0;
}
