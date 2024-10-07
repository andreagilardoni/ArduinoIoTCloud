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

enum CommandId: uint32_t {

  /* Device commands */
  DeviceBeginCmdId,
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

  /* Provisioning commands*/
  ProvisioningStatus,
  ProvisioningListWifiNetworks,
  ProvisioningUniqueId,
  ProvisioningSignature,
  ProvisioningTimestamp,
  ProvisioningCommands,
  ProvisioningWifiConfig,
  ProvisioningLoRaConfig,
  ProvisioningGSMConfig,
  ProvisioningNBIOTConfig,
  ProvisioningCATM1Config,
  ProvisioningEthernetConfig,
  ProvisioningCellularConfig,
};

struct Command {
  CommandId id;
};

typedef Command Message;

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

struct ProvisioningStatusMessage {
  Command c;
  struct {
    int16_t status;
  } params;
};

struct WiFiNetwork {
  char *SSID;
  int *RSSI;
};

struct ProvisioningListWifiNetworksMessage {
  Command c;
  struct {
    WiFiNetwork discoveredWifiNetworks[20];
    uint8_t numDiscoveredWiFiNetworks = 0;
  } params;
};

struct ProvisioningUniqueIdMessage {
  Command c;
  struct {
    char uniqueId[32]; //The payload is an array of char with a maximum length of 32, not null terminated. It's not a string.
  } params;
};

struct ProvisioningSignatureMessage {
  Command c;
  struct {
    char signature[32]; //The payload is an array of char with a maximum length of 32, not null terminated. It's not a string.
  } params;
};

struct ProvisioningTimestampMessage {
  Command c;
  struct {
    uint64_t timestamp;
  } params;
};

struct ProvisioningCommandsMessage {
  Command c;
  struct {
    uint8_t cmd;
  } params;
};

struct ProvisioningWifiConfigMessage {
  Command c;
  struct {
    char ssid[33]; // Max length of ssid is 32 + \0
    char pwd[64];  // Max length of password is 63 + \0
  } params;
};

struct ProvisioningLoRaConfigMessage {
  Command c;
  struct {
    char       appeui[17];    // appeui is 8 octets * 2 (hex format) + \0
    char       appkey[33];    // appeui is 16 octets * 2 (hex format) + \0
    uint8_t    band;
    char       channelMask[13];
    char       deviceClass[2];
  } params;
};

struct ProvisioningCATM1ConfigMessage {
  Command c;
  struct {
    char      pin[9];
    char      apn[101]; // Max length of apn is 100 + \0
    char      login[65];
    char      pass[65];
    uint32_t  band[4];
  } params;
};

struct ProvisioningIPStruct{
  enum IPType {
    IPV4,
    IPV6
  };
  IPType type;
  uint8_t ip[16];
};

struct ProvisioningEthernetConfigMessage {
  Command c;
  struct {
    ProvisioningIPStruct       ip;
    ProvisioningIPStruct       dns;
    ProvisioningIPStruct       gateway;
    ProvisioningIPStruct       netmask;
    unsigned long              timeout;
    unsigned long              response_timeout;
  } params;
};


struct ProvisioningCellularConfigMessage {
  Command c;
  struct {
    char pin[9];
    char apn[101]; // Max length of apn is 100 + \0
    char login[65];
    char pass[65];
  } params;
};

union CommandDown {
  struct Command                  c;
  struct OtaUpdateCmdDown         otaUpdateCmdDown;
  struct ThingUpdateCmd           thingUpdateCmd;
  struct ThingDetachCmd           thingDetachCmd;
  struct LastValuesUpdateCmd      lastValuesUpdateCmd;
  struct TimezoneCommandDown      timezoneCommandDown;
};

union ProvisioningCommandDown {
  struct Command                           c;
  struct ProvisioningTimestampMessage      provisioningTimestamp;
  struct ProvisioningCommandsMessage       provisioningCommands;
  struct ProvisioningWifiConfigMessage     provisioningWifiConfig;
  struct ProvisioningLoRaConfigMessage     provisioningLoRaConfig;
  struct ProvisioningCATM1ConfigMessage    provisioningCATM1Config;
  struct ProvisioningEthernetConfigMessage provisioningEthernetConfig;
  struct ProvisioningCellularConfigMessage provisioningCellularConfig;
};
