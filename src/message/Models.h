/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <stdint.h>

#if __cplusplus >= 201402L // Cpp14
#include <algorithm>
#endif // __cplusplus

/******************************************************************************
 * DEFINE
 ******************************************************************************/

#define THING_ID_SIZE               37
#define SHA256_SIZE                 32
#define URL_SIZE                   256
#define ID_SIZE                     16
#define MAX_LIB_VERSION_SIZE        10

/******************************************************************************
   TYPEDEF
 ******************************************************************************/

enum CommandID: uint32_t {
    // Commands UP
    OtaBeginUpId               = 0x10000,
    ThingGetIdCmdUpId          = 0x10300,
    ThingGetLastValueCmdUpId   = 0x10500,
    DeviceBeginCmdUpId         = 0x10700,
    OtaProgressCmdUpId         = 0x10200,
    TimezoneCommandUpId        = 0x10800,

    // Commands DOWN
    OtaUpdateCmdDownId         = 0x10100,
    ThingGetIdCmdDownId        = 0x10400,
    ThingGetLastValueCmdDownId = 0x10600,
    TimezoneCommandDownId      = 0x10900,

    // Internal messages
    RequestlastValues          = 100,
    SendProperties             = 200,
    Disconnect                 = 300,
};

struct Command {
    CommandID id;
};

typedef Command Message;

struct ThingGetIdCmdUp {
    Command c;
    struct {
        char thing_id[THING_ID_SIZE];
    } params;
};

struct ThingGetLastValueCmdUp {
    Command c;
};

struct OtaBeginUp {
    Command c;
    struct {
        uint8_t sha [SHA256_SIZE];
    } params;
};

struct DeviceBeginCmdUp {
    Command c;
    struct {
        char lib_version[MAX_LIB_VERSION_SIZE];
    } params;
};

struct OtaProgressCmdUp {
    Command c;
    struct {
        uint8_t  id[ID_SIZE];
        uint8_t  state;
        int32_t  state_data;
        uint32_t time;
        uint32_t count;
    } params;
};

struct TimezoneCommandUp {
    Command c;
};

struct __attribute__((__packed__)) OtaUpdateCmdDown {
    Command c;
    struct {
        uint8_t id[ID_SIZE];
        char    url[URL_SIZE];
        uint8_t initialSha256[SHA256_SIZE];
        uint8_t finalSha256[SHA256_SIZE];
    } params;
};

struct __attribute__((__packed__)) ThingGetIdCmdDown {
    Command c;
    struct {
        char thing_id[THING_ID_SIZE];
    } params;
};

struct __attribute__((__packed__)) ThingGetLastValueCmdDown {
    Command c;
    struct {
        uint8_t * last_values;
        size_t length;
    } params;
};

struct __attribute__((__packed__)) TimezoneCommandDown{
    Command c;
    struct {
        uint32_t offset;
        uint32_t until;
    } params;
};

union CommandDown {
    struct Command                  c;
    struct OtaUpdateCmdDown         otaUpdateCmdDown;
    struct ThingGetIdCmdDown        thingGetIdCmdDown;
    struct ThingGetLastValueCmdDown thingGetLastValueCmdDown;
    struct TimezoneCommandDown      timezoneCommandDown;
    uint8_t buf[
#if __cplusplus >= 201402L // Cpp14
        std::max({sizeof(otaUpdateCmdDown),
            sizeof(thingGetIdCmdDown),
            sizeof(thingGetLastValueCmdDown),
            sizeof(timezoneCommandDown)})
#else
        /* for versions of cpp prior to cpp14 std::max is not defined as constexpr,
         * hence it cannot be used here thus we need to specify the size of the
         * biggest member manually.
         */
        sizeof(otaUpdateCmdDown)
#endif // __cplusplus
        ];
};
