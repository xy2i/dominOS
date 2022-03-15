#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include "segment.h"

#define TASK_GATE          0x05
#define INTERRUPT_GATE_16  0x06
#define TRAP_GATE_16       0x07
#define INTERRUPT_GATE_32  0x0E
#define TRAP_GATE_32       0x0F
#define INTERRUPT_GATE     INTERRUPT_GATE_32
#define TRAP_GATE          TRAP_GATE_16

#define RING0              0X00
#define RING1              0x01
#define RING2              0x02
#define RING3              0x03

void fill_gate(uint64_t * entry,
               uint32_t   isr,
               uint16_t   selector,
               uint8_t    privilege_level,
               uint8_t    gate_type,
               uint8_t    present);

uint64_t * gate_adress(uint16_t offset);

#endif