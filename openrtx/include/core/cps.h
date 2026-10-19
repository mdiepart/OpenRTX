/*
 * SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CPS_H
#define CPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "core/datatypes.h"
#include "rtx/rtx.h"

// Magic number to identify the binary file
#define CPS_MAGIC 0x43585452
// Codeplug version v0.1
#define CPS_VERSION_MAJOR  0
#define CPS_VERSION_MINOR  2
#define CPS_VERSION_NUMBER ((CPS_VERSION_MAJOR << 8) | CPS_VERSION_MINOR)
#define CPS_STR_SIZE 32


/******************************************************************************
 *                         FM MODE                                            *
 ******************************************************************************/






/******************************************************************************
 *                         DMR MODE                                           *
 ******************************************************************************/

/**
 * Data structure containing all and only the information for DMR channels.
 */
typedef struct
{
    uint8_t  rxColorCode : 4,      //< Color code for RX squelch opening
             txColorCode : 4;      //< Color code sent during transmission

    uint8_t  dmr_timeslot;         //< DMR timeslot, either 1 or 2
    uint16_t contact_index;        //< Index to retrieve contact from list
}
__attribute__((packed)) dmrInfo_t; // 4B

/**
 * Enumeration type defining the types of a DMR contact.
 */
enum dmrContactType_t
{
    GROUP   = 0,    //< Group contact (Talkgroup)
    PRIVATE = 1,    //< Private contact
    ALL     = 2     //< Broadcast call
};

/**
 * Data structure describing a DMR contact entry.
 */
typedef struct
{
    uint32_t id;                //< DMR id

    uint8_t contactType : 2,    //< Call type
            rx_tone     : 1,    //< Call receive tone
            _unused     : 5;    //< Padding
}
__attribute__((packed)) dmrContact_t; // 5B



/******************************************************************************
 *                         M17 MODE                                           *
 ******************************************************************************/

/**
 * M17 channel modes.
 */
enum m17mode_t
{
    DIGITAL_VOICE      = 1,     //< Digital Voice
    DIGITAL_DATA       = 2,     //< Digital Data
    DIGITAL_VOICE_DATA = 3      //< Digital Voice and Data
};

/**
 * M17 channel encryption.
 */
enum m17crypto_t
{
    PLAIN     = 0,              //< No encryption, plaintext data is sent
    SCRAMBLER = 1,              //< Scrambler
    AES       = 2               //< AES
};

/**
 * M17 gps operation.
 */
enum m17gps_t
{
    NO_GPS   = 0,               //< No GPS information is sent
    GPS_META = 1                //< GPS position is sent along with payload
};

/**
 * Data structure containing all and only the information for M17 channels.
 */
typedef struct
{
    uint8_t  mode  : 4,         ///< Channel operation mode
             encr  : 4;         ///< Encryption mode
    uint8_t gps_mode;           ///< Channel GPS mode
    uint16_t contact_index;     ///< Index to retrieve data from contact list
}
__attribute__((packed)) m17Info_t; // 4B

/**
 * Data structure describing M17-specific contact fields.
 */
typedef struct
{
    uint8_t address[6];         //< M17 encoded address
}
__attribute__((packed)) m17Contact_t; // 6B



/******************************************************************************
 *                         COMMON DATA STRUCTURES                             *
 ******************************************************************************/

/**
 * Data structure for geolocation data
 */
typedef struct
{
    int8_t   ch_lat_int;    //< Latitude integer part
    uint16_t ch_lat_dec;    //< Latitude decimal part
    int16_t  ch_lon_int;    //< Longitude integer part
    uint16_t ch_lon_dec;    //< Longitude decimal part
    uint16_t ch_altitude;   //< Meters MSL. Stored +500
}
__attribute__((packed)) geo_t; // 9B

/**
 * Data structure containing all the information of a channel.
 */
typedef struct
{
    uint8_t mode;                  //< Operating mode

    uint8_t bandwidth      : 2,    //< Bandwidth
            rx_only        : 1,    //< 1 means RX-only channel
            m17_can        : 4,    ///< M17 Channel Access Number (should move to m17Info_t)
            _unused        : 1;     //< Padding to 8 bits

    uint8_t  sqlLevel;             //< Squelch level
    uint32_t power;                //< Tx power, in mW

    freq_t  rx_frequency;          //< RX Frequency, in Hz
    freq_t  tx_frequency;          //< TX Frequency, in Hz

    uint8_t scanList_index;        //< Scan List: None, ScanList1...250
    uint8_t groupList_index;       //< Group List: None, GroupList1...128

    char    name[CPS_STR_SIZE];    //< Channel name
    char    descr[CPS_STR_SIZE];   //< Description of the channel
    geo_t   ch_location;           //< Transmitter geolocation

    // These 2 values alogn with m17_can above should be in m17Info_t below
    // but because these can be changed in the settings at any time,
    // they can not be dependant on the selected opMode.
    char    m17_dest[10];           ///< M17 destination
    char    M17_meta_text[53];     ///< M17 Meta Text to send

    union
    {
        fmInfo_t  fm;              //< Information block for FM channels
        dmrInfo_t dmr;             //< Information block for DMR channels
        m17Info_t m17;             //< Information block for M17 channels
    };
}
__attribute__((packed)) channel_t; // 158B

/**
 * Data structure describing a codeplug contact.
 */
typedef struct
{
    char    name[CPS_STR_SIZE]; //< Display name of the contact
    uint8_t mode;               //< Operating mode

    union
    {
        dmrContact_t  dmr;      //< DMR specific contact info
        m17Contact_t  m17;      //< M17 specific contact info
    }
    info; // 6B
}
__attribute__((packed)) contact_t; // 39B

/**
 * Data structure describing a bank header.
 * A bank is a variable size structure composed of a bank header and a
 * variable length array of uint32_t each representing a channel index.
 */
typedef struct
{
    char     name[CPS_STR_SIZE];
    uint16_t ch_count;             //< Count of all the channels in this bank
}
__attribute__((packed)) bankHdr_t; // 18B + 2 * ch_count

/**
 * The codeplug binary structure is composed by:
 * - A header struct
 * - A variable length array of all the contacts
 * - A variable length array of all the channels
 * - A variable length array of the offsets to reach each bank
 * - A binary dense structure of all the banks
 */
typedef struct
{
    uint64_t magic;                //< Magic number "RTXC"
    uint16_t version_number;       //< Version number for the cps structure
    char     author[CPS_STR_SIZE]; //< Author of the codeplug
    char     descr[CPS_STR_SIZE];  //< Description of the codeplug
    uint64_t timestamp;            //< unix timestamp of the codeplug

    uint16_t ct_count;             //< Number of stored contacts
    uint16_t ch_count;             //< Number of stored channels
    uint16_t b_count;              //< Number of stored banks
}
__attribute__((packed)) cps_header_t; // 88B

/**
 * Create and return a viable channel for this radio.
 * Suitable for default VFO settings or the creation of a new channel.
 * Needs to be generated by a function frequency settings require details from
 * the running hardware on limitations.
 */
channel_t cps_getDefaultChannel();

#ifdef __cplusplus
}
#endif

#endif // CPS_H
