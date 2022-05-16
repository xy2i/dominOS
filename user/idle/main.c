#define WITH_SEM
#include "../tests/lib/sysapi.h"

int main()
{
    //start("autotest", 4096, 2, NULL);
    //start("test3", 4096, 128, NULL);
    start("shell", 4096, 2, NULL);
    while (1) {
    }
}