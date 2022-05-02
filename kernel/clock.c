#include <stdint.h>
#include "cpu.h"
#include "interrupts.h"
#include "isr.h"
#include "pic.h"
#include "task.h"

#define PIT_QUARTZ 0x1234DD
#define PIT_INTERRUPT_NUMBER 32
#define PIT_IRQ 0x00
#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_1 0x41
#define PIT_CHANNEL_3 0x42
#define PIT_CMD 0x43

uint32_t clock_frequency = 0;
uint32_t ticks           = 0;
uint8_t  seconds         = 0;
uint8_t  minutes         = 0;
uint8_t  hours           = 0;
uint32_t days            = 0;

static void set_clock_frequency(uint32_t hz)
{
    uint16_t divisor = PIT_QUARTZ / hz;
    outb(0x34, PIT_CMD);
    outb(divisor & 0xFF, PIT_CHANNEL_0);
    outb(divisor >> 8, PIT_CHANNEL_0);
    clock_frequency = hz;
}

void clock_handler(void)
{
    EOI(PIT_INTERRUPT_NUMBER);
    if (is_preempt_enabled())
        schedule();
}

void init_clock(void)
{
    set_clock_frequency(CLOCK_FREQUENCY);
    register_interrupt_handler(PIT_INTERRUPT_NUMBER, clock_isr);
    unmask_IRQ(PIT_IRQ);
}

void clock_settings(unsigned long *quartz, unsigned long *ticks)
{
    *quartz = (uint32_t)PIT_QUARTZ;
    *ticks  = (uint32_t)PIT_QUARTZ / clock_frequency;
}

uint32_t current_clock()
{
    return ticks;
}