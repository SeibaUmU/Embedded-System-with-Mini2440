#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
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
    
    // Trả lại thuộc tính terminal như cũ để không làm lỗi hệ thống
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}

// Hàm đếm số lượng ký tự số
int count_digits() {
    FILE *f = fopen(filename, "r");
    if (f == NULL) return 0;
    
    int count = 0;
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (isdigit(ch)) { // Kiểm tra xem ký tự có phải là số (0-9)
            count++;
        }
    }
    fclose(f);
    return count;
}

// Hàm đếm cả ký tự số và chữ thường (dùng cho phím E)
void get_stats(int *digits, int *lowers) {
    FILE *f = fopen(filename, "r");
    *digits = 0;
    *lowers = 0;
    if (f == NULL) return;
    
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (isdigit(ch)) {
            (*digits)++;
        } else if (islower(ch)) { // Kiểm tra ký tự in thường (a-z)
            (*lowers)++;
        }
    }
    fclose(f);
}

// Hàm xóa toàn bộ ký tự số bằng kỹ thuật File Tạm
void remove_digits() {
    FILE *fin = fopen(filename, "r");
    if (fin == NULL) {
        printf("\nLoi: Khong the mo file %s!\n", filename);
        return;
    }
    
    // Tạo một file tạm thời để lưu các ký tự KHÔNG phải là số
    FILE *ftemp = fopen("temp.txt", "w");
    if (ftemp == NULL) {
        printf("\nLoi: Khong the tao file tam!\n");
        fclose(fin);
        return;
    }

    int ch;
    int removed = 0;
    // Đọc từng ký tự từ file gốc
    while ((ch = fgetc(fin)) != EOF) {
        if (!isdigit(ch)) {
            fputc(ch, ftemp); // Chỉ ghi những ký tự KHÔNG phải là số vào file tạm
        } else {
            removed++; // Ghi nhận đã bỏ qua (xóa) một ký tự số
        }
    }
    
    fclose(fin);
    fclose(ftemp);

    // Xóa file gốc cũ và đổi tên file tạm thành file gốc
    remove(filename);
    rename("temp.txt", filename);

    if (removed > 0) {
        printf("\n=> Da xoa %d ky tu so khoi file %s!\n", removed, filename);
    } else {
        printf("\n=> Khong tim thay ky tu so nao de xoa.\n");
    }
}

// Hàm ghi thêm thống kê vào cuối file
void append_stats() {
    int digits = 0, lowers = 0;
    get_stats(&digits, &lowers); // Lấy số liệu cập nhật mới nhất
    
    FILE *f = fopen(filename, "a"); // Mở chế độ Append
    if (f == NULL) {
        printf("\nLoi: Khong the mo file %s de ghi!\n", filename);
        return;
    }
    
    // Ghi thêm vào cuối file (thêm \n để xuống dòng cho đẹp)
    fprintf(f, "\nTong so ky tu so: %d, Tong so ky tu chu thuong: %d\n", digits, lowers);
    fclose(f);
    
    printf("\n=> Da ghi thong ke (So: %d, Chu thuong: %d) vao cuoi file!\n", digits, lowers);
}

int main() {
    int key; // Đổi char thành int để phù hợp với hàm getch() trả về
    
    // Tạo file tạm nếu chưa có
    FILE *init = fopen(filename, "a");
    if(init) fclose(init);

    printf("--- CHUONG TRINH XU LY KY TU TRONG FILE (LINUX) ---\n");
    printf("[q] : Xem tong so ky tu so trong file.\n");
    printf("[w] : Xoa toan bo ky tu so khoi file.\n");
    printf("[e] : Ghi tong so ky tu (so & chu thuong) vao cuoi file.\n");
    printf("[ESC] : Thoat chuong trinh.\n");
    printf("---------------------------------------------------\n");

    while (1) {
        key = getch();

        switch (key) {
            case 'q':
            case 'Q':
                printf("\n--- Phim Q ---\n");
                printf("=> Co %d ky tu so trong file.\n", count_digits());
                break;
                
            case 'w':
            case 'W':
                printf("\n--- Phim W ---\n");
                remove_digits();
                break;
                
            case 'e':
            case 'E':
                printf("\n--- Phim E ---\n");
                append_stats();
                break;
                
            case 27: // Mã phím ESC
                printf("\nThoat chuong trinh.\n");
                return 0;
        }
    }
    return 0;
}