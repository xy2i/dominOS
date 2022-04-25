
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
    mov 0x0C(%esp), %ecx
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