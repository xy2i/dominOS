#define WITH_SEM
#include "../tests/lib/sysapi.h"

int main()
{
    cons_write("proc2", sizeof("proc2"));
    return 0;
}