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
#include <stddef.h>
#include <interfaces/message.h>

/******************************************************************************
 * DEFINE
 ******************************************************************************/


// FIXME make this constants into an enum
// TODO remove provisioning constants
#define THING_ID_SIZE               37
#define SHA256_SIZE                 32
#define URL_SIZE                   256
#define ID_SIZE                     16
#define MAX_LIB_VERSION_SIZE        10
#define UHWID_SIZE                  32
#define PROVISIONING_JWT_SIZE      268 // Max length of jwt is 268
#define BLE_MAC_ADDRESS_SIZE         6
#define WIFI_SSID_SIZE              33 // Max length of ssid is 32 + \0
#define WIFI_PWD_SIZE               64 // Max length of password is 63 + \0
#define LORA_APPEUI_SIZE            17 // appeui is 8 octets * 2 (hex format) + \0
#define LORA_APPKEY_SIZE            33 // appeui is 16 octets * 2 (hex format) + \0
#define LORA_CHANNEL_MASK_SIZE      13
#define LORA_DEVICE_CLASS_SIZE       2 // 1 char + \0
#define PIN_SIZE                     9 // 8 digits + \0
#define APN_SIZE                   101 // Max length of apn is 100 + \0
#define LOGIN_SIZE                  65 // Max length of login is 64 + \0
#define PASS_SIZE                   65 // Max length of password is 64 + \0
#define BAND_SIZE                    4
#define MAX_WIFI_NETWORKS           20
#define MAX_IP_SIZE                 16

/******************************************************************************
    TYPEDEF
 ******************************************************************************/

enum CommandId: MessageId {

  /* Device commands */
  DeviceBeginCmdId        = 1,
  ThingBeginCmdId,
  ThingUpdateCmdId,
  ThingDetachCmdId,
  DeviceRegisteredCmdId,
  DeviceAttachedCmdId,
  DeviceDetachedCmdId,

  /* Thing commands */
  LastValuesBeginCmdId,
  LastValuesUpdateCmdId,
  PropertiesUpdateCmdId,

  /* Generic commands */
  ResetCmdId,

  /* OTA commands */
  OtaBeginUpId,
  OtaProgressCmdUpId,
  OtaUpdateCmdDownId,

  /* Timezone commands */
  TimezoneCommandUpId,
  TimezoneCommandDownId,

  /* Unknown command id */
  UnknownCmdId,
};

typedef Message Command;

struct DeviceBeginCmd {
  Command c;
  struct {
    char lib_version[MAX_LIB_VERSION_SIZE];
  } params;
};

struct ThingBeginCmd {
  Command c;
  struct {
    char thing_id[THING_ID_SIZE];
  } params;
};

struct ThingUpdateCmd {
  Command c;
  struct {
    char thing_id[THING_ID_SIZE];
  } params;
};

struct ThingDetachCmd {
  Command c;
  struct {
    char thing_id[THING_ID_SIZE];
  } params;
};

struct LastValuesBeginCmd {
  Command c;
};

struct LastValuesUpdateCmd {
  Command c;
  struct {
    uint8_t * last_values;
    size_t length;
  } params;
};

struct OtaBeginUp {
  Command c;
  struct {
    uint8_t sha [SHA256_SIZE];
  } params;
};

struct OtaProgressCmdUp {
  Command c;
  struct {
    uint8_t  id[ID_SIZE];
    uint8_t  state;
    int32_t  state_data;
    uint64_t time;
  } params;
};

struct OtaUpdateCmdDown {
  Command c;
  struct {
    uint8_t id[ID_SIZE];
    char    url[URL_SIZE];
    uint8_t initialSha256[SHA256_SIZE];
    uint8_t finalSha256[SHA256_SIZE];
  } params;
};

struct TimezoneCommandUp {
    Command c;
};

struct TimezoneCommandDown {
  Command c;
  struct {
    int32_t offset;
    uint32_t until;
  } params;
};

union CommandDown {
  Command                         c;
  struct OtaUpdateCmdDown         otaUpdateCmdDown;
  struct ThingUpdateCmd           thingUpdateCmd;
  struct ThingDetachCmd           thingDetachCmd;
  struct LastValuesUpdateCmd      lastValuesUpdateCmd;
  struct TimezoneCommandDown      timezoneCommandDown;
};
