// Declared in syscall_handler.h

.data
.globl syscalls

.text
// int $49
.globl syscall_isr
syscall_isr:
	// Save registers
    // Do not save eax, as it will contain the syscall value
    push %es
    push %ds
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
	pushl %ebx
	// Push parameters on the stack again, for function call
	// We need to do this again, as the by value arguments may be modified by the calling function
	// https://stackoverflow.com/questions/30284914/x86-calling-convention-should-arguments-passed-by-stack-be-read-only
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx

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
    // Pop parameters
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    // Restore registers
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    pop %ds
    pop %es
    iret

.globl syscall_0
syscall_0:
    mov 0x04(%esp), %eax
    int $49
    ret

.globl syscall_1
syscall_1:
    mov 0x04(%esp), %eax
    mov 0x08(%esp), %ebx
    int $49
    ret

.globl syscall_2
syscall_2:
    mov 0x04(%esp), %eax
    mov 0x08(%esp), %ebx
    mov 0x08(%esp), %ecx
    int $49
    ret

.globl syscall_3
syscall_3:
    mov 0x04(%esp), %eax
    mov 0x08(%esp), %ebx
    mov 0x0C(%esp), %ecx
    mov 0x10(%esp), %edx
    int $49
    ret

.globl syscall_4
syscall_4:
    mov 0x04(%esp), %eax
    mov 0x08(%esp), %ebx
    mov 0x0C(%esp), %ecx
    mov 0x10(%esp), %edx
    mov 0x14(%esp), %esi
    int $49
    ret

.globl syscall_5
syscall_5:
    mov 0x04(%esp), %eax
    mov 0x08(%esp), %ebx
    mov 0x0C(%esp), %ecx
    mov 0x10(%esp), %edx
    mov 0x14(%esp), %esi
    mov 0x18(%esp), %edi
    int $49
    ret

.globl syscall_6
syscall_6:
    mov 0x04(%esp), %eax
    mov 0x08(%esp), %ebx
    mov 0x0C(%esp), %ecx
    mov 0x10(%esp), %edx
    mov 0x14(%esp), %esi
    mov 0x18(%esp), %edi
    mov 0x1C(%esp), %ebp
    int $49
    ret