#include "syscall.h"
#include "stdio.h"
#include "syscall_asm.h"

// syscall handlers are in syscall.h
#define SYSCALL_1(syscall_number, arg1) syscall_1(syscall_number, (void *)arg1)

int ggetprio(int pid)
{
    return (int)SYSCALL_1(2, pid);
}