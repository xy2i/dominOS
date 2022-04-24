#define WITH_SEM
#include "../tests/lib/sysapi.h"

int main()
{
    return getprio(0);
    //int x = *(int *)0x42;
    // should crash
}