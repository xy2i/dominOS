#include "sysapi.h"

int main()
{
    printf("call in main");
    __asm__("int $49");
    int retval;
    __asm __volatile("movl %%eax , %0\n\t" : "=r"(retval));
    for (;;)
	;
}