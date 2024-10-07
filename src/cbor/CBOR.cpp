/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "CBOR.h"

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/

CommandId toCommandId(CBORCommandTag tag) {
  switch(tag) {
  case CBORCommandTag::CBOROtaBeginUp:
    return CommandId::OtaBeginUpId;
  case CBORCommandTag::CBORThingBeginCmd:
    return CommandId::ThingBeginCmdId;
  case CBORCommandTag::CBORLastValuesBeginCmd:
    return CommandId::LastValuesBeginCmdId;
  case CBORCommandTag::CBORDeviceBeginCmd:
    return CommandId::DeviceBeginCmdId;
  case CBORCommandTag::CBOROtaProgressCmdUp:
    return CommandId::OtaProgressCmdUpId;
  case CBORCommandTag::CBORTimezoneCommandUp:
    return CommandId::TimezoneCommandUpId;
  case CBORCommandTag::CBOROtaUpdateCmdDown:
    return CommandId::OtaUpdateCmdDownId;
  case CBORCommandTag::CBORThingUpdateCmd:
    return CommandId::ThingUpdateCmdId;
  case CBORCommandTag::CBORThingDetachCmd:
    return CommandId::ThingDetachCmdId;
  case CBORCommandTag::CBORLastValuesUpdate:
    return CommandId::LastValuesUpdateCmdId;
  case CBORCommandTag::CBORTimezoneCommandDown:
    return CommandId::TimezoneCommandDownId;
  case CBORCommandTag::CBORProvisioningStatus:
    return CommandId::ProvisioningStatus;
  case CBORCommandTag::CBORProvisioningListWifiNetworks:
    return CommandId::ProvisioningListWifiNetworks;
  case CBORCommandTag::CBORProvisioningUniqueId:
    return CommandId::ProvisioningUniqueId;
  case CBORCommandTag::CBORProvisioningSignature:
    return CommandId::ProvisioningSignature;
  case CBORCommandTag::CBORProvisioningTimestamp:
    return CommandId::ProvisioningTimestamp;
  case CBORCommandTag::CBORProvisioningCommands:
    return CommandId::ProvisioningCommands;
  case CBORCommandTag::CBORProvisioningWifiConfig:
    return CommandId::ProvisioningWifiConfig;
  case CBORCommandTag::CBORProvisioningLoRaConfig:
    return CommandId::ProvisioningLoRaConfig;
  case CBORCommandTag::CBORProvisioningGSMConfig:
    return CommandId::ProvisioningGSMConfig;
  case CBORCommandTag::CBORProvisioningNBIOTConfig:
    return CommandId::ProvisioningNBIOTConfig;
  case CBORCommandTag::CBORProvisioningCATM1Config:
    return CommandId::ProvisioningCATM1Config;
  case CBORCommandTag::CBORProvisioningEthernetConfig:
    return CommandId::ProvisioningEthernetConfig;
  case CBORCommandTag::CBORProvisioningCellularConfig:
    return CommandId::ProvisioningCellularConfig;
  default:
    return CommandId::UnknownCmdId;
  }
}

CBORCommandTag toCBORCommandTag(CommandId id) {
  switch(id) {
  case CommandId::OtaBeginUpId:
    return CBORCommandTag::CBOROtaBeginUp;
  case CommandId::ThingBeginCmdId:
    return CBORCommandTag::CBORThingBeginCmd;
  case CommandId::LastValuesBeginCmdId:
    return CBORCommandTag::CBORLastValuesBeginCmd;
  case CommandId::DeviceBeginCmdId:
    return CBORCommandTag::CBORDeviceBeginCmd;
  case CommandId::OtaProgressCmdUpId:
    return CBORCommandTag::CBOROtaProgressCmdUp;
  case CommandId::TimezoneCommandUpId:
    return CBORCommandTag::CBORTimezoneCommandUp;
  case CommandId::OtaUpdateCmdDownId:
    return CBORCommandTag::CBOROtaUpdateCmdDown;
  case CommandId::ThingUpdateCmdId:
    return CBORCommandTag::CBORThingUpdateCmd;
  case CommandId::ThingDetachCmdId:
    return CBORCommandTag::CBORThingDetachCmd;
  case CommandId::LastValuesUpdateCmdId:
    return CBORCommandTag::CBORLastValuesUpdate;
  case CommandId::TimezoneCommandDownId:
    return CBORCommandTag::CBORTimezoneCommandDown;
  case CommandId::ProvisioningStatus:
    return CBORCommandTag::CBORProvisioningStatus;
  case CommandId::ProvisioningListWifiNetworks:
    return CBORCommandTag::CBORProvisioningListWifiNetworks;
  case CommandId::ProvisioningUniqueId:
    return CBORCommandTag::CBORProvisioningUniqueId;
  case CommandId::ProvisioningSignature:
    return CBORCommandTag::CBORProvisioningSignature;
  case CommandId::ProvisioningTimestamp:
    return CBORCommandTag::CBORProvisioningTimestamp;
  case CommandId::ProvisioningCommands:
    return CBORCommandTag::CBORProvisioningCommands;
  case CommandId::ProvisioningWifiConfig:
    return CBORCommandTag::CBORProvisioningWifiConfig;
  case CommandId::ProvisioningLoRaConfig:
    return CBORCommandTag::CBORProvisioningLoRaConfig;
  case CommandId::ProvisioningGSMConfig:
    return CBORCommandTag::CBORProvisioningGSMConfig;
  case CommandId::ProvisioningNBIOTConfig:
    return CBORCommandTag::CBORProvisioningNBIOTConfig;
  case CommandId::ProvisioningCATM1Config:
    return CBORCommandTag::CBORProvisioningCATM1Config;
  case CommandId::ProvisioningEthernetConfig:
    return CBORCommandTag::CBORProvisioningEthernetConfig;
  case CommandId::ProvisioningCellularConfig:
    return CBORCommandTag::CBORProvisioningCellularConfig;
  default:
    return CBORCommandTag::CBORUnknownCmdTag;
  }
}
