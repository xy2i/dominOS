#include "../../kernel/syscall.h"
#define WITH_SEM

int main()
{
    // ggetprio(0);
    __asm__("mov $1, %eax");
    __asm__("mov $1, %eax");
    __asm__("mov $1, %eax");
    __asm__("mov $1, %eax");
    return 123;
}