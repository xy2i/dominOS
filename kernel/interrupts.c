#include <stdint.h>

struct idt_entry {
    uint32_t isr_low          :16;
    uint32_t segment_selector :16;
    uint32_t ignored          :8;
    uint32_t gate_type        :4;
    uint32_t always_0         :1;
    uint32_t privilege_level  :2;
    uint32_t present          :1;
    uint32_t isr_high         :16;
} __attribute__((__packed__));

static uint64_t sidt(void)
{
    uint64_t idtr = 0;
    __asm__("sidt %0" : "=m"(idtr));
    return idtr;
}

static uint32_t idt_adress(void)
{
    return (sidt() & 0xffffffff0000) >> 16;
}

uint64_t * gate_adress(uint16_t offset)
{
    return (uint64_t *)(idt_adress() + 8 * offset);
}

void fill_gate(uint64_t * entry,
               uint32_t   isr,
               uint16_t   segment_selector,
               uint8_t    privilege_level,
               uint8_t    gate_type)
{
    struct idt_entry * idt_entry;
    
    idt_entry                   = (struct idt_entry *) entry;
    idt_entry->isr_low          = isr & 0x0000ffff;
    idt_entry->segment_selector = segment_selector;
    idt_entry->gate_type        = gate_type;
    idt_entry->always_0         = 0;
    idt_entry->privilege_level  = privilege_level;
    idt_entry->present          = 1;
    idt_entry->isr_high         = (isr & 0xffff0000) >> 16;
}