#include <stdio.h>
 
 
void ipc_init(void)
{
    puts("Hello, I am a shared library");
}

/* Compilação 
 * gcc -c -Wall -Werror -fpic ipc.c
 * gcc -shared -o libipc.so ipc.o
 * gcc -L/home/username/ipc -Wall -o test main.c -lipc
 * export LD_LIBRARY_PATH=/home/username/foo:$LD_LIBRARY_PATH
 */

