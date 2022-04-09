#include "sysapi.h"

int suicide(void *arg);
int suicide_launcher(void *arg)
{
    int pid1;
    (void)arg;
    pid1 = start(suicide, 4000, 192, "suicide", 0);
    assert(pid1 > 0);
    return pid1;
}
