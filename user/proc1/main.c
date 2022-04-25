#define WITH_SEM
#include "../tests/lib/sysapi.h"

int main()
{
    cons_write("Hello from virtual address process\n",
               sizeof("Hello from virtual address process\n"));
    return getprio(0);
    //int x = *(int *)0x42;
    // should crash
}