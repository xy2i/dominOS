#ifndef __PAGING_H__
#define __PAGING_H__

struct pde {
    uint32_t present            :1;
    uint32_t writeable          :1;
    uint32_t user_access        :1;
    uint32_t write_through      :1;
    uint32_t cache_disabled     :1;
    uint32_t accessed           :1;
    uint32_t ignored_1          :1;
    uint32_t page_size          :1;
    uint32_t ignored_2          :4;
    uint32_t page_table_address :20;
} __attribute__((__packed__));

struct pte {
    uint32_t present              :1;
    uint32_t writeable            :1;
    uint32_t user_access          :1;
    uint32_t write_through        :1;
    uint32_t cache_disabled       :1;
    uint32_t accessed             :1;
    uint32_t dirty                :1;
    uint32_t page_attribute_table :1;
    uint32_t global               :1;
    uint32_t ignored              :3;
    uint32_t page_frame_address   :20;
} __attribute__((__packed__));
void __enable_paging(void);
void __disable_paging(void);
void load_page_directory(struct pde * page_dir);
struct pde * empty_virtual_adress_space(void);

#endif