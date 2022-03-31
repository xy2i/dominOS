#ifndef __MEMORY_H__
#define __MEMORY_H___

struct vm_area;
struct mm;

void             switch_virtual_adress_space(struct mm * mm);
struct vm_area * alloc_vm_area(uint32_t start,
                               uint32_t end,
                               uint32_t writeable,
                               uint32_t user_accessible);
struct mm *      alloc_mm(void);
void             free_mm(struct mm * mm);
int              add_vm_area(struct mm * mm, struct vm_area * vm_area);
void             map_vm_area(struct mm * mm, struct vm_area * vm_area);
void             unmap_vm_area(struct mm * mm, struct vm_area * vm_area);
void             do_kernel_mapping(struct mm * mm);
void             init_page_fault_handler(void);

#endif