#include "task.h"

int getpid(void)
{
    return current()->pid;
}