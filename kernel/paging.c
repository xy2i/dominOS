/**
 * Implements paging and allows mapping virtual to physical addresses.
 *
 * In x86, we have access to a two tier paging system. A virtual address within
 * this system looks like this:
 *
 *
 * ┌────────┬──────────┬───────┐
 * │pd_index│pt_index  │offset │
 * └────────┴──────────┴───────┘
 *  ◄──────► ◄────────► ◄─────►
 *     10       10        12     size in bits
 *   [31-22]   [21-12]  [11-0]
 *
 * pt_index: Index of the address of table within the page directory.
 * pt_index: Index of the physical address page within the page table.
 * offset  : Offset within the page.
 *
 * The page directory has pointers to page tables, which have pointers to pages.
 *
 * Addresses of page directories and page tables MUST be 4KB aligned.
 * Because of this, the 12 lower bits will be 0.
 * Intel decided to use the lower 12 bits to store flags.
 * A page directory / page table entry  looks like this:
 *
 * ┌───────────────────┬───────┐
 * │       address     │ flags │
 * └───────────────────┴───────┘
 *  ◄─────────────────► ◄─────►
 *           20           12
 *        [31-12]        [11-0]
 *
 * Important flags:
 * - P/present : tells the CPU the page is present on the system
 * - RW: when set, the page can be read and written, else read-only
 * - US (user/supervisor): when set, page is readable by all, else only by kernel
 * See https://wiki.osdev.org/Paging for all flags.
 *
 * Note that because flags are set in the addresses, to get an address from a page
 * table / page directory,
 * you MUST mask the lower 12 bits, for example with `& 0xFFFFF000`.
 *
 * Useful resources:
 * https://wiki.osdev.org/Paging
 * https://www.youtube.com/watch?v=dn55T2q63RU video on two tier system
 */

#include "paging.h"
#include "string.h"
#include "debug.h"
#include "page_allocator.h"

// Align to page size.
#define ALIGN(addr) ((addr)&0xFFFFF000)
// Align to page size with rounding up. If it's already aligned, we don't
// need to round it up.
#define ALIGN_UP(addr) ((!((addr)&0xFFF) ? (addr) : ALIGN(addr)))

/**
 * Maps the specified page with given flags.
 * The present flag is set by this function.
 * @pre start and end must be 4K-aligned.
 * @param virt_addr The virtual adress to map
 * @param phy_addr The physical adress to map it to
 * @param flags Flags to set on the page
 */
void map_page(uint32_t *dir, uint32_t virt_addr, uint32_t phy_addr,
              uint32_t flags)
{
    // First 10 bits: page directory (bits 31-22)
    uint32_t pd_index = virt_addr >> 22;
    // Next 12 bits: page table (bits 21-12)
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;

    // Check whether a page table entry is present
    if (((uint32_t)dir[pd_index] & PRESENT) == 0) {
        // If it's not, we'll create a new page table
        dir[pd_index] = (uint32_t)alloc_physical_page(1) | flags | PRESENT;
    }

    // Get the page table adress: only upper 20 bits, bits 31-10
    uint32_t *page_table = (uint32_t *)(dir[pd_index] & 0xFFFFF000);
    // Set the physical adress in the page table with flags
    page_table[pt_index] = phy_addr | flags | PRESENT;
}

void map_zone(uint32_t *pdir, uint32_t virt_start, uint32_t virt_end,
              uint32_t phy_start, uint32_t phy_end, int align_virt_end, uint32_t flags)
{
    if (virt_end - virt_start != phy_end - phy_start) {
        panic("map_zone physical and virtual zones must be the same size!");
    }

    // Align virtual and physical adresses, if they are not already.
    phy_start  = ALIGN(phy_start);
    virt_start = ALIGN(virt_start);
    phy_end    = ALIGN_UP(phy_end);
    if (align_virt_end)
        virt_end   = ALIGN_UP(virt_end) - 1;

    for (uint32_t virt = virt_start, phy = phy_start; virt <= virt_end;
         virt += PAGE_SIZE, phy += PAGE_SIZE) {
        map_page(pdir, virt, phy, flags);
    }
}

uint32_t *page_directory_create()
{
    // Early page directory from early_mm.c
    extern uint32_t pgdir[];

    // Page directories and page tables must be 4Kb aligned.
    // Conveniently, they are the same table as a page, so we can reuse the page allocator.
    uint32_t *dir = (uint32_t *)alloc_physical_page(1);
    memset(dir, 0, PAGE_SIZE);

    // For the first 64 entries, the project has set up page tables for us,
    // in the pgdir[] variable. Following the advice at
    // https://ensiwiki.ensimag.fr/index.php?title=Projet_syst%C3%A8me_:_Aspects_techniques#Pagination,
    // we copy them in our page directory.
    for (int i = 0; i < 64; i++) {
        dir[i] = pgdir[i];
    }

    return dir;
}

void page_directory_destroy(uint32_t *dir)
{
    // The 64 first entries are shared between page directories of all processes,
    // so we must not free them explicitly.
    // Instead, free the other entries if they exist.
    for (int i = 64; i < 1024; i++) {
        if (((uint32_t)dir[i] & PRESENT) == 1) {
            uint32_t page_directory_address = dir[i] & 0xFFFFF000;
            free_physical_page((void *)page_directory_address, 1);
        }
    }

    free_physical_page((void *)dir, 1);
}