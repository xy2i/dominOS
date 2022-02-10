#
#   void swtch(struct cpu_context ** old, struct cpu_context * new)
#

.globl swtch
swtch:
    movl 8(%esp), %edx # Load new
    movl 4(%esp), %eax # Load old

    # Save registers
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

    # Switch stacks
    movl %esp, (%eax)
    movl %edx, %esp # Switch to new

    # Load new register
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret
