#define WITH_MSG
#define CONS_READ_LINE
#include "../tests/lib/sysapi.h"

int main()
{
    cons_write("Hello from virtual address process\n",
               sizeof("Hello from virtual address process\n"));
    cons_read("test",10);
    cons_echo(1);
    return getprio(0);
    //int x = *(int *)0x42;
    // should crash
}