#include <stdint.h>

void fill_gate(uint64_t * entry,
               uint32_t   isr,
               uint16_t   selector,
               uint8_t    privilege_level,
               uint8_t    gate_type,
               uint8_t    present)
{
    uint32_t lower_part;
    uint32_t higher_part;

    lower_part = selector << 16;
    lower_part |= isr & 0xffff;
    higher_part = isr & 0xffff0000;
    higher_part |= (present & 0x01) << 15;
    higher_part |= (privilege_level & 0x03) << 13;
    higher_part |= (gate_type & 0x0f) << 8;

    *entry = ((uint64_t) higher_part) << 32 | lower_part;
}

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