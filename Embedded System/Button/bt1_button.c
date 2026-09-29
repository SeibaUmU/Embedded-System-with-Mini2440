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
    {   //nhan nut thi led sang, nha nut thi led tat
        char current_buttons[4];
        int count_of_changed_key;
        int i;
        if (read(buttons_fd, current_buttons, sizeof current_buttons) != sizeof current_buttons) 
        //buttons_fd: doc du lieu tu file descriptor cua buttons, luu vao current_buttons, so byte toi da doc duoc phai bang kich thuoc cua current_buttons (4 byte =  4 nut)
        {   //tam dung chg trinh cho den khi co su thay doi o nut nhan, doc trang thai moi vao current_buttons
            perror("read"); //neu doc khong du 4 byte thi bao loi va thoat
            exit(1);
        }
        for (i=0, count_of_changed_key=0; i< sizeof buttons /sizeof buttons[0]; i++)
        {
            if (buttons[i] != current_buttons[i]) // Nếu trạng thái nút thứ i thay đổi
            {   //buttons[i] la trang thai truoc do, current_buttons[i] la trang thai moi doc duoc
                buttons[i] = current_buttons[i]; // Cập nhật trạng thái mới
                printf("key %d is %s",i+1,buttons[i] == '0' ? "up" : "down");

                // Kiểm tra nút là '0' (nhả) hay '1' (nhấn)
                if (buttons[i] == '0')
                {
                    ioctl(fd, OFF,i);
                }
                else
                {
                    ioctl(fd, ON, i);
                }
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