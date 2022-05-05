#define WITH_SEM
#include "../tests/lib/sysapi.h"

extern void halt();

__inline__ static void cli(void)
{
    __asm__ __volatile__("cli" ::: "memory");
}

__inline__ static void sti(void)
{
    __asm__ __volatile__("sti" ::: "memory");
}

__inline__ static void hlt(void)
{
    __asm__ __volatile__("hlt" ::: "memory");
}

int main()
{
    //start("autotest", 4096, 2, NULL);
    start("test3", 4096, 128, NULL);
    //start("shell", 4096, 129, NULL);
    while (1) {
    }
}