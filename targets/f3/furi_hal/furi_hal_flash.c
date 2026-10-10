#include <furi_hal_flash.h>
#include <hardware/flash.h>
#include <hardware/regs/addressmap.h>

// Fixed layout on bare Pico 2, without the Flipper 16 MiB A/B partition table.
bool furi_hal_flash_get_partition_info(FlashPartitionId id, size_t* base, size_t* size) {
    if(id == FlashPartitionIdStorage) {
        if(base) *base = 0x3c0000;
        if(size) *size = 0x40000;
        return true;
    }
    if(id == FlashPartitionIdFwA) {
        if(base) *base = 0;
        if(size) *size = 0x3c0000;
        return true;
    }
    return false;
}
FlashPartitionId furi_hal_flash_get_active_fw_partition(void) { return FlashPartitionIdFwA; }
bool furi_hal_flash_rollback(void) { return false; }
size_t furi_hal_flash_get_page_size(void) { return FLASH_PAGE_SIZE; }
size_t furi_hal_flash_get_base(void) { return XIP_BASE; }
const void* furi_hal_flash_get_free_end_address(void) { return (const void*)(XIP_BASE + 0x3c0000); }
