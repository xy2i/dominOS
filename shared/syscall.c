
#include "syscall.h"

#define syscall_0(syscall_number, arg1)                                        \
    ({                                                                         \
        __asm__ __volatile__("movl %%eax,%0" ::"a"(syscall_number));           \
        __asm__ __volatile__("int $49");                                       \
    })

#define syscall_1(syscall_number, arg1)                                        \
    ({                                                                         \
        __asm__ __volatile__("movl %%eax,%0" ::"a"(syscall_number));           \
        __asm__ __volatile__("movl %%ebx,%0" ::"a"(arg1));                     \
        __asm__ __volatile__("int $49");                                       \
    })

#define syscall_2(syscall_number, arg1, arg2)                                  \
    ({                                                                         \
        __asm__ __volatile__("movl %%eax,%0" ::"a"(syscall_number));           \
        __asm__ __volatile__("movl %%ebx,%0" ::"a"(arg1));                     \
        __asm__ __volatile__("movl %%ecx,%0" ::"a"(arg2));                     \
        __asm__ __volatile__("int $49");                                       \
    })

#define syscall_3(syscall_number, arg1, arg2, arg3)                            \
    ({                                                                         \
        __asm__ __volatile__("movl %%eax,%0" ::"a"(syscall_number));           \
        __asm__ __volatile__("movl %%ebx,%0" ::"a"(arg1));                     \
        __asm__ __volatile__("movl %%ecx,%0" ::"a"(arg2));                     \
        __asm__ __volatile__("movl %%edx,%0" ::"a"(arg3));                     \
        __asm__ __volatile__("int $49");                                       \
    })

#define syscall_4(syscall_number, arg1, arg2, arg3, arg4)                      \
    ({                                                                         \
        __asm__ __volatile__("movl %%eax,%0" ::"a"(syscall_number));           \
        __asm__ __volatile__("movl %%ebx,%0" ::"a"(arg1));                     \
        __asm__ __volatile__("movl %%ecx,%0" ::"a"(arg2));                     \
        __asm__ __volatile__("movl %%edx,%0" ::"a"(arg3));                     \
        __asm__ __volatile__("movl %%esi,%0" ::"a"(arg4));                     \
        __asm__ __volatile__("int $49");                                       \
    })

#define syscall_5(syscall_number, arg1, arg2, arg3, arg4, arg5)                \
    ({                                                                         \
        __asm__ __volatile__("movl %%eax,%0" ::"a"(syscall_number));           \
        __asm__ __volatile__("movl %%ebx,%0" ::"a"(arg1));                     \
        __asm__ __volatile__("movl %%ecx,%0" ::"a"(arg2));                     \
        __asm__ __volatile__("movl %%edx,%0" ::"a"(arg3));                     \
        __asm__ __volatile__("movl %%esi,%0" ::"a"(arg4));                     \
        __asm__ __volatile__("movl %%edi,%0" ::"a"(arg5));                     \
        __asm__ __volatile__("int $49");                                       \
    })

#define syscall_6(syscall_number, arg1, arg2, arg3, arg4, arg5, arg6)          \
    ({                                                                         \
        __asm__ __volatile__("movl %%eax,%0" ::"a"(syscall_number));           \
        __asm__ __volatile__("movl %%ebx,%0" ::"a"(arg1));                     \
        __asm__ __volatile__("movl %%ecx,%0" ::"a"(arg2));                     \
        __asm__ __volatile__("movl %%edx,%0" ::"a"(arg3));                     \
        __asm__ __volatile__("movl %%esi,%0" ::"a"(arg4));                     \
        __asm__ __volatile__("movl %%edi,%0" ::"a"(arg5));                     \
        __asm__ __volatile__("movl %%ebp,%0" ::"a"(arg6));                     \
        __asm__ __volatile__("int $49");                                       \
    })

int getprio(int pid)
{
    syscall_1(2, pid);
    return 0;
}