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

# Structure de la pile en entree :
#   %esp + 4 : adresse de l'ancien contexte
#   %esp + 8 : adresse du nouveau contexte

.globl swtch
swtch:
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

    // Switch stacks
    movl 8(%esp), %eax
    movl 4(%eax), %esp

    // Switch CR3
    movl 20(%eax), %ebx
    movl %ebx, %cr3

    // Restore general purpose registers
    movl (%eax), %ebx
    movl 8(%eax), %ebp
    movl 12(%eax), %esi
    movl 16(%eax), %edi


    ret
