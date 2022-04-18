/**
 * Describes the interrupt descriptor table,
 * and provides a mechanism to register an interrupt handler.
 */

#include <stdint.h>
#include "segment.h"

#define IDT_ADDR 0x1000
#define INTERRUPT_GATE 0x0E
#define RING3 0x03

// https://wiki.osdev.org/Interrupt_Descriptor_Table
struct idt_entry {
    uint32_t isr_low : 16;
    uint32_t segment_selector : 16;
    uint32_t ignored : 8;
    uint32_t gate_type : 4;
    uint32_t always_0 : 1;
    uint32_t privilege_level : 2;
    uint32_t present : 1;
    uint32_t isr_high : 16;
} __attribute__((__packed__));

static uint32_t *get_idt_entry(uint16_t index)
{
    return (uint32_t *)(IDT_ADDR + 8 * index);
}

/* Please call this function with an isr (interrupt service routine) that
 * does an iret at the end. See isr.S for some examples.
 */
void register_interrupt_handler(int number, void (*handler)(void))
{
    struct idt_entry *entry = (struct idt_entry *)get_idt_entry(number);
    entry->isr_low          = (uint32_t)handler & 0x0000ffff;
    entry->segment_selector = KERNEL_CS;
    entry->gate_type        = INTERRUPT_GATE;
    entry->privilege_level  = RING3;
    entry->present          = 1;
    entry->isr_high         = ((uint32_t)handler & 0xffff0000) >> 16;
}