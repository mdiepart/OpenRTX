/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include "hwconfig.h"
#include <stdbool.h>

/** \file
 * The settings.h file is the header defining the structure for the device
 * settings structure and the functions to load/save the settings to
 * non-volatile memory.
 * The settings are saved across two partitions using an A/B partitions scheme.
 * The settings are thus saved first in partition A then partition B and so on.
 * Each new settings structure is appended to the last one in the partition.
 * When the partition is full, it is erased. Because of the A/B partition
 * scheme, this ensures that there is always a copy of the settings existing
 * on the device. If an error occurs during the save, or if the latest copy
 * gets corruptes, there is at lease one previous copy stored in the other
 * partition.
 */

typedef enum
{
    TIMER_OFF =  0,
    TIMER_5S  =  1,
    TIMER_10S =  2,
    TIMER_15S =  3,
    TIMER_20S =  4,
    TIMER_25S =  5,
    TIMER_30S =  6,
    TIMER_1M  =  7,
    TIMER_2M  =  8,
    TIMER_3M  =  9,
    TIMER_4M  = 10,
    TIMER_5M  = 11,
    TIMER_15M = 12,
    TIMER_30M = 13,
    TIMER_45M = 14,
    TIMER_1H  = 15
}
display_timer_t;

typedef struct
{
    uint8_t brightness;           ///< Display brightness
    uint8_t contrast;             ///< Display contrast
    uint8_t sqlLevel;             ///< Squelch level
    uint8_t voxLevel;             ///< Vox level
    int8_t  utc_timezone;         ///< Timezone, in units of half hours
    bool    gps_enabled;          ///< GPS active
    char    callsign[10];         ///< Plaintext callsign
    uint8_t display_timer   : 4,  ///< Standby timer
            m17_can         : 4;  ///< M17 CAN
    uint8_t vpLevel         : 3,  ///< Voice prompt level
            vpPhoneticSpell : 1,  ///< Phonetic spell enabled
            macroMenuLatch  : 1,  ///< Automatic latch of macro menu
            _reserved       : 3;
    bool    m17_can_rx;           ///< Check M17 CAN on RX
    char    m17_dest[10];         ///< M17 destination
    bool    showBatteryIcon;      ///< Battery display true: icon, false: percentage
    bool    gpsSetTime;           ///< Use GPS to ajust RTC time
}
__attribute__((packed)) settings_t;


static const settings_t default_settings =
{
    100,                          // Brightness
#ifdef CONFIG_SCREEN_CONTRAST
    CONFIG_DEFAULT_CONTRAST,      // Contrast
#else
    255,                          // Contrast
#endif
    4,                            // Squelch level, 4 = S3
    0,                            // Vox level
    0,                            // UTC Timezone
    false,                        // GPS enabled
    "",                           // Empty callsign
    TIMER_30S,                    // 30 seconds
    0,                            // M17 CAN
    0,                            // Voice prompts off
    0,                            // Phonetic spell off
    1,                            // Automatic latch of macro menu enabled
    0,                            // not used
    false,                        // Check M17 CAN on RX
    "",                           // Empty M17 destination
    false,                        // Display battery icon
    false,                        // Update RTC with GPS
};

struct settings_storage_s;
typedef struct settings_storage_s settings_storage_t;

/**
 * Initialize a settings_storage_t structure to save and load device settings.
 *
 * @param s pointer to a pre-allocated settings_storage_t structure to be initialized
 * @param nvm_dev NVM device number in which to store the device settings
 * @param part_A NVM device partition number for storage partition A
 * @param part_B NVM device partition number for storage partition B
 * @return 0 if successful, negative error code otherwise
 */
int settings_storage_init(settings_storage_t *s, const int nvm_dev,
                          const int part_A, const int part_B);

/**
 * Load device settings from non-volatile memory.
 *
 * @param s pointer to an initialized settings_storage_t structure
 * @param settings pointer to a settings_t structure where to write the loaded settings
 * @return 0 if successful, negative error code otherwise
 */
int settings_storage_load(settings_storage_t *s, settings_t *settings);

/**
 * Save device settings to non-volatile memory. Will not perform any actual write
 * if the settings haven't changed.
 *
 * @param s pointer to an initialized settings_storage_t structure
 * @param settings pointer to a settings_t structure containing the settings to save
 * @return 0 if successful, negative error code otherwise
 */
int settings_storage_save(settings_storage_t *s, const settings_t *settings);



#endif /* SETTINGS_H */
