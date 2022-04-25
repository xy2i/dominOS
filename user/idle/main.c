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
        sti();
        hlt();
        cli();
    }
}
