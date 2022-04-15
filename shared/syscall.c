#include "syscall.h"
#include "stdio.h"

// syscall handlers are in syscall.h
extern void syscall_1(int syscall_number, void *arg1);
#define SYSCALL_1(syscall_number, arg1) syscall_1(syscall_number, (void *)arg1)

int ggetprio(int pid)
{
    printf("%d", pid);
    //syscall_1(2, (void *)pid);
    return 0;
}