#ifndef __PIC_H__
#define __PIC_H__

void EOI(uint8_t irq);
void mask_IRQ(uint8_t irq);
void unmask_IRQ(uint8_t irq);

#endif
