#include "stdio.h"
int main(int argc, char**argv)
{
int i=0;
printf("Hello world, lap trinh arm linux\n");
printf("Ten chuong trinh la: %s\n", argv[0]);
if(argc<2)
printf("Chuong trinh khong co tham so dau vao\n");
else
{
for(i=1;i<argc;i++)
{
printf("Tham so thu %d, noi dung %s\n",i,argv[i]);
}
}
return 0;
}
