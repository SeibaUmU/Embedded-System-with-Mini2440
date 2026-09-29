#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>

#define ON 1    // Fixed: Was 'I', now '1'
#define OFF 0

int led_fd, button_fd; 
static int t = 1000; 

void sleepms(int ms) {
    usleep(ms * 1000);
}

void* btn_polling(void* param); 

int main(int argc, char** argv) {
    const char *thread_parms = "button thread";
    pthread_t thread_id; 
    int led_no; 
    int i;

    // Open LED device
    led_fd = open("/dev/leds", 0); 
    if (led_fd < 0) {
        perror("open device leds");
        exit(1);
    } else {
        printf("open device led ok\n");
    }

    // Open Button device
    button_fd = open("/dev/buttons", 0); 
    if (button_fd < 0) {
        perror("open device buttons");
        exit(1); // Fixed: Was ':'
    } else {
        printf("open device button ok\n");
    }

    // Create polling thread
    if(pthread_create(&thread_id, NULL, btn_polling, (void*)thread_parms) != 0) {
        perror("pthread_create");
        exit(1);
    }

    // Initial state: all LEDs off
    for(i = 0; i < 4; i++) ioctl(led_fd, OFF, i);
    
    led_no = 0;
    while(1) {
        ioctl(led_fd, ON, led_no);
        sleepms(t);
        ioctl(led_fd, OFF, led_no); // Fixed: Was 'led fd'
        
        led_no++; 
        if(led_no == 4) led_no = 0;
    }

    close(button_fd);
    close(led_fd);
    return 0;
}

void* btn_polling(void* param) {
    // Fixed: Initialization used () instead of {}
    char cur_btn[2], old_btn[2] = {'0', '0'}; 
    
    for(;;) {
        int num = read(button_fd, cur_btn, sizeof(cur_btn));
        if(num != sizeof(cur_btn)) {
            perror("read buttons:");
            exit(1);
        }

        // Handle Button K1 (Increase delay)
        if(old_btn[0] != cur_btn[0]) {
            sleepms(300); 
            old_btn[0] = cur_btn[0];
            t = t + 50; 
            printf("K1 changed: t = %dms\n", t);
        }

        // Handle Button K2 (Decrease delay)
        if(old_btn[1] != cur_btn[1]) {
            sleepms(300); 
            old_btn[1] = cur_btn[1];
            t = t - 50; 
            if(t < 100) t = 100;
            printf("K2 changed: t = %dms\n", t);
        }
    }
    return NULL;
}