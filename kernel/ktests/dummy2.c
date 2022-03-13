#include "test1.h"
#include "sysapi.h"

int dummy2(void *arg)
{
        printf(" 5");
        assert((int) arg == DUMMY_VAL + 1);
        return 4;
}
