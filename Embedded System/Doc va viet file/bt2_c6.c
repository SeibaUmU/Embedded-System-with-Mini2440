#include <stdio.h>
#include <stdlib.h>
#include <termios.h> // Thư viện cấu hình terminal trên Linux
#include <unistd.h>  // Thư viện chuẩn chứa định nghĩa STDIN_FILENO

const char* filename = "input.txt";

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
    
    // Trả lại thuộc tính terminal như cũ
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}

// Hàm đếm số ký tự 'a' trong file
int count_a() {
    FILE *f = fopen(filename, "r");
    if (f == NULL) return 0; // Trả về 0 nếu file chưa tồn tại
    
    int count = 0;
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (ch == 'a') {
            count++;
        }
    }
    fclose(f);
    return count;
}

// Hàm mô phỏng hiển thị trạng thái 4 LED đơn (Dùng phép toán Bitwise)
void display_leds(int value) {
    printf("=> Trang thai 4 LED (so du %d): ", value);
    
    // Kiểm tra từ bit 3 xuống bit 0 (tương ứng LED 3 đến LED 0)
    for (int i = 3; i >= 0; i--) {
        // Kỹ thuật mặt nạ bit (Bitmasking): AND giá trị với 1 dịch trái i bit
        if (value & (1 << i)) {
            printf("* "); // LED Sáng (Mức 1)
        } else {
            printf("_ "); // LED Tắt (Mức 0)
        }
    }
    printf("\n");
}

// Hàm đổi tất cả 'a' thành 'A' trực tiếp trên file
void replace_a_with_A() {
    // Mở ở chế độ r+ (Đọc và Ghi trên file đã tồn tại)
    FILE *f = fopen(filename, "r+");
    if (f == NULL) {
        printf("\nLoi: Khong the mo file %s!\n", filename);
        return;
    }
    
    int ch;
    int replaced = 0;
    while ((ch = fgetc(f)) != EOF) {
        if (ch == 'a') {
            // Lùi con trỏ file lại 1 byte để ghi đè lên đúng chữ 'a' vừa đọc
            fseek(f, -1, SEEK_CUR); 
            fputc('A', f);
            // Bắt buộc đồng bộ lại luồng đọc/ghi sau khi thực hiện fputc
            fseek(f, 0, SEEK_CUR); 
            replaced = 1;
        }
    }
    fclose(f);
    
    if (replaced) printf("\n=> Da doi toan bo 'a' thanh 'A' trong file!\n");
    else printf("\n=> Khong tim thay ky tu 'a' nao de doi.\n");
}

// Hàm ghi thêm số lượng vào cuối file
void append_count(int count) {
    // Mở ở chế độ a (Append - Ghi nối thêm)
    FILE *f = fopen(filename, "a");
    if (f == NULL) {
        printf("\nLoi: Khong the mo file %s!\n", filename);
        return;
    }
    
    fprintf(f, "\nTong so ky tu 'a' truoc do: %d\n", count);
    fclose(f);
    printf("\n=> Da ghi them so %d vao cuoi file %s!\n", count, filename);
}

int main() {
    int key; // Chuyển từ char sang int để phù hợp với hàm getch()
    int current_count = 0;

    // Tạo file tạm nếu chưa có để tránh lỗi khi đọc
    FILE *init = fopen(filename, "a");
    if(init) fclose(init);

    printf("--- HE THONG QUAN LY FILE & GIA LAP LED (LINUX) ---\n");
    printf("[q] : Dem ky tu 'a', in ra man hinh & LED.\n");
    printf("[w] : Doi toan bo 'a' thanh 'A'.\n");
    printf("[r] : Ghi tong so ky tu 'a' vao cuoi file.\n");
    printf("[ESC] : Thoat chuong trinh.\n");
    printf("---------------------------------------------------\n");

    while (1) {
        key = getch(); // Thay _getch() bằng getch()

        switch (key) {
            case 'q':
            case 'Q':
                current_count = count_a();
                printf("\n--- Phim Q ---\n");
                printf("=> So luong ky tu 'a' trong file: %d\n", current_count);
                display_leds(current_count % 16); // Lấy số dư chia 16 để nạp vào 4 LED
                break;
                
            case 'w':
            case 'W':
                printf("\n--- Phim W ---\n");
                replace_a_with_A();
                break;
                
            case 'r':
            case 'R':
                printf("\n--- Phim R ---\n");
                current_count = count_a(); // Đếm lại cho chắc chắn số liệu mới nhất
                append_count(current_count);
                break;
                
            case 27: // Mã ASCII của phím ESC
                printf("\nThoat chuong trinh.\n");
                return 0;
        }
    }
    return 0;
}