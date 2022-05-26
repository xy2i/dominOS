/**
 * FAT32 driver.
 *
 * https://wiki.osdev.org/FAT
 * http://www.osdever.net./documents/fatgen103.pdf - MS doc
 */

#include "fat32.h"
#include "stdint.h"

// https://wiki.osdev.org/FAT
struct BPB {
    // Original BPB
    uint8_t jmp_boot[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sector_per_cluster;
    uint16_t nbr_reserved_sectors;
    uint8_t nbr_fats;
    uint16_t nbr_root_entry;
    uint16_t nbr_sectors_in_volume;
    uint8_t media;
    uint16_t nbr_sectors_per_fat;
    uint16_t nbr_sectors_per_track;
    uint16_t nbr_heads;
    uint32_t nbr_hidden_sectors;
    uint32_t nbr_large_sectors;
    // FAT32 extension
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

struct FSInfo {
    uint32_t lead_signature;
    uint8_t reserved_1[480];
    uint32_t struc_signature;
    uint32_t last_free_cluster_in_volume;
    uint32_t last_used_cluster;
    uint8_t reserved_2[12];
    uint32_t trail_signature;
} __attribute__((__packed__));

struct DirEntry {
    uint8_t filename[11];

    union {
        struct {
            uint8_t read_only : 1;
            uint8_t hidden : 1;
            uint8_t system : 1;
            uint8_t volume_id : 1;
            uint8_t directory : 1;
            uint8_t archive : 1;
            uint8_t reserved1 : 2;
        };
        uint8_t attr;
    };

    uint8_t win_nt_reserved;

    // Values in tenths of ms, from 0-200
    uint8_t created_ticks;
    // Has a 2 second granularity, not 1 second!
    uint16_t created_second : 5;
    uint16_t created_minute : 6;
    uint16_t created_hour : 5;
    uint16_t created_day : 5;
    uint16_t created_month : 4;
    uint16_t created_year : 7;

    uint16_t accessed_day : 5;
    uint16_t accessed_month : 4;
    uint16_t accessed_year : 7;

    uint16_t cluster_hi;

    uint16_t modified_second : 5;
    uint16_t modified_minute : 6;
    uint16_t modified_hour : 5;

    uint16_t modified_day : 5;
    uint16_t modified_month : 4;
    uint16_t modified_year : 7;

    uint16_t cluster_lo;

    uint32_t size;
} __attribute__((__packed__));

#define LAST_LONG_ENTRY 0x40
// https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#VFAT_long_file_names
struct LongNameEntry {
    // Mask with LAST_LONG_ENTRY to know if this is the last entry in set
    uint8_t order;
    uint16_t filename_lo[5];
    // Must be Attr.LONG_NAME
    uint8_t attr;
    uint8_t type; // 0 = long entry
    uint8_t checksum_of_short_name;
    uint16_t filename_mid[6];
    uint16_t zero;
    uint16_t filename_hi[2];
} __attribute__((__packed__));

// For DIR_Attr, reversed since little endian
enum Attr {
    READ_ONLY = 0x80,
    HIDDEN = 0x40,
    SYSTEM = 0x20,
    VOLUME_ID = 0x10,
    DIRECTORY = 0x8,
    ARCHIVE = 0x4,
    LONG_NAME = READ_ONLY | HIDDEN | SYSTEM | VOLUME_ID,
    LONG_NAME_MASK =
        READ_ONLY | HIDDEN | SYSTEM | VOLUME_ID | DIRECTORY | ARCHIVE,
};

int init_fat();