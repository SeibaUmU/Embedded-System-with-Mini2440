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
    int led_status[4] = {0,0,0,0};
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
    {   //nhan nha nut dao trang thai led
        char current_buttons[4];
        int count_of_changed_key;
        int i;
        if (read(buttons_fd, current_buttons, sizeof current_buttons) != sizeof current_buttons)
        {
            perror("read");
            exit(1);
        }
        for (i=0, count_of_changed_key=0; i< sizeof buttons /sizeof buttons[0]; i++)
        {
            if (buttons[i] != current_buttons[i])
            {
                // Kiểm tra: Nếu TRƯỚC ĐÓ là '1' (đang nhấn) và HIỆN TẠI là '0' (đã nhả)
                if (buttons[i] == '1' && current_buttons[i] == '0')
                {
                    led_status[i] = !led_status[i];
                    ioctl(fd, led_status[i] ? ON : OFF, i);
                    printf("\nKey %d released -> LED %d toggled!\n", i + 1, i + 1);
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