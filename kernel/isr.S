
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