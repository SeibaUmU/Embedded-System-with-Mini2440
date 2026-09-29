#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>
int main(int argc, char **argv)
{
	int status;
	int led_no;
	int fd;
	if (argc != 3 || sscanf(argv[1], "%d", &status) != 1 || sscanf(argv[2],"%d", &led_no) != 1 ||
                status < 0 || status > 1 || led_no < 0 || led_no > 3) 
	{
		fprintf(stderr, "Usage: leds led_no 0|1\n");
		exit(1);
	}
        //Open leds device file
	fd = open("/dev/mini6410_leds", 0);
	if (fd < 0) {
		perror("error open device leds\n");
		exit(1);
	}
	ioctl(fd, status, led_no);
	close(fd);
	return 0;
}
