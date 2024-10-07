/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
    INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#undef max
#undef min
#include <algorithm>

#include "MessageDecoder.h"
#include <AIoTC_Config.h>

/******************************************************************************
    PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

Decoder::Status CBORMessageDecoder::decode(Message * message, uint8_t const * const payload, size_t& length)
{
  CborValue main_iter, array_iter;
  CborTag tag;
  CborParser parser;

  if (cbor_parser_init(payload, length, 0, &parser, &main_iter) != CborNoError)
    return Decoder::Status::Error;

  if (main_iter.type != CborTagType)
    return Decoder::Status::Error;

  if (cbor_value_get_tag(&main_iter, &tag) == CborNoError) {
    message->id = toCommandId(CBORCommandTag(tag));
  }

  if (cbor_value_advance(&main_iter) != CborNoError) {
    return Decoder::Status::Error;
  }

  ArrayParserState current_state  = ArrayParserState::EnterArray,
                   next_state     = ArrayParserState::Error;

  while (current_state != ArrayParserState::Complete) {
    switch (current_state) {
      case ArrayParserState::EnterArray           : next_state = handle_EnterArray(&main_iter, &array_iter); break;
      case ArrayParserState::ParseParam           : next_state = handle_Param(&array_iter, message); break;
      case ArrayParserState::LeaveArray           : next_state = handle_LeaveArray(&main_iter, &array_iter); break;
      case ArrayParserState::Complete             : return Decoder::Status::Complete;
      case ArrayParserState::MessageNotSupported  : return Decoder::Status::Error;
      case ArrayParserState::Error                : return Decoder::Status::Error;
    }

    current_state = next_state;
  }

  return Decoder::Status::Complete;
}

/******************************************************************************
    PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

bool copyCBORStringToArray(CborValue * param, char * dest, size_t dest_size) {
  if (cbor_value_is_text_string(param)) {
    // NOTE: keep in mind that _cbor_value_copy_string tries to put a \0 at the end of the string
    if(_cbor_value_copy_string(param, dest, &dest_size, NULL) == CborNoError) {
      return true;
    }
  }

  return false;
}

// FIXME dest_size should be also returned, the copied byte array can have a different size from the starting one
// for the time being we need this on SHA256 only
bool copyCBORByteToArray(CborValue * param, uint8_t * dest, size_t dest_size) {
  if (cbor_value_is_byte_string(param)) {
    // NOTE: keep in mind that _cbor_value_copy_string tries to put a \0 at the end of the string
    if(_cbor_value_copy_string(param, dest, &dest_size, NULL) == CborNoError) {
      return true;
    }
  }

  return false;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::handle_EnterArray(CborValue * main_iter, CborValue * array_iter) {
  ArrayParserState next_state = ArrayParserState::Error;
  if (cbor_value_get_type(main_iter) == CborArrayType) {
    if (cbor_value_enter_container(main_iter, array_iter) == CborNoError) {
      next_state = ArrayParserState::ParseParam;
    }
  }

  return next_state;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::handle_LeaveArray(CborValue * main_iter, CborValue * array_iter) {
  // Advance to the next parameter (the last one in the array)
  if (cbor_value_advance(array_iter) == CborNoError) {
    // Leave the array
    if (cbor_value_leave_container(main_iter, array_iter) == CborNoError) {
      return ArrayParserState::Complete;
    }
  }

  return ArrayParserState::Error;
}

/******************************************************************************
    MESSAGE DECODE FUNCTIONS
 ******************************************************************************/

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeThingUpdateCmd(CborValue * param, Message * message) {
  ThingUpdateCmd * thingCommand = (ThingUpdateCmd *) message;

  // Message is composed of a single parameter, a string (thing_id)
  if (!copyCBORStringToArray(param, thingCommand->params.thing_id, sizeof(thingCommand->params.thing_id))) {
    return ArrayParserState::Error;
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeThingDetachCmd(CborValue * param, Message * message) {
  ThingDetachCmd * thingCommand = (ThingDetachCmd *) message;

  // Message is composed of a single parameter, a string (thing_id)
  if (!copyCBORStringToArray(param, thingCommand->params.thing_id, sizeof(thingCommand->params.thing_id))) {
    return ArrayParserState::Error;
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeTimezoneCommandDown(CborValue * param, Message * message) {
  TimezoneCommandDown * setTz = (TimezoneCommandDown *) message;

  // Message is composed of 2 parameters, offset 32-bit signed integer and until 32-bit unsigned integer
  // Get offset
  if (cbor_value_is_integer(param)) {
    int64_t val = 0;
    if (cbor_value_get_int64(param, &val) == CborNoError) {
      setTz->params.offset = static_cast<int32_t>(val);
    }
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  // Get until
  if (cbor_value_is_integer(param)) {
    uint64_t val = 0;
    if (cbor_value_get_uint64(param, &val) == CborNoError) {
      setTz->params.until = static_cast<uint32_t>(val);
    }
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeLastValuesUpdateCmd(CborValue * param, Message * message) {
  LastValuesUpdateCmd * setLv = (LastValuesUpdateCmd *) message;

  // Message is composed by a single parameter, a variable length byte array.
  if (cbor_value_is_byte_string(param)) {
    // Cortex M0 is not able to assign a value to pointed memory that is not 32bit aligned
    // we use a support variable to cope with that
    size_t s;
    if (cbor_value_dup_byte_string(param, &setLv->params.last_values, &s, NULL) != CborNoError) {
      return ArrayParserState::Error;
    }

    setLv->params.length = s;
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeOtaUpdateCmdDown(CborValue * param, Message * message) {
  CborError error = CborNoError;
  OtaUpdateCmdDown * ota = (OtaUpdateCmdDown *) message;

  // Message is composed 4 parameters: id, url, initialSha, finalSha
  if (!copyCBORByteToArray(param, ota->params.id, sizeof(ota->params.id))) {
    return ArrayParserState::Error;
  }

  error = cbor_value_advance(param);

  if ((error != CborNoError) || !copyCBORStringToArray(param, ota->params.url, sizeof(ota->params.url))) {
    return ArrayParserState::Error;
  }

  error = cbor_value_advance(param);

  if ((error != CborNoError) || !copyCBORByteToArray(param, ota->params.initialSha256, sizeof(ota->params.initialSha256))) {
    return ArrayParserState::Error;
  }

  error = cbor_value_advance(param);

  if ((error != CborNoError) || !copyCBORByteToArray(param, ota->params.finalSha256, sizeof(ota->params.finalSha256))) {
    return ArrayParserState::Error;
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeProvisioningTimestampMessage(CborValue * param, Message * message) {
  ProvisioningTimestampMessage * provisioningTimestamp = (ProvisioningTimestampMessage *) message;

  // Message is composed of a single parameter: a 64-bit unsigned integer
  if (cbor_value_is_integer(param)) {
    uint64_t val = 0;
    if (cbor_value_get_uint64(param, &val) == CborNoError) {
      provisioningTimestamp->params.timestamp = val;
    }
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeProvisioningWifiConfigMessage(CborValue * param, Message * message) {
  ProvisioningWifiConfigMessage * provisioningWifiConfig = (ProvisioningWifiConfigMessage *) message;

  // Message is composed of 2 parameters: ssid and password
  if (!copyCBORStringToArray(param, provisioningWifiConfig->params.ssid, sizeof(provisioningWifiConfig->params.ssid))) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORStringToArray(param, provisioningWifiConfig->params.pwd, sizeof(provisioningWifiConfig->params.pwd))) {
    return ArrayParserState::Error;
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeProvisioningCommandsMessage(CborValue * param, Message * message) {
  ProvisioningCommandsMessage * provisioningCommands = (ProvisioningCommandsMessage *) message;

  // Message is composed of a single parameter: a 32-bit signed integer
  if (cbor_value_is_integer(param)) {
    int val = 0;
    if (cbor_value_get_int(param, &val) == CborNoError) {
      provisioningCommands->params.cmd = val;
    }
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeProvisioningLoRaConfigMessage(CborValue * param, Message * message) {
  ProvisioningLoRaConfigMessage * provisioningLoRaConfig = (ProvisioningLoRaConfigMessage *) message;
  // Message is composed of 5 parameters: app_eui, app_key, band, channel_mask, device_class
  if (!copyCBORStringToArray(param, provisioningLoRaConfig->params.appeui, sizeof(provisioningLoRaConfig->params.appeui))) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORStringToArray(param, provisioningLoRaConfig->params.appkey, sizeof(provisioningLoRaConfig->params.appkey))) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (cbor_value_is_integer(param)) {
    int val = 0;
    if (cbor_value_get_int(param, &val) == CborNoError) {
      provisioningLoRaConfig->params.band = val;
    }
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if(!copyCBORStringToArray(param, provisioningLoRaConfig->params.channelMask, sizeof(provisioningLoRaConfig->params.channelMask))) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORStringToArray(param, provisioningLoRaConfig->params.deviceClass, sizeof(provisioningLoRaConfig->params.deviceClass))) {
    return ArrayParserState::Error;
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeProvisioningCATM1ConfigMessage(CborValue * param, Message * message) {
  ProvisioningCATM1ConfigMessage * provisioningCATM1Config = (ProvisioningCATM1ConfigMessage *) message;
  CborValue array_iter;
  size_t arrayLength = 0;

  memset(provisioningCATM1Config->params.band, 0x00, sizeof(provisioningCATM1Config->params.band));

  // Message is composed of 5 parameters: pin, band, apn, login and password 
  if (!copyCBORStringToArray(param, provisioningCATM1Config->params.pin, sizeof(provisioningCATM1Config->params.pin))) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (cbor_value_get_type(param) != CborArrayType) {
    return ArrayParserState::Error;
  }
  
  cbor_value_get_array_length(param, &arrayLength);
  size_t maxArraySize = sizeof(provisioningCATM1Config->params.band) / sizeof(provisioningCATM1Config->params.band[0]);

  if(arrayLength > maxArraySize) {
    return ArrayParserState::Error;
  }

  if (cbor_value_enter_container(param, &array_iter) != CborNoError) {
    return ArrayParserState::Error;
  }

  for(size_t i = 0; i < arrayLength; i++) {
    if (!cbor_value_is_unsigned_integer(&array_iter)) {
      return ArrayParserState::Error;
    }

    int val = 0;
    if (cbor_value_get_int(&array_iter, &val) != CborNoError) {
      return ArrayParserState::Error;
    }

    provisioningCATM1Config->params.band[i] = val;
    
    if (cbor_value_advance(&array_iter) != CborNoError) {
      return ArrayParserState::Error;
    }      
  }

  if (cbor_value_leave_container(param, &array_iter) != CborNoError){
    return ArrayParserState::Error;
  }

  if (!copyCBORStringToArray(param, provisioningCATM1Config->params.apn, sizeof(provisioningCATM1Config->params.apn))) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORStringToArray(param, provisioningCATM1Config->params.login, sizeof(provisioningCATM1Config->params.login))) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORStringToArray(param, provisioningCATM1Config->params.pass, sizeof(provisioningCATM1Config->params.pass))) {
    return ArrayParserState::Error;
  }  

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeProvisioningEthernetConfigMessage(CborValue * param, Message * message){
  ProvisioningEthernetConfigMessage * provisioningEthernetConfig = (ProvisioningEthernetConfigMessage *) message;

  // Message is composed of 2 parameters: static ip, dns, default gateway, netmask, timeout and response timeout
  if (!getProvisioningIPStructFromMessage(param, &provisioningEthernetConfig->params.ip)) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!getProvisioningIPStructFromMessage(param, &provisioningEthernetConfig->params.dns)) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!getProvisioningIPStructFromMessage(param, &provisioningEthernetConfig->params.gateway)) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!getProvisioningIPStructFromMessage(param, &provisioningEthernetConfig->params.netmask)) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (cbor_value_is_integer(param)) {
    int val = 0;
    if (cbor_value_get_int(param, &val) == CborNoError) {
      provisioningEthernetConfig->params.timeout = val;
    }
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (cbor_value_is_integer(param)) {
    int val = 0;
    if (cbor_value_get_int(param, &val) == CborNoError) {
      provisioningEthernetConfig->params.response_timeout = val;
    }
  }

  return ArrayParserState::LeaveArray;
}

bool CBORMessageDecoder::getProvisioningIPStructFromMessage(CborValue *param, ProvisioningIPStruct *ipStruct)
{
  size_t len = sizeof(ipStruct->ip);
  
  memset(ipStruct->ip, 0, len);

  if (!cbor_value_is_byte_string(param)) {
    return false;
  }

  if(_cbor_value_copy_string(param, ipStruct->ip, &len, NULL) != CborNoError) {
      return false;
  }

  if(len == 4){
    ipStruct->type = ProvisioningIPStruct::IPType::IPV4;
  } else if(len == 16){
    ipStruct->type = ProvisioningIPStruct::IPType::IPV6;
  } else {
    return false;
  }

  return true;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeProvisioningCellularConfigMessage(CborValue * param, Message * message){
  ProvisioningCellularConfigMessage * provisioningCellularConfig = (ProvisioningCellularConfigMessage *) message;

  // Message is composed of 4 parameters: pin, apn, login and password
  if (!copyCBORStringToArray(param, provisioningCellularConfig->params.pin, sizeof(provisioningCellularConfig->params.pin))) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORStringToArray(param, provisioningCellularConfig->params.apn, sizeof(provisioningCellularConfig->params.apn))) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORStringToArray(param, provisioningCellularConfig->params.login, sizeof(provisioningCellularConfig->params.login))) {
    return ArrayParserState::Error;
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORStringToArray(param, provisioningCellularConfig->params.pass, sizeof(provisioningCellularConfig->params.pass))) {
    return ArrayParserState::Error;
  }

  return ArrayParserState::LeaveArray;
}



CBORMessageDecoder::ArrayParserState CBORMessageDecoder::handle_Param(CborValue * param, Message * message) {

  switch (message->id)
  {
  case CommandId::ThingUpdateCmdId:
    return CBORMessageDecoder::decodeThingUpdateCmd(param, message);

  case CommandId::ThingDetachCmdId:
    return CBORMessageDecoder::decodeThingDetachCmd(param, message);

  case CommandId::TimezoneCommandDownId:
    return CBORMessageDecoder::decodeTimezoneCommandDown(param, message);

  case CommandId::LastValuesUpdateCmdId:
    return CBORMessageDecoder::decodeLastValuesUpdateCmd(param, message);

  case CommandId::OtaUpdateCmdDownId:
    return CBORMessageDecoder::decodeOtaUpdateCmdDown(param, message);

  case CommandId::ProvisioningTimestamp:
    return CBORMessageDecoder::decodeProvisioningTimestampMessage(param, message);
  
  case CommandId::ProvisioningWifiConfig:
    return CBORMessageDecoder::decodeProvisioningWifiConfigMessage(param, message);
  
  case CommandId::ProvisioningCommands:
    return CBORMessageDecoder::decodeProvisioningCommandsMessage(param, message);

  case CommandId::ProvisioningLoRaConfig:
    return CBORMessageDecoder::decodeProvisioningLoRaConfigMessage(param, message);
  
  case CommandId::ProvisioningGSMConfig:
    return CBORMessageDecoder::decodeProvisioningCellularConfigMessage(param, message);

  case CommandId::ProvisioningNBIOTConfig:
    return CBORMessageDecoder::decodeProvisioningCellularConfigMessage(param, message);
  
  case CommandId::ProvisioningCATM1Config:
    return CBORMessageDecoder::decodeProvisioningCATM1ConfigMessage(param, message);
  
  case CommandId::ProvisioningEthernetConfig:
    return CBORMessageDecoder::decodeProvisioningEthernetConfigMessage(param, message);
  
  case CommandId::ProvisioningCellularConfig:
    return CBORMessageDecoder::decodeProvisioningCellularConfigMessage(param, message);

  default:
    return ArrayParserState::MessageNotSupported;
  }

  return ArrayParserState::LeaveArray;
}
