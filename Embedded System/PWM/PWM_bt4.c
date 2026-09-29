#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ON 1
#define OFF 0

#define PWM_IOCTL_SET_FREQ 1
#define PWM_IOCTL_STOP 0

void set_buzzer_freq(int fd, int freq) {
    int ret = ioctl(fd, PWM_IOCTL_SET_FREQ, freq);
    if(ret < 0) {
        perror("Loi set tan so buzzer");
        exit(1);
    }
}

void stop_buzzer(int fd) {
    int ret = ioctl(fd, PWM_IOCTL_STOP);
    if(ret < 0) {
        perror("Loi stop buzzer");
        exit(1);
    }
    close(fd);
}
int main(int argc, char *argv[]) {
    // Mở rộng mảng lên 6 phần tử để nhận diện được phím K5
    char buttons[6] = {'0','0','0','0','0','0'};

    int btn_fd = open("/dev/buttons", 0);
    if (btn_fd < 0) { perror("Loi mo /dev/buttons"); exit(1); }

    int pwm_fd = open("/dev/pwm", 0);
    if (pwm_fd < 0) { perror("Loi mo /dev/pwm"); exit(1); }

    int freq = 1000;
    int i;
    // Bật ngay mặc định 1000Hz khi chạy chương trình
    ioctl(pwm_fd, PWM_IOCTL_SET_FREQ, freq);
    printf("\n--- BAI 4: DIEU CHINH TAN SO BUI NUT NHAN ---\n");
    printf("Mac dinh: 1000Hz\nK1(+100) | K2(+50) | K3(-100) | K4(-50) | K5(STOP)\n\n");

    while (1) {
        char current_buttons[6];
        // Đọc dữ liệu vào mảng thực tế từ /dev/buttons
        int bytes_read = read(btn_fd, current_buttons, sizeof(current_buttons));
        if (bytes_read < 0) {
            perror("Loi doc buttons");
            exit(1);
        }

        // Tính toán số lượng nút thực tế driver trả về để duyệt cho đúng
        int num_buttons = bytes_read / sizeof(current_buttons[0]);

        for (i = 0; i < num_buttons; i++) {
            if (buttons[i] != current_buttons[i]) {
                buttons[i] = current_buttons[i];

                // Chỉ xử lý logic tăng/giảm tần số khi CÓ SỰ KIỆN NHẤN XUỐNG ('1')
                if (buttons[i] == '1') {
                    int is_freq_changed = 0;

                    if (i == 0) { freq += 100; is_freq_changed = 1; }      // K1
                    else if (i == 1) { freq += 50; is_freq_changed = 1; }  // K2
                    else if (i == 2) { freq -= 100; is_freq_changed = 1; } // K3
                    else if (i == 3) { freq -= 50; is_freq_changed = 1; }  // K4
                    else if (i == 4) { // K5
                        printf("Nhan K5 -> STOP PWM\n");
                        ioctl(pwm_fd, PWM_IOCTL_STOP);
                    }

                    if (is_freq_changed) {
                        // Khống chế giới hạn 1Hz -> 2000Hz
                        if (freq > 2000) freq = 2000;
                        if (freq < 1) freq = 1;

                        // Set lại tần số mới
                        ioctl(pwm_fd, PWM_IOCTL_SET_FREQ, freq);
                        printf("Nhan K%d -> Tan so hien tai: %d Hz\n", i + 1, freq);
                    }
                }
            }
        }
    }

    close(btn_fd);
    close(pwm_fd);
    return 0;
}