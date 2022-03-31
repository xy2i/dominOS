#include "segment.h"

.global goto_user_mode
goto_user_mode:
    movl $USER_DS, %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    pushl $USER_DS   // SS
    pushl 12(%esp)   // ESP
    pushf            // EFLAGS
    pushl $USER_CS   // CS
    pushl 20(%esp)   // EIP

    iret