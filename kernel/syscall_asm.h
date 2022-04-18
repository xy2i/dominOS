#ifndef __SYSCALL_ASM_H__
#define __SYSCALL_ASM_H__

int syscall_0(int syscall_number);
int syscall_1(int syscall_number, int arg1);
int syscall_2(int syscall_number, int arg1, int arg2);
int syscall_3(int syscall_number, int arg1, int arg2, int arg3);
int syscall_4(int syscall_number, int arg1, int arg2, int arg3, int arg4);
int syscall_5(int syscall_number, int arg1, int arg2, int arg3, int arg4,
              int arg5);
int syscall_6(int syscall_number, int arg1, int arg2, int arg3, int arg4,
              int arg5, int arg6);

#endif
