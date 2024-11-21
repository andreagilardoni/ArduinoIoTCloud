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
#define UHWID_SIZE                  32
#define PROVISIONING_JWT_SIZE      246
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
  ProvisioningUniqueHardwareId,
  ProvisioningJWT,
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
    WiFiNetwork discoveredWifiNetworks[MAX_WIFI_NETWORKS];
    uint8_t numDiscoveredWiFiNetworks = 0;
  } params;
};

struct ProvisioningUniqueHardwareIdMessage {
  Command c;
  struct {
    char uniqueHardwareId[UHWID_SIZE]; //The payload is an array of char with a maximum length of 32, not null terminated. It's not a string.
  } params;
};

struct ProvisioningJWTMessage {
  Command c;
  struct {
    char jwt[PROVISIONING_JWT_SIZE]; //The payload is an array of char with a maximum length of 246, not null terminated. It's not a string.
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
    char ssid[WIFI_SSID_SIZE]; 
    char pwd[WIFI_PWD_SIZE];  
  } params;
};

struct ProvisioningLoRaConfigMessage {
  Command c;
  struct {
    char       appeui[LORA_APPEUI_SIZE];    
    char       appkey[LORA_APPKEY_SIZE];    
    uint8_t    band;
    char       channelMask[LORA_CHANNEL_MASK_SIZE];
    char       deviceClass[LORA_DEVICE_CLASS_SIZE];
  } params;
};

struct ProvisioningCATM1ConfigMessage {
  Command c;
  struct {
    char      pin[PIN_SIZE];
    char      apn[APN_SIZE]; 
    char      login[LOGIN_SIZE];
    char      pass[PASS_SIZE];
    uint32_t  band[BAND_SIZE];
  } params;
};

struct ProvisioningIPStruct{
  enum IPType {
    IPV4,
    IPV6
  };
  IPType type;
  uint8_t ip[MAX_IP_SIZE];
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
    char pin[PIN_SIZE];
    char apn[APN_SIZE];
    char login[LOGIN_SIZE];
    char pass[PASS_SIZE];
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
