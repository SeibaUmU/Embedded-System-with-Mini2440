#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
int main(){
    printf("POSIX ver is set to %ld\n",_POSIX_VERSION);
    if(_POSIX_VERSION<199506L)
        printf("This system does not sp thread!\n");
    else
        printf("Thread is available in this system!\n");
    
}
