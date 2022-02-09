#ifndef __SWTCH__
#define __SWTCH__

//  EAX, ECX, EDX are caller saved.
struct cpu_context {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebx;
  uint32_t ebp;
  uint32_t eip;
};

void swtch(struct cpu_context ** old, struct cpu_context ** new);

#endif