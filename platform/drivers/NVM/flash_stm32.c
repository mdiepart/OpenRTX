/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "flash_stm32.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include "interfaces/arch_registers.h"
#include "flash.h"
#include "interfaces/nvmem.h"


int STM32Flash_init(struct nvmSTM32FlashDevice *dev)
{
    const struct STM32FlashCfg *config = (const struct STM32FlashCfg*)dev->priv;

    #if defined(STM32F405xx) || defined(STM32F415xx) || \
        defined(STM32F407xx) || defined(STM32F417xx)

    // These devices have 4 pages of 16K followed by 1 page of 64k 
    // followed by 3 or 7 pages of 128K
    switch(config->page_size)
    {
        case PAGE_16K:
            if(config->address < 0x8000000)
                return -EINVAL; // Not a flash memory address

            if(config->address > 0x800C000)
                return -EINVAL; // Not the beginning of a 16K page

            if(config->address & 0x3FFF)
                return -EINVAL; // Not aligned on a 16K page

            const size_t upper_bound_16k = config->address + config->length;
            if(upper_bound_16k > 0x8010000)
                return -EINVAL; // Requested memory goes further than the 16K pages space
            break;
        case PAGE_64K:
            if(config->address != 0x8010000)
                return -EINVAL; // Not the beginning of the 64K page

            if(config->length != 65536)
                return -EINVAL; // Only one 64K page available
            break;
        case PAGE_128K:
            if(config->address < 0x8020000)
                return -EINVAL; // Address is below 128K pages area

            if(config->length & 0x1FFFF)
                return -EINVAL; // Length is not a multiple of page size

            if(config->address & 0x1FFFF)
                return -EINVAL; // Memory area is not aligned on a page
            
            uint16_t flash_size = *(uint16_t*)(0x1FFF7A22);
            const size_t upper_bound_128k = (config->address & 0x1FFFFF) + config->length;
            if( (upper_bound_128k>>10) > flash_size)
                return -ENOMEM; // Highest address is outside address space

            break;
        default:
            return -EINVAL; // Invalid page size for STM32F405
    }

    #elif defined(STM32H743xx)

    switch(config->page_size)
    {
        case PAGE_128K:
            if(config->address < 0x8000000)
                return -EINVAL; // Not a flash memory address
            if(config->length & 0x1FFFF)
                return -EINVAL; // Length is not a multiple of page size
            if(config->address & 0x1FFFF)
                return -EINVAL; // Memory area is not aligned on a page
            break;
        default:
            return -EINVAL; // Invalid page size for STM32H743
    }
    
    // Check if the area is within device flash area
    const uint16_t flash_size = *(uint16_t*)(0x1FF1E880); // Device flash size in kB
    const size_t upper_bound = (config->address & 0x1FFFFF) + config->length;

    if( (upper_bound>>10) > flash_size)
        return -ENOMEM; // Highest address is outside address space

    #else
    #error "Unsupported MCU type for flash_stm32 NVM device"
    #endif

    return 0;
}


int STM32Flash_terminate(struct nvmSTM32FlashDevice *dev)
{
    // Nothing to do here
    (void) dev;
    return 0;
}

static int nvm_api_read(const struct nvmDevice *dev, uint32_t offset,
                        void *data, size_t len)
{
    const struct STM32FlashCfg *config = (const struct STM32FlashCfg*)(dev->priv);
    if(offset+len > config->length)
    {
        return -EINVAL;
    }
    uint32_t* flash_addr_32 = (uint32_t*)(config->address + offset);

    size_t i = 0;
    for(; i < len/4; i++)
    {
        ((uint32_t *)data)[i] = flash_addr_32[i];
    }

    char* flash_addr_8 = (char*)(config->address + offset);

    for(i *= 4; i < len; i++)
    {
        ((char *)data)[i] = flash_addr_8[i];
    }

    return 0;
}

static int nvm_api_write(const struct nvmDevice *dev, uint32_t offset,
                         const void *data, size_t len)
{
    const struct STM32FlashCfg *config = (const struct STM32FlashCfg*)(dev->priv);
    if(offset+len > config->length)
    {
        return -EINVAL;
    }

    flash_write(config->address + offset, data, len);
    
    return 0;
}

static int nvm_api_erase(const struct nvmDevice *dev, uint32_t offset, size_t len)
{
    const struct STM32FlashCfg *config = (const struct STM32FlashCfg*)(dev->priv);
    
    if(offset+len > config->length)
        return -EINVAL; // Erasing memory outside the assigned memory

    if(len & (dev->erase_size-1))
        return -EINVAL; // len is not an integer number of sector
    
    if(offset & (dev->erase_size-1))
        return -EINVAL; // offset is not an integer number of sector

    size_t nb_sectors = len/(dev->erase_size);


#if defined(STM32F405xx) || defined(STM32F415xx) || \
    defined(STM32F407xx) || defined(STM32F417xx)    
    size_t sector_start = 0;

    switch (config->page_size)
    {
        case PAGE_16K:
            // Check the base address sector nb
            sector_start = (config->address - 0x8000000)/dev->erase_size
                         + offset/(dev->erase_size);
            break;
        case PAGE_64K:
            sector_start = (config->address - 0x8000000)/dev->erase_size
                         + offset/(dev->erase_size) + 3;

            break;
        case PAGE_128K:
            sector_start = (config->address - 0x8000000)/dev->erase_size
                         + offset/(dev->erase_size) + 4;
    }
#elif defined(STM32H743xx)
    sector_start = (config->address - 0x8000000)/dev->erase_size
                 + offset/(dev->erase_size);
#endif
    for(size_t i = 0; i < nb_sectors; i++)
    {
        if(!flash_eraseSector(sector_start + i))
            return -EACCESS; // Could not unlock flash
    }

    return 0;
}

const struct nvmOps stm32_flash_ops =
{
    .read = nvm_api_read,
    .write = nvm_api_write,
    .erase = nvm_api_erase,
    .sync = NULL,
};

const struct nvmInfo stm32_flash_infos[] = {
{
    .write_size = 1,
    .erase_size = 16384,
    .erase_cycles = 10000,
    .device_info = NVM_FLASH | NVM_WRITE | NVM_ERASE,
},
{
    .write_size = 1,
    .erase_size = 65536,
    .erase_cycles = 10000,
    .device_info = NVM_FLASH | NVM_WRITE | NVM_ERASE,
},
{
    .write_size = 1,
    .erase_size = 131072,
    .erase_cycles = 10000,
    .device_info = NVM_FLASH | NVM_WRITE | NVM_ERASE,
},
};
