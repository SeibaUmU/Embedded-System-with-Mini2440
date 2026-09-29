#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h> // Thư viện cấu hình terminal trên Linux
#include <unistd.h>  // Thư viện chuẩn chứa định nghĩa STDIN_FILENO

const char* filename = "text.txt";

// --- HÀM GIẢ LẬP getch() TRÊN LINUX ---
int getch(void) {
    struct termios oldattr, newattr;
    int ch;
    // Lấy thuộc tính hiện tại của terminal
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    // Tắt chế độ chờ Enter (ICANON) và tắt hiển thị ký tự (ECHO)
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    
    // Đọc 1 ký tự
    ch = getchar();
    
    // Trả lại thuộc tính terminal như cũ để không làm lỗi hệ thống
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}

// Hàm hỗ trợ: Đọc số đếm hiện tại từ file
int read_count() {
    FILE *f = fopen(filename, "r");
    int count = 0;
    // Nếu file tồn tại, đọc giá trị ra. Nếu không, mặc định là 0.
    if (f != NULL) {
        fscanf(f, "%d", &count);
        fclose(f);
    }
    return count;
}

// Hàm hỗ trợ: Lưu số đếm ghi đè vào file
void write_count(int count) {
    FILE *f = fopen(filename, "w"); // Chế độ "w" sẽ ghi đè dữ liệu cũ
    if (f != NULL) {
        fprintf(f, "%d", count);
        fclose(f);
    } else {
        printf("\nLoi: Khong the mo file de ghi!\n");
    }
}

int main() {
    // 1. Đọc số đếm cũ từ file khi khởi động chương trình (để không bị mất dữ liệu lần chạy trước)
    int q_count = read_count();
    int key; // Đổi từ char sang int để khớp với hàm getch()

    printf("--- CHUONG TRINH DEM SO LAN BAM PHIM (LINUX) ---\n");
    printf("[q] : Tang bien dem 'q' va luu file.\n");
    printf("[w] : Xem so lan nhan phim 'q'.\n");
    printf("[r] : Reset so lan nhan ve 0.\n");
    printf("[ESC] : Thoat chuong trinh.\n");
    printf("------------------------------------------------\n");

    // Vòng lặp vô hạn chờ người dùng bấm phím
    while(1) {
        key = getch(); // Đổi từ _getch() sang hàm getch() vừa tạo

        switch(key) {
            case 'q':
            case 'Q':
                q_count++;            // Tăng biến đếm
                write_count(q_count); // Cập nhật ngay lập tức vào file
                // printf("Da nhan 'q'...\n"); // Có thể bỏ comment dòng này nếu muốn có phản hồi màn hình
                break;
                
            case 'w':
            case 'W':
                // Đọc trực tiếp lại từ file để chứng minh dữ liệu thực sự đã được lưu ổ cứng
                printf("\n=> So lan nhan phim 'q' (doc tu file): %d\n", read_count());
                break;
                
            case 'r':
            case 'R':
                q_count = 0;          // Reset biến cục bộ
                write_count(q_count); // Ghi đè số 0 vào file text.txt
                printf("\n=> Da reset so lan nhan ve 0!\n");
                break;
                
            case 27: // 27 là mã ASCII của phím ESCape
                printf("\nThoat chuong trinh.\n");
                return 0; 
        }
    }
    return 0;
}