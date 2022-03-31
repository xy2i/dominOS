#ifndef __MEMORY_H__
#define __MEMORY_H___

struct vm_area;
struct mm;

void        switch_virtual_adress_space(struct mm * mm);
struct mm * alloc_mm(void);
void        free_mm(struct mm * mm);
void        map_vm_area(struct mm * mm, struct vm_area * vm_area);
void        unmap_vm_area(struct mm * mm, struct vm_area * vm_area);
void        do_kernel_mapping(struct mm * mm);
void        init_page_fault_handler(void);

#endif