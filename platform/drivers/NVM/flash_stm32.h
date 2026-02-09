/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FLASH_STM32_H
#define FLASH_STM32_H

#include "interfaces/nvmem.h"

/**
 * Device driver for STM32 internal-flash based storage.
 */
struct nvmSTM32FlashDevice
{
    const void           *priv;
    const struct nvmOps  *ops;
    const struct nvmInfo *info;
    int                  device;
}

/**
 *Driver API functions and info.
 */
extern const struct nvmOps stm32_flash_ops;
extern const struct nvmInfo stm32_flash_info;

#define STM32_FLASH_DEVICE_DEFINE(name)     \
static struct nvmSTM32FlashDevice name =    \
{                                           \
    .ops    = &stm32_flash_ops,             \
    .info   = &stm32_flash_info,            \
    .device = 0                             \
};                                          \

/**
 *
 */
int STM32Flash_init(struct nvmSTM32FlashDevice *dev);

int STM32Flash_terminate(struct nvmSTM32FlashDevice *dev);
#endif
