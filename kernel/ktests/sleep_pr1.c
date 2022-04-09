#include "sysapi.h"

int sleep_pr1(void *arg)
{
    (void)arg;
    wait_clock(current_clock() + 2);
    printf(" not killed !!!");
    assert(0);
    return 1;
}
