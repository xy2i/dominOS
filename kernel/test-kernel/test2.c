#include "task.h"

int procKill(void *args)
{
    printf(" X");
    return (int)args;
}

int procExit(void *args)
{
    printf(" 5");
    exit((int)args);
    assert(0);
    return 0;
}

int test2_main(void *arg)
{
    int rval;
    int r;
    int pid1;
    int val = 45;
    (void)arg;

    printf("1");
    pid1 = start(procKill, 4000, 100, "procKill", (void *)val);
    assert(pid1 > 0);
    printf(" 2");
    r = kill(pid1);
    assert(r == 0);
    printf(" 3");
    r = waitpid(pid1, &rval);
    assert(rval == 0);
    assert(r == pid1);
    printf(" 4");
    pid1 = start(procExit, 4000, 192, "procExit", (void *)val);
    assert(pid1 > 0);
    printf(" 6");
    r = waitpid(pid1, &rval);
    assert(rval == val);
    assert(r == pid1);
    assert(waitpid(getpid(), &rval) < 0);
    printf(" 7.\n");
    return 0;
}
