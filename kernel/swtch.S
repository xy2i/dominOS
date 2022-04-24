# Context switch
#
#   void swtch(struct cpu_context **old, struct cpu_context *new, uint32_t* new_page_dir);
# 
# Save current register context in old
# and then load register context from new.
# Switches from the context's current page directory (in the %cr3 register)
# to the one of the new process, switching the virtual address space.

.globl swtch
swtch:
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