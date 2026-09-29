//
// Created by seiba on 31/03/2026.
//

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/select.h>
#include<sys/time.h>
#include<errno.h>

#define ON 1
#define OFF 0

int main(int argc,char *argv[])
{
    int buttons_fd;
    char buttons[4] = {'0','0','0','0'};
    //int led_status[4] = {0,0,0,0};
    buttons_fd = open("/dev/buttons",0);
    if (buttons_fd<0)
    {
        perror("open");
        exit(1);
    }
    int fd;
    fd = open("/dev/leds", 0);
    if (fd < 0)
    {
        perror("open device leds");
        exit(1);
    }
    while (1)
    {   //K3: 4 led dem len nhi phan t 500ms, lap lai 1 lan
        //K4: 4 led sang dan tu tren xuong duoi t=500ms, lap lai 1 lan
        char current_buttons[4];
        int count_of_changed_key;
        int i,j,n;
        if (read(buttons_fd, current_buttons, sizeof current_buttons) != sizeof current_buttons)
        {
            perror("read");
            exit(1);
        }
        for (i=0, count_of_changed_key=0; i< sizeof buttons /sizeof buttons[0]; i++)
        {
            if (buttons[i] != current_buttons[i])
            {
                // KIỂM TRA NHẢ PHÍM (Từ 1 về 0)
                if (buttons[i] == '1' && current_buttons[i] == '0')
                {
                    if (i == 2) // Nút 3
                    {
                        for (n = 0; n <= 15; n++)
                        {
                            // Kiểm tra từng bit của số i để điều khiển 4 LED
                            for (j = 0; j < 4; j++)
                            {
                                // (i >> j) & 1: Lấy giá trị bit thứ j của số i
                                if ((n >> j) & 1) {
                                    ioctl(fd, ON, j);
                                } else {
                                    ioctl(fd, OFF, j);
                                }
                            }

                            usleep(100000);
                        }

                        for (j = 0; j < 4; j++) ioctl(fd, OFF, j);
                        usleep(500000);
                    }
                    else if (i == 3) // Nút 4
                    {
                        for (j = 3; j >= 0; j--)
                        {
                            ioctl(fd, ON, j);
                            usleep(500000);
                        }
                        for (j = 0; j < 4; j++) ioctl(fd, OFF, j);
                        usleep(500000);
                    }
                }

                // In trạng thái để theo dõi
                printf("key %d is %s ", i + 1, current_buttons[i] == '0' ? "up" : "down");

                // CUỐI CÙNG mới cập nhật trạng thái cũ bằng trạng thái mới
                buttons[i] = current_buttons[i];
                count_of_changed_key++;
            }
        }
        if (count_of_changed_key)
        {
            printf("\n");
        }
    }
    close(buttons_fd);
    close(fd);
    return 0;
}