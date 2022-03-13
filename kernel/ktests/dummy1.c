#include "test1.h"
#include "sysapi.h"

int dummy1(void *arg)
{
        printf("1");
        assert((int) arg == DUMMY_VAL);
        return 3;
}
