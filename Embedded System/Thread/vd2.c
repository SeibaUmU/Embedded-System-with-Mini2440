#include <stdio.h>
#include <pthread.h>
#include <unistd.h>  // Required for sleep()
#include <stdint.h>  // Required for intptr_t (safest way to cast)

/* Ham thuc thi tuyen */
void* do_loop(void* data)
{
    // Cast void* back to an integer type
    int id = (intptr_t)data; 
    int i;
    for(i = 0; i < 10; i++)
    {
        sleep(1); // Dung 1 s
        // Use id (the integer) instead of data (the pointer)
        printf("Thread '%d' - Got '%d'\n", id, i);
    }
    pthread_exit(NULL); 
}

int main(void)
{
    pthread_t p_thread;
    int a = 1;
    int b = 2;

    /* Tao tuyen */
    // Use (intptr_t) to safely cast the int to a pointer size
    pthread_create(&p_thread, NULL, do_loop, (void*)(intptr_t)a);
    
    // Call the function for the second "thread" (runs in main)
    do_loop((void*)(intptr_t)b);

    // Wait for the created thread to finish before exiting main
    pthread_join(p_thread, NULL);

    return 0;
}