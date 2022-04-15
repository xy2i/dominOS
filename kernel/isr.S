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
    add $4, %esp            # Remove error code
    popl %ecx
    popl %edx
    popl %eax
    iret

# int $49
.globl syscall_isr
syscall_isr:
    push %es
    push %ds
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    call syscall_handler
    # eax contains return value of syscall
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    pop %ds
    pop %es
    iret