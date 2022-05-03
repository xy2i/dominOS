// Declared in syscall_handler.h
#include "segment.h"

.data
.globl syscalls

.text
// int $49
.globl syscall_isr
syscall_isr:
	// Save registers
    // Do not save eax, as it will contain the syscall value
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

    // Set kernel privilege
    mov $KERNEL_DS, %cx
    movw %cx, %ds
    movw %cx, %es
    movw %cx, %fs
    movw %cx, %gs
    sti

    /* if (eax >= num_syscalls) {
         return;
       }
    */
    movl    num_syscalls, %ebx   // ebx = num_syscalls
    cmp     %ebx, %eax             // eax >= num_syscalls
    jge     1f

    /* void* syscall_function = syscalls[num_syscall]; */
    lea     syscalls, %ebx      // ebx = &syscalls
    // eax is the index of the array
    // since the size of each element in the array is 4 bytes (array of pointers), add 4
    lea     (%ebx,%eax,4), %ebx // ebx = ebx + index * 4
    movl    0(%ebx), %ebx       // ebx = *ebx

    /* syscall_function(); */
    call    %ebx

1:
    // Set user privilege
    mov $USER_DS, %cx
    movw %cx, %ds
    movw %cx, %es
    movw %cx, %fs
    movw %cx, %gs
    // Restore registers
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp

    iret