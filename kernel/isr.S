
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

