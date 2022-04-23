/**
 * Implements paging and allows mapping virtual to physical addresses.
 *
 * In x86, we have access to a two tier paging system. A virtual adress within
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
#include "kalloc.h"
#include "debug.h"
#include "string.h"

// A page is 4Kb (0x1000)
#define PAGE_SIZE 0x1000

// Flags
// Entry present in page table/directory
#define NONE 0x0
#define PRESENT 0x1
// Read write page
#define RW 0x2
// Page accessible in user mode if true, otherwhise only kernel mode page
#define US 0x4

// Early page directory from early_mm.c
extern uint32_t pgdir[];

// Page directory must be 4KB aligned.
uint32_t page_directory[1024] __attribute__((aligned(PAGE_SIZE)));

/**
 * Maps the specified page with given flags.
 * The present flag is set by this function.
 * @pre start and end must be 4K-aligned.
 * @param virt_addr The virtual adress to map
 * @param phy_addr The physical adress to map it to
 * @param flags Flags to set on the page
 */
void map_page(uint32_t virt_addr, uint32_t phy_addr, uint32_t flags)
{
    // First 10 bits: page directory (bits 31-22)
    uint32_t pd_index = virt_addr >> 22;
    // Next 12 bits: page table (bits 21-12)
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;

    // Check whether a page table entry is present
    if (((uint32_t)page_directory[pd_index] & PRESENT) == 0) {
        // If it's not, we'll create a new page table
        pgdir[pd_index] = kalloc(PAGE_SIZE) | flags | PRESENT;
    }

    // Get the page table adress: only upper 20 bits, bits 31-10
    uint32_t *page_table = (uint32_t *)(pgdir[pd_index] & 0xFFFFF000);
    // Set the physical adress in the page table with flags
    page_table[pt_index] = phy_addr | flags | PRESENT;
}

/**
 * Map a zone of virtual adresses to a zone of physical adresses.
 * A zone is a range of memory (start-end).
 * @pre both zones must be the same size
 * @param flags Flags to set on all pages
 */
void map_zone(uint32_t virt_start, uint32_t virt_end, uint32_t phy_start,
              uint32_t phy_end, uint32_t flags)
{
    if (virt_end - virt_start != phy_end - phy_start) {
        panic("map_zone physical and virtual zones must be the same size!");
    }
    for (uint32_t virt = virt_start, phy = phy_start; virt < virt_end;
         virt += PAGE_SIZE, phy += PAGE_SIZE) {
        map_page(virt, phy, flags);
    }
}

void map_zone_identity(uint32_t start, uint32_t end, uint32_t flags)
{
    for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
        map_page(addr, addr, flags);
    }
}

void initialise_paging()
{
    // For the first 64 entries, the project has set up page tables for us,
    // in the pgdir[] variable. Following the advice at
    // https://ensiwiki.ensimag.fr/index.php?title=Projet_syst%C3%A8me_:_Aspects_techniques#Pagination,
    // we copy them in our page directory.
    for (int i = 0; i < 64; i++) {
        page_directory[i] = pgdir[i];
    }

    // Test: map the null pointer (in reality 0x0 to 0x1000) to be accessible and read/write.
    map_zone_identity(0x0, 0x1000, RW | US);

    // Switch to our own page directory.
    // Paging is already enabled by early_mm, so just have to switch to our pagedir.
    // TODO: one page dir per process
    __asm__ volatile("mov %0, %%cr3" ::"r"(page_directory));
}