/**
 * Clock handling.
 *
 * This will run a function defined in this file, tic_PIT, every tick of the
 * PIT.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "cga.h"
#include "clock.h"
#include "cpu.h"
#include "segment.h"
#include "task.h"

/**
 * Frequency of the clock in Hz.
 * Change this if you want to trigger ticks more/less frequently.
 */
#define CLOCKFREQ               50
/**
 * IRQ definition.
 */
#define IRQ_MASK_DATA_PORT      0x21
#define CLOCK_IRQ               0
/**
 * Quartz of the processor.
 */
#define QUARTZ                  0x1234DD

/**
 * Interrupt number for the clock.
 */
#define CLOCK_INT               32
/**
 * Base address for the interrupt vector table.
 */
#define IVT_BASE_ADDR     0x1000

uint32_t total_ticks = 0;
uint32_t total_seconds = 0;

uint32_t ticks = 0;
uint8_t hours, minutes, seconds = 0;

uint32_t clock_freq;

extern void handler_IT_32();

/**
 * This function is called by handler_IT_32,
 * defined in handlers.S
 */
__attribute__((used)) void tic_PIT() {
    // Acquit interrupt
    outb(0x20, 0x20);

    //scheduler();

    // Display time
    total_ticks++;
    ticks++;
    if (ticks == clock_freq) {
        ticks = 0;
        seconds++;
        total_seconds++;

        if (seconds == 60) {
            seconds = 0;
            minutes++;

            if (minutes == 60) {
                minutes = 0;
                hours++;

                if (hours == 24)
                    seconds = hours = minutes = 0;
            }
        }

        char str[30]; // max size, increase for a bigger string
        int size = sprintf(str, "uptime: %02d:%02d:%02d", hours, minutes, seconds);
        console_putbytes_topright(str, size);
    }
}

void init_handler_IT(int32_t num_IT, void (*handler)(void)) {
    uint32_t handler_addr = (uint32_t) handler;

    uint64_t *ivt = (uint64_t *) IVT_BASE_ADDR;
    uint16_t *ivt_entry = (uint16_t *) &ivt[num_IT];

    // Little endian
    *ivt_entry++ = handler_addr & 0xffff;
    *ivt_entry++ = KERNEL_CS;
    *ivt_entry++ = 0x8f00;
    *ivt_entry = handler_addr >> 16;
}

void set_clock_freq(uint32_t freq) {
    clock_freq = freq;
    uint16_t value = (QUARTZ / clock_freq);
    outb(0x34, 0x43);
    outb(value & 0xFF, 0x40);
    outb(value >> 8, 0x40);
}

void masque_IRQ(uint32_t num_IRQ, bool masque) {
    uint8_t irq_mask = inb(IRQ_MASK_DATA_PORT);

    if (masque)
        irq_mask |= 1 << num_IRQ;
    else
        irq_mask &= ~(1 << num_IRQ);

    outb(irq_mask, IRQ_MASK_DATA_PORT);
}

void init_clock() {
    set_clock_freq(CLOCKFREQ);
    init_handler_IT(CLOCK_INT, handler_IT_32);
    masque_IRQ(CLOCK_IRQ, false);

    //sti(); // Enable interrupts (may need to comment this out later)
}