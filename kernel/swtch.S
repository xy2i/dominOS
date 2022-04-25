#include "segment.h"
#define USER_START 0x40000000
#define USER_STACK_END 0xffffffff

// Context switch
//
//  void swtch(struct cpu_context **old, struct cpu_context *new, uint32_t* new_page_dir, uint32_t* stack_addr);
//
// Save current register context in old
// and then load register context from new.
// Switches from the context's current page directory (in the %cr3 register)
// to the one of the new process, switching the virtual address space.

.globl swtch
swtch:
    cli // disable interrupts

    # Save old callee-save registers
    movl 4(%esp), %eax
    movl 8(%esp), %edx
    movl 12(%esp), %ecx
    pushl %ebp
    pushl %ebx
    movl 24(%esp), %ebx  // Hack to get access to ebx (2 pushes, so +8)
    pushl %esi
    pushl %edi

    # Switch virtual address space
    movl %ecx, %cr3

    // Switch stacks, to restore saved registers
    movl %esp, (%eax)
    movl %edx, %esp

    // Conserve the stack address, just before popping ebx
    movl %ebx, %ecx     // ecx = %esp
    # Load new callee-save registers
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp

    // Switch back to first stack.
    movl 0(%eax), %esp

    // Perform user-mode switch
    // Stack layout prior to iret:
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
    // Note that the two last values will not be popped if we restore to kernel,
    // if passing KERNEL_DS and KERNEL_CS instead of USER_DS and USER_CS: "
    // INT does only push SS and ESP if a privilege switch happens, and that means going from Ring3 => Ring0,
    // otherwise the information is not pushed.
    // IRET works exactly the same way, it ONLY restores SS and ESP if a ring switch happens,
    // and that means from Ring0 => Ring3".
    // https://forum.osdev.org/viewtopic.php?p=271343&sid=ca2895885793686bb027c9dbb9deeaf1#p271343
    mov $USER_DS, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    pushl $USER_DS     // SS
    pushl %ecx         // ESP
    pushf             // EFLAGS

    // Set IF on eflags undo the cli at the start, when the iret is executed
    orl $0x200, (%esp)

    // Set TSS so that register will restore from these values
    // We set both esp0 and ss0, since we are returning from a ring0 "interrupt"
   // mov %ecx, (0x20004) // tss->esp0
   // mov $USER_DS, (0x20008) // tss->ss0

    push $USER_CS  // CS
    pushl $USER_START // EIP
    iret
