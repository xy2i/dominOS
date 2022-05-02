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
    for (;;) {
        cons_write("Hello, idle!\n", sizeof("Hello, idle!\n"));
        start("proc1", 4096, 2, NULL);
    }
    while (1) {
        halt();
    }
}