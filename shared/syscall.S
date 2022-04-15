.globl syscall_1
syscall_1:
    mov 0x08(%ebp), %eax
    mov 0x0C(%ebp), %ebx
    int $49

.globl syscall_2
syscall_2:
    mov 0x08(%ebp), %eax
    mov 0x0C(%ebp), %ebx
    mov 0x10(%ebp), %ecx
    int $49

.globl syscall_3
syscall_3:
    mov 0x08(%ebp), %eax
    mov 0x0C(%ebp), %ebx
    mov 0x10(%ebp), %ecx
    mov 0x14(%ebp), %edx
    int $49

.globl syscall_4
syscall_4:
    mov 0x08(%ebp), %eax
    mov 0x0C(%ebp), %ebx
    mov 0x10(%ebp), %ecx
    mov 0x14(%ebp), %edx
    mov 0x18(%ebp), %esi
    int $49

.globl syscall_5
syscall_5:
    mov 0x08(%ebp), %eax
    mov 0x0C(%ebp), %ebx
    mov 0x10(%ebp), %ecx
    mov 0x14(%ebp), %edx
    mov 0x18(%ebp), %esi
    mov 0x1C(%ebp), %edi
    int $49

.globl syscall_6
syscall_6:
    mov 0x08(%ebp), %eax
    mov 0x0C(%ebp), %ebx
    mov 0x10(%ebp), %ecx
    mov 0x14(%ebp), %edx
    mov 0x18(%ebp), %esi
    mov 0x1C(%ebp), %edi
    mov 0x20(%ebp), %ebp
    int $49