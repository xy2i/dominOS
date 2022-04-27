#ifndef __ISR_H__
#define __ISR_H__

void clock_isr(void);
void keyboard_isr(void);
void page_fault_isr(void);
void syscall_isr(void);

#endif
