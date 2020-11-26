/***************************************************************************
 *   Copyright (C) 2020 by Federico Amedeo Izzo IU2NUO,                    *
 *                         Niccolò Izzo IU2KIN                             *
 *                         Frederik Saraci IU2NRO                          *
 *                         Silvano Seva IU2KWO                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#ifndef NVMEM_H
#define NVMEM_H

#include <stdint.h>
#include <cps.h>

/**
 * Interface for nonvolatile memory management, usually an external SPI flash
 * memory, containing calibration, contact data and so on.
 */

/**
 * Initialise NVM driver.
 */
void nvm_init();

/**
 * Terminate NVM driver.
 */
void nvm_terminate();

/**
 * Load calibration data from nonvolatile memory.
 *
 * @param buf: destination buffer for calibration data.
 */
void nvm_readCalibData(void *buf);

/**
 * Read one channel entry from table stored in nonvolatile memory.
 *
 * @param channel: pointer to the channel_t data structure to be populated.
 * @param pos: position, inside the channel table, from which read data.
 * @return 0 on success, -1 on failure
 */
int nvm_readChannelData(channel_t *channel, uint16_t pos);

#endif /* NVMEM_H */