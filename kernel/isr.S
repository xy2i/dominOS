#include "segment.h"

.globl clock_isr
clock_isr:
    pushl %eax
    pushl %edx
    pushl %ecx
    // Set kernel privilege
    mov $KERNEL_DS, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    call clock_handler

    // Set user privilege
    mov $USER_DS, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    popl %ecx
    popl %edx
    popl %eax

    iret

.globl page_fault_isr
page_fault_isr:
    pushl %eax
    pushl %edx
    pushl %ecx
    // Set kernel privilege
    mov $KERNEL_DS, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    call page_fault_handler

    // Set user privilege
    mov $USER_DS, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    popl %ecx
    popl %edx
    popl %eax
    iret

.globl keyboard_isr
keyboard_isr:
    pushl %eax
    pushl %edx
    pushl %ecx

    // Set kernel privilege
    mov $KERNEL_DS, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    inb $0x60,%al
    call keyboard_handler

    // Set user privilege
    mov $USER_DS, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    popl %ecx
    popl %edx
    popl %eax
    iret