#ifndef __SYSCALL_ASM_H__
#define __SYSCALL_ASM_H__

void syscall_0(int syscall_number);
void syscall_1(int syscall_number, void *arg1);
void syscall_2(int syscall_number, void *arg1, void *arg2);
void syscall_3(int syscall_number, void *arg1, void *arg2, void *arg3);
void syscall_4(int syscall_number, void *arg1, void *arg2, void *arg3,
               void *arg4);
void syscall_5(int syscall_number, void *arg1, void *arg2, void *arg3,
               void *arg4, void *arg5);
void syscall_6(int syscall_number, void *arg1, void *arg2, void *arg3,
               void *arg4, void *arg5, void *arg6);

#endif
