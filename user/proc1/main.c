#define WITH_SEM
#include "../tests/lib/sysapi.h"

int main()
{
    //getprio(0);
    __asm__("    mov $1, %eax\n"
            "    int $49\n");
    int retval;
    __asm__("mov %%eax, %0" : "=r"(retval));
    return retval;
    //int x = *(int *)0x42;
    // should crash
}