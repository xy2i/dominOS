#include "syscall.h"

void syscall(int syscall_number, int nbr_args, ...)
{
    va_list ap;
    va_start(ap, nbr_args);



    va_end(ap);
}
