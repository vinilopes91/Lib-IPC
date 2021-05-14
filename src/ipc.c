#include <stdio.h>
 
 
void ipc(void)
{
    puts("Hello, I am a shared library");
}

/* Compilação 
 * gcc -c -Wall -Werror -fpic ipc.c
 * gcc -shared -o libipc.so ipc.o
 * gcc -L/home/username/ipc -Wall -o test main.c -lipc
 */

