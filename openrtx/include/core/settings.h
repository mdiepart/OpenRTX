/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include "hwconfig.h"
#include <stdbool.h>

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

/**
 * The settings struct has a few rules:
 * - Once a location in settings is used, we cannot take it away
 * - Use 1-bit bitfield values for booleans, but this then requires that the settings default is always false
 */
typedef struct
{
    uint8_t header[2];            // Reserved header bytes (used by settings_manager)
    uint8_t brightness;           // Display brightness
    uint8_t contrast;             // Display contrast
    uint8_t sqlLevel;             // Squelch level
    uint8_t voxLevel;             // Vox level
    int8_t  utc_timezone;         // Timezone, in units of half hours
    bool    gps_enabled      : 1, // GPS active
            m17_can_rx       : 1, // Check M17 CAN on RX
            showBatteryIcon  : 1, // Battery display true: icon, false: percentage
            gpsSetTime       : 1, // Use GPS to ajust RTC time
            vpPhoneticSpell  : 1, // Phonetic spell enabled
            macroMenuUnlatch : 1, // Automatic latch of macro menu
            reserved1        : 1, // Reserved for future use
            reserved2        : 1; // Reserved for future use
    uint8_t display_timer    : 4, // Standby timer
            m17_can          : 4; // M17 CAN
    uint8_t vpLevel          : 3, // Voice prompt level
            reserved3        : 5; // Reserved for future use
    char    callsign[10];         // Plaintext callsign
    char    m17_dest[10];         // M17 destination
    char    M17_meta_text[53];    // M17 Meta Text to send
}
__attribute__((packed)) settings_t;

static const settings_t default_settings =
{
    { 0 , 0 },                      // Reserved header bytes
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
    false,                        // Check M17 CAN on RX
    false,                        // Display battery icon
    false,                        // Update RTC with GPS
    false,                        // Phonetic spell off
    false,                        // Automatic unlatch of macro menu enabled
    false,                        // Reserved 1
    false,                        // Reserved 2
    TIMER_30S,                    // 30 seconds
    0,                            // M17 CAN
    0,                            // Voice prompts off
    0,                            // Reserved 2
    "OPNRTX",                     // Empty callsign
    "",                           // Empty M17 destination
    "OpenRTX",                    // Default M17 meta text
};

#endif /* SETTINGS_H */
