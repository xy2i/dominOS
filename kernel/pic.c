#include <stdint.h>
#include "cpu.h"

#define MASTER_PIC_CMD  0x20
#define MASTER_PIC_DATA 0x21
#define SLAVE_PIC_CMD   0xA0
#define SLAVE_PIC_DATA  0xA1

#define PIC_EOI         0x20

void EOI(uint8_t irq)
{
    if (irq >= 8)
        outb(SLAVE_PIC_CMD, PIC_EOI);

    outb(MASTER_PIC_CMD, PIC_EOI);
}

void mask_IRQ(uint8_t irq)
{
    uint16_t port;
    
    port = irq < 8 ? MASTER_PIC_DATA : SLAVE_PIC_DATA;
    irq =  irq < 8 ? irq             : irq - 8;
    outb(port, inb(port) & (1 << irq));
}

void unmask_IRQ(uint8_t irq)
{
    uint16_t port;
    
    port = irq < 8 ? MASTER_PIC_DATA : SLAVE_PIC_DATA;
    irq =  irq < 8 ? irq             : irq - 8;
    outb(port, inb(port) & ~(1 << irq));
}