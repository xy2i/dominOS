#include "segment.h"
#define USER_START 0x40000000
// USER_STACK_START - 8 for exit and arg (2 words)
#define USER_STACK_BEGIN 0xfffffff8

// void goto_user_mode(uint32_t[] old)
// Save current register context in old
// This user mode switch is only used for the first process.
// We must simulate a fake interrupt and return.
.global goto_user_mode
goto_user_mode:
    // Save gp registers
    movl 4(%esp), %eax
    movl %ebx, (%eax)
    movl %esp, 4(%eax)
    movl %ebp, 8(%eax)
    movl %esi, 12(%eax)
    movl %edi, 16(%eax)
    // Save cr3
    movl 20(%eax), %ebx
    movl %cr3, %ebx

    // New regs
    movl 8(%esp), %eax

    // Restore cr3
    movl 20(%eax), %ebx
    movl %ebx, %cr3
    // Set tss->cr3
    movl %ebx, 0x2001c
    // Set tss->esp0
    movl 24(%eax), %ebx
    movl %ebx, 0x20004

    // ┌───────┐
    // │  SS   │
    // ├───────┤
    // │  ESP  │
    // ├───────┤
    // │ EFLAGS│
    // ├───────┤
    // │  CS   │
    // ├───────┤
    // │  EIP  │
    // └───────┘
    mov $USER_DS, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    pushl $USER_DS        // SS
    pushl $USER_STACK_BEGIN // ESP
    pushf                 // EFLAGS

    // Set IF on eflags
    orl $0x200, (%esp)

    push $USER_CS      // CS
    pushl $USER_START  // EIP

    iret