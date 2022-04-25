#include "segment.h"
#define USER_START 0x40000000
#define USER_STACK_END 0xffffffff

// Context switch
//
//  void swtch(struct cpu_context **old, struct cpu_context *new, uint32_t* new_page_dir);
//
// Save current register context in old
// and then load register context from new.
// Switches from the context's current page directory (in the %cr3 register)
// to the one of the new process, switching the virtual address space.

.globl swtch
swtch:
  cli
  movl 4(%esp), %eax
  movl 8(%esp), %edx
  movl 12(%esp), %ecx

  # Save old callee-save registers
  pushl %ebp
  pushl %ebx
  pushl %esi
  pushl %edi

  # Switch virtual address space
  movl %ecx, %cr3

  # Switch stacks
  movl %esp, (%eax)
  movl %edx, %esp

  # Load new callee-save registers
  popl %edi
  popl %esi
  popl %ebx
  popl %ebp

  ret
/*
  // Perform user-mode switch
  // Stack layout prior to iret:
  //┌───────┐
  //│   SS  │
  //├───────┤
  //│  ESP  │
  //├───────┤
  //│EFLAGS │
  //├───────┤
  //│  CS   │
  //├───────┤
  //│  EIP  │
  //└───────┘
  mov $KERNEL_CS, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs

  pushl $KERNEL_DS  // SS
  // Because we just pushed something onto the stack, the first parameter,
  // which was at 4(%esp), moves to 8(%esp).
  pushl 8(%esp)     // ESP
  pushf             // EFLAGS

  // Set IF on eflags.
  pop %eax
  orl $0x200, %eax   // set IF
  push %eax

  pushl $KERNEL_CS  // CS
  pushl $USER_START // EIP
  iret*/
