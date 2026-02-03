/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "interfaces/nvmem.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "flash_stm32.h"

static int nvm_api_read(const struct nvmDevice *dev, uint32_t offset,
                        void *data, size_t len)
{
    (void) dev;
    (void) offset;
    (void) data;
    (void) len;
    // TODO

    return -ENOSYS; // Not implemented yet
}

static int nvm_api_write(const struct nvmDevice *dev, uint32_t offset,
                         const void *data, size_t len)
{
    (void) dev;
    (void) offset;
    (void) data;
    (void) len;
    // TODO

    return -ENOSYS; // Not implemented yet
}

static int nvm_api_erase(const struct nvmDevice *dev, uint32_t offset, size_t len)
{
    (void) dev;
    (void) offset;
    (void) len;
    // TODO

    return -ENOSYS; // Not implemented yet
}

const struct nvmOps stm32_flash_ops = 
{
    .read = nvm_api_read,
    .write = nvm_api_write,
    .erase = nvm_api_erase,
    .sync = NULL,
};

