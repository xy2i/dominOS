#include "fat32.h"
#include "stdint.h"

// https://wiki.osdev.org/FAT
struct BPB {
    uint8_t jmp_boot[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sector_per_cluster;
    uint16_t nbr_reserved_sectors;
    uint8_t nbr_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_in_volume;
    uint8_t media;
    uint16_t nbr_sectors_per_fat;
    uint16_t nbr_sectors_per_track;
    uint16_t nbr_heads;
    uint32_t nbr_hidden_sectors;
    uint32_t nbr_large_sectors;
    uint32_t nbr_sectors_per_fat32;
    uint32_t extension_flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fsinfo_sector;
    uint16_t book_sector_backup;
    uint32_t reserved[3];
    uint8_t drive_nbr;
    uint8_t win_nt_reserved;
    uint8_t signature;
    uint32_t volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[420];
    uint16_t boot_signature;
} __attribute__((__packed__));

