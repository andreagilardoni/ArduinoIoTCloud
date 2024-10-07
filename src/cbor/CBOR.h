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
#include <message/Commands.h>

/******************************************************************************
   TYPEDEF
 ******************************************************************************/

enum CBORCommandTag: uint64_t {
  // Commands UP
  CBOROtaBeginUp                     = 0x010000,
  CBORThingBeginCmd                  = 0x010300,
  CBORLastValuesBeginCmd             = 0x010500,
  CBORDeviceBeginCmd                 = 0x010700,
  CBOROtaProgressCmdUp               = 0x010200,
  CBORTimezoneCommandUp              = 0x010800,

  // Commands DOWN
  CBOROtaUpdateCmdDown               = 0x010100,
  CBORThingUpdateCmd                 = 0x010400,
  CBORThingDetachCmd                 = 0x011000,
  CBORLastValuesUpdate               = 0x010600,
  CBORTimezoneCommandDown            = 0x010900,

  // Provisioning Commands
  CBORProvisioningStatus             = 0x012000,
  CBORProvisioningListWifiNetworks   = 0x012001,
  CBORProvisioningUniqueId           = 0x012010,
  CBORProvisioningSignature          = 0x012011,
  CBORProvisioningTimestamp          = 0x012002,
  CBORProvisioningCommands           = 0x012003,
  CBORProvisioningWifiConfig         = 0x012004,
  CBORProvisioningLoRaConfig         = 0x012005,
  CBORProvisioningGSMConfig          = 0x012006,
  CBORProvisioningNBIOTConfig        = 0x012007,
  CBORProvisioningCATM1Config        = 0x012008,
  CBORProvisioningEthernetConfig     = 0x012009,
  CBORProvisioningCellularConfig     = 0x012012,


  // Unknown Command Tag https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml
  CBORUnknownCmdTag16b    = 0xffff,              // invalid tag
  CBORUnknownCmdTag32b    = 0xffffffff,          // invalid tag
  CBORUnknownCmdTag64b    = 0xffffffffffffffff,  // invalid tag
  CBORUnknownCmdTag       = CBORUnknownCmdTag32b
};

/******************************************************************************
 * FUNCTION DECLARATION
 ******************************************************************************/

CommandId toCommandId(CBORCommandTag tag);
CBORCommandTag toCBORCommandTag(CommandId id);
