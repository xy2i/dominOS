#include "segment.h"

.globl clock_isr
clock_isr:
    pushl %eax
    pushl %edx
    pushl %ecx
    call clock_handler
    popl %ecx
    popl %edx
    popl %eax
    iret

.globl page_fault_isr
page_fault_isr:
    pushl %eax
    pushl %edx
    pushl %ecx
    call page_fault_handler
    popl %ecx
    popl %edx
    popl %eax
    iret

.globl keyboard_isr
keyboard_isr:
    pushl %eax
    pushl %edx
    pushl %ecx
    inb $0x60,%al
    call do_scancode
    popl %ecx
    popl %edx
    popl %eax
    iret