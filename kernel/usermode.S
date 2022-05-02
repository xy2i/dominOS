#include "segment.h"
#define USER_START 0x40000000
// USER_STACK_START - 8 for exit and arg (2 words)
#define USER_STACK_BEGIN 0xfffffff8

// This user mode switch is only used for the first process.
// We must simulate a fake interrupt and return.
.global goto_user_mode
goto_user_mode:
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