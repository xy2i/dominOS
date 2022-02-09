#ifndef __SWTCH__
#define __SWTCH__

#include <stdint.h>

/*
 EAX, ECX, EDX are caller saved.
 TO DO...
 */
struct cpu_context {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebx;
  uint32_t ebp;
  uint32_t eip;
};

void swtch(struct cpu_context ** old, struct cpu_context ** new);

#endif