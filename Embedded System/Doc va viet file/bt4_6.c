#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>   // Thay cho <windows.h> de dung usleep()
#include <termios.h>  // Thu vien can thiep Terminal de lam ham getch()

const char* filename = "nhap.txt";

// --- HAM GIA LAP _getch() TREN LINUX ---
int getch(void) {
    struct termios oldattr, newattr;
    int ch;
    // Lay thuoc tinh hien tai cua terminal
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    // Tat che do doi Enter (ICANON) va tat hien thi ky tu (ECHO)
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    
    // Doc ky tu
    ch = getchar();
    
    // Tra lai thuoc tinh terminal nhu cu de khong loi he thong
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}

// --- HAM GIA LAP PWM TREN LINUX ---
void simulate_pwm(int freq, int ton, int toff, int cycles) {
    printf("=> Phat xung PWM: f=%dHz\n", freq);
    for (int i = 1; i <= cycles; i++) {
        printf("  [Chu ky %d/%d] delay %dms Ton\n", i, cycles, ton);
        usleep(ton * 1000); // usleep dung don vi micro-giay
        
        printf("  [Chu ky %d/%d] delay %dms Toff\n", i, cycles, toff);
        usleep(toff * 1000); 
    }
    printf("=> Ket thuc phat PWM.\n");
}

void process_q() {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("Loi: Khong the mo file %s\n", filename);
        return;
    }
    
    int count = 0, ch;
    while ((ch = fgetc(f)) != EOF) {
        if (islower(ch)) count++;
    }
    fclose(f);
    
    printf("\n--- Phim Q ---\n");
    printf("=> So ky tu thuong (a-z) trong file: %d\n", count);
    simulate_pwm(500, 300, 200, 1);
}

void process_w() {
    FILE *f = fopen(filename, "r+");
    if (f == NULL) {
        printf("Loi: Khong the mo file %s\n", filename);
        return;
    }
    
    int ch;
    int modified = 0;
    while ((ch = fgetc(f)) != EOF) {
        if (islower(ch)) {
            fseek(f, -1, SEEK_CUR);
            fputc(toupper(ch), f);
            fseek(f, 0, SEEK_CUR);
            modified = 1;
        } else if (isupper(ch)) {
            fseek(f, -1, SEEK_CUR);
            fputc(tolower(ch), f);
            fseek(f, 0, SEEK_CUR);
            modified = 1;
        }
    }
    fclose(f);
    
    printf("\n--- Phim W ---\n");
    if (modified) printf("=> Da dao nguoc in hoa/in thuong trong file.\n");
    else printf("=> Khong co ky tu chu cai nao de dao.\n");
    
    simulate_pwm(700, 400, 100, 2);
}

void process_e() {
    FILE *fin = fopen(filename, "r");
    if (fin == NULL) {
        printf("Loi: Khong the mo file %s\n", filename);
        return;
    }
    
    FILE *ftemp = fopen("temp.txt", "w");
    if (ftemp == NULL) {
        printf("Loi: Khong tao duoc file tam\n");
        fclose(fin);
        return;
    }
    
    int ch;
    while ((ch = fgetc(fin)) != EOF) {
        if (!isdigit(ch)) {
            fputc(ch, ftemp);
        }
    }
    
    fprintf(ftemp, "\nThe end\n");
    
    fclose(fin);
    fclose(ftemp);
    
    remove(filename);
    rename("temp.txt", filename);
    
    printf("\n--- Phim E ---\n");
    printf("=> Da xoa toan bo so va ghi 'The end' vao cuoi file.\n");
    
    simulate_pwm(900, 600, 500, 3);
}

int main() {
    int key; // getch() tra ve int
    
    FILE *init = fopen(filename, "a");
    if(init) fclose(init);

    printf("--- HE THONG QUAN LY FILE & GIA LAP PWM (LINUX) ---\n");
    printf("[q] : Dem ky tu thuong & Phat xung 500Hz.\n");
    printf("[w] : Dao chu hoa/thuong & Phat xung 700Hz.\n");
    printf("[e] : Xoa so, them 'The end' & Phat xung 900Hz.\n");
    printf("[ESC] : Thoat chuong trinh.\n");
    printf("---------------------------------------------------\n");

    while (1) {
        key = getch();

        switch (key) {
            case 'q':
            case 'Q':
                process_q();
                break;
                
            case 'w':
            case 'W':
                process_w();
                break;
                
            case 'e':
            case 'E':
                process_e();
                break;
                
            case 27: // Nút ESC
                printf("\nThoat chuong trinh.\n");
                return 0;
        }
    }
    return 0;
}