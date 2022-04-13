#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <stdarg.h>

void syscall(int syscall_number, int nbr_args, ...);

#endif
