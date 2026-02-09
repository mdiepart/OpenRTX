/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FLASH_STM32_H
#define FLASH_STM32_H

#include <unistd.h>
#include <stdint.h>
#include "interfaces/nvmem.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Flash page size enum
 */
typedef enum {
    PAGE_16K, /// 16kB page size
    PAGE_64K, /// 64kB page size
    PAGE_128K /// 128kB page size
} STM32_Page_Size_e;

// Currently, only STM32F405 and STM32H743 are used

struct STM32FlashCfg
{
    const uint32_t          address;
    const size_t            length;
    const STM32_Page_Size_e page_size;
};

/**
 * Device driver for STM32 internal-flash based storage.
 */
struct nvmSTM32FlashDevice
{
    const void           *priv;
    const struct nvmOps  *ops;
    const struct nvmInfo *info;
};

/**
 *Driver API functions and info.
 */
extern const struct nvmOps stm32_flash_ops;
extern const struct nvmInfo stm32_flash_infos[];

/**
 * Instantiates an STM32 Flash nonvolatile memory device.
 *
 * @param name: device name.
 * @param config: an instance of STM32FlashCfg containing proper config
 */
#define STM32_FLASH_DEVICE_DEFINE(name, config)     \
static struct nvmSTM32FlashDevice name =            \
{                                                   \
    .priv   = (void *)&config,                      \
    .ops    = &stm32_flash_ops,                     \
    .info   = &(stm32_flash_info[config.page_size]),\
};                                                  

/**
 * Initialize an STM32Flash NVM device. The device must have been defined 
 * using STM32_FLASH_DEVICE_DEFINE beforehand.
 *
 * @param dev: pointer to a STM32 nonvolatile memory device
 * 
 * @return 0 on success, negative error code otherwise
 */
int STM32Flash_init(struct nvmSTM32FlashDevice *dev);

/**
 * Terminates an STM32Flash NVM device.
 *
 * @param dev: pointer to a STM32 nonvolatile memory device
 * 
 * @return 0 on success, negative error code otherwise
 */
int STM32Flash_terminate(struct nvmSTM32FlashDevice *dev);

#ifdef __cplusplus
}
#endif

#endif
