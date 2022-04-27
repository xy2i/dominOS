#define WITH_SEM
#include "../tests/lib/sysapi.h"

int main()
{
    for (int i = 0; i < 20; i++) {
        int pid = start("proc1", 0x401, 1, NULL);
        int retval = 0;
        waitpid(pid, &retval);
        cons_write("hello", sizeof("hello"));
    }
    return 0;
}