#include "sysapi.h"

int suicide(void *arg)
{
    (void)arg;
    kill(getpid());
    assert(0);
    return 0;
}
