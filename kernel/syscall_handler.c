#include "primitive.h"
#include "interrupts.h"
#include "isr.h"
#include "syscall_handler.h"

void syscall_handler()
{
    // Reserve registers for local use, so that GCC does not overwrite them.
    register int *eax __asm__("eax") __attribute__((unused));
    register int *ebx __asm__("ebx") __attribute__((unused));
    register int *ecx __asm__("ecx") __attribute__((unused));
    register int *edx __asm__("edx") __attribute__((unused));
    register int *esi __asm__("esi") __attribute__((unused));
    register int *edi __asm__("edi") __attribute__((unused));
    register int *ebp __asm__("ebp") __attribute__((unused));

    int syscall_number;
    __asm__("mov %%eax, %0" : "=r"(syscall_number));

    switch (syscall_number) {
    case 1:
        break;
    case 2: {
        int pid;
        __asm__("mov %%ebx , %0" : "=r"(pid));
        getprio(pid);
        int xx;
        __asm__("mov %%eax , %0" : "=r"(xx));
    }
    }
}

void init_syscall_handler(void)
{
    register_interrupt_handler(49, syscall_handler);
}
