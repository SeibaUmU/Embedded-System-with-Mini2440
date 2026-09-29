#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PWM_IOCTL_SET_FREQ 1
#define PWM_IOCTL_STOP 0

// --- BẢNG TẦN SỐ NỐT NHẠC (QUÃNG 6 & 7) ---
// Đã được tuỳ chỉnh để kêu mượt nhất trên còi từ Mini6410
#define B5  988   // Si 5 (Nốt trầm nhất bài)
#define D6  1175  // Rê 6
#define E6  1319  // Mi 6
#define G6  1568  // Sol 6
#define A6  1760  // La 6
#define B6  1976  // Si 6
#define D7  2349  // Rê 7
#define E7  2600  // Mi 7 (Đã hạ từ 2637Hz để né đỉnh cộng hưởng gây rè loa)
#define REST 0    // Dấu lặng

// --- ĐỊNH NGHĨA NHỊP ĐỘ (TEMPO) ---
// Bài hành khúc nhịp 2/4 hơi nhanh (Andantino)
#define E 220 // Móc đơn (Eighth note)
#define Q 440 // Nốt đen (Quarter note)
#define H 880 // Nốt trắng (Half note)

struct Note {
    int freq;
    int duration_ms;
};

// --- GIAI ĐIỆU BÀI HÁT ---
struct Note bac_ho_beat[] = {
    // 1. Như có Bác Hồ trong ngày vui đại thắng
    {G6, E}, {D7, E},
    {B6, E}, {G6, E}, {E7, E}, {D7, E},
    {B6, E}, {G6, E}, {A6, Q}, {REST, E},

    // 2. Lời Bác nay đã thành chiến thắng huy hoàng
    {A6, E}, {B6, E},
    {G6, E}, {E6, E}, {A6, E}, {G6, E},
    {E6, E}, {D6, E}, {D6, Q}, {REST, E},

    // 3. Ba mươi năm đấu tranh giành trọn vẹn non sông
    {G6, E}, {D7, E},
    {B6, E}, {G6, E}, {E7, E}, {D7, E},
    {B6, E}, {G6, E}, {A6, E}, {A6, Q}, {REST, E},

    // 4. Ba mươi năm Dân chủ Cộng hòa, Kháng chiến đã thành công!
    {A6, E}, {B6, E},
    {G6, E}, {E6, E}, {A6, E}, {G6, E},
    {E6, E}, {D6, E}, {B5, E}, {D6, E}, {E6, E}, {G6, Q}, {REST, Q},

    // --- ĐIỆP KHÚC ---
    // Việt Nam - Hồ Chí Minh (Lần 1)
    {D7, Q}, {B6, Q}, {G6, E}, {A6, E}, {B6, Q}, {REST, E},
    // Việt Nam - Hồ Chí Minh (Lần 2)
    {D7, Q}, {B6, Q}, {G6, E}, {A6, E}, {G6, Q}, {REST, E},
    // Việt Nam - Hồ Chí Minh (Lần 3)
    {D7, Q}, {B6, Q}, {G6, E}, {A6, E}, {B6, Q}, {REST, E},
    // Việt Nam - Hồ Chí Minh (Lần 4 - Kết thúc ngân dài)
    {D7, Q}, {B6, Q}, {G6, E}, {A6, E}, {G6, H}
};

int main(int argc, char **argv) {
    int pwm_fd = open("/dev/pwm", 0);
    int i;
    if (pwm_fd < 0) {
        perror("Loi mo thiet bi /dev/pwm");
        exit(1);
    }

    printf("\n🇻🇳 DANG PHAT: Nhu co Bac Ho trong ngay vui dai thang 🇻🇳\n");

    int num_notes = sizeof(bac_ho_beat) / sizeof(bac_ho_beat[0]);

    for (i = 0; i < num_notes; i++) {
        int current_freq = bac_ho_beat[i].freq;
        int current_duration = bac_ho_beat[i].duration_ms;

        if (current_freq == REST) {
            ioctl(pwm_fd, PWM_IOCTL_STOP);
        } else {
            ioctl(pwm_fd, PWM_IOCTL_SET_FREQ, current_freq);
        }

        usleep(current_duration * 1000);

        // Kỹ thuật ngắt âm chuẩn cho nhạc Hành khúc (Marching)
        // Ngắt còi 25ms tạo độ nảy (staccato) dứt khoát giữa các chữ
        ioctl(pwm_fd, PWM_IOCTL_STOP);
        usleep(25000);
    }

    printf("Hoan thanh! Da dung PWM.\n\n");
    ioctl(pwm_fd, PWM_IOCTL_STOP);
    close(pwm_fd);
    
    return 0;
}