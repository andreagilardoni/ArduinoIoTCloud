//
// This file is part of CBORDecoder
//
// Copyright 2019 ARDUINO SA (http://www.arduino.cc/)
//
// This software is released under the GNU General Public License version 3,
// which covers the main part of CBORDecoder.
// The terms of this license can be found at:
// https://www.gnu.org/licenses/gpl-3.0.en.html
//
// You can be released from the requirements of the above licenses by purchasing
// a commercial license. Buying such a license is mandatory if you want to modify or
// otherwise use the software for commercial activities involving the Arduino
// software without disclosing the source code of your own applications. To purchase
// a commercial license, send an email to license@arduino.cc.
//

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

MessageDecoder::DecoderState MessageDecoder::decode(Message * message, uint8_t const * const payload, size_t const length)
{
  CborValue main_iter, array_iter;
  CborTag tag;
  CborParser parser;

  if (cbor_parser_init(payload, length, 0, &parser, &main_iter) != CborNoError)
    return DecoderState::MalformedMessage;

  if (main_iter.type != CborTagType)
    return DecoderState::MalformedMessage;

  if (cbor_value_get_tag(&main_iter, &tag) == CborNoError) {
    message->id = CommandID(tag);
  }

  if (cbor_value_advance(&main_iter) != CborNoError) {
    return DecoderState::MalformedMessage;
  }

  ArrayParserState current_state  = ArrayParserState::EnterArray,
                   next_state     = ArrayParserState::Error;

  while (current_state != ArrayParserState::Complete) {
    switch (current_state) {
      case ArrayParserState::EnterArray           : next_state = handle_EnterArray(&main_iter, &array_iter); break;
      case ArrayParserState::ParseParam           : next_state = handle_Param(&array_iter, message); break;
      case ArrayParserState::LeaveArray           : next_state = handle_LeaveArray(&main_iter, &array_iter); break;
      case ArrayParserState::Complete             : return DecoderState::Success; break;
      case ArrayParserState::MessageNotSupported  : return DecoderState::MessageNotSupported; break;
      case ArrayParserState::Error                : return DecoderState::Error; break;
    }

    current_state = next_state;
  }

  return DecoderState::Success;
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

bool copyCBORStringToArray(CborValue * param, char * dest, size_t dest_size) {
    if (cbor_value_is_text_string(param)) {
      char * val      = 0;
      size_t val_size = 0;
      if (cbor_value_dup_text_string(param, &val, &val_size, NULL) == CborNoError) {
        if (val_size > dest_size) {
          val_size = dest_size;
        }
        strncpy(dest, val, dest_size);
        free(val);
        return true;
      }
    }
    return false;
}

bool copyCBORByteToArray(CborValue * param, uint8_t * dest, size_t dest_size) {
    if (cbor_value_is_byte_string(param)) {
      uint8_t * val      = 0;
      size_t    val_size = 0;
      if (cbor_value_dup_byte_string(param, &val, &val_size, NULL) == CborNoError) {
        if (val_size > dest_size) {
          val_size = dest_size;
        }
        std::copy(val, val + val_size, dest);
        free(val);
        return true;
      }
    }
    return false;
}

MessageDecoder::ArrayParserState MessageDecoder::handle_EnterArray(CborValue * main_iter, CborValue * array_iter) {
  ArrayParserState next_state = ArrayParserState::Error;
  if (cbor_value_get_type(main_iter) == CborArrayType) {
    if (cbor_value_enter_container(main_iter, array_iter) == CborNoError) {
      next_state = ArrayParserState::ParseParam;
    }
  }

  return next_state;
}

MessageDecoder::ArrayParserState MessageDecoder::handle_LeaveArray(CborValue * main_iter, CborValue * array_iter) {
  // Advance to the next parameter (the last one in the array) 
  if (cbor_value_advance(array_iter) != CborNoError) {
    return ArrayParserState::Error;
  }
  // Leave the array
  if (cbor_value_leave_container(main_iter, array_iter) != CborNoError) {
    return ArrayParserState::Error;
  }
  return ArrayParserState::Complete;
}

/******************************************************************************
   MESSAGE DECODE FUNCTIONS
 ******************************************************************************/

MessageDecoder::ArrayParserState MessageDecoder::decodeThingGetIdCmdDown(CborValue * param, Message * message) {
  ThingGetIdCmdDown * thingCommand = (ThingGetIdCmdDown *) message;

  // Message is composed of a single parameter, a string (thing_id)
  if (!copyCBORStringToArray(param, thingCommand->fields.params.thing_id, sizeof(thingCommand->fields.params.thing_id))) {
    return ArrayParserState::Error;
  }

  return ArrayParserState::LeaveArray;
}

MessageDecoder::ArrayParserState MessageDecoder::decodeTimezoneCommandDown(CborValue * param, Message * message) {
  TimezoneCommandDown * setTz = (TimezoneCommandDown *) message;

  // Message is composed of 2 parameters, offset and until 32-bit unsigned integer
  // Get offset
  if (cbor_value_is_integer(param)) {
    uint64_t val = 0;
    if (cbor_value_get_uint64(param, &val) == CborNoError) {
      setTz->fields.params.offset = static_cast<uint32_t>(val);
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
      setTz->fields.params.until = static_cast<uint32_t>(val);
    }
  }

  return ArrayParserState::LeaveArray;
}

MessageDecoder::ArrayParserState MessageDecoder::decodeThingGetLastValueCmdDown(CborValue * param, Message * message) {
  ThingGetLastValueCmdDown * setLv = (ThingGetLastValueCmdDown *) message;

  // Message is composed by a single parameter, a variable length byte array.
  if (cbor_value_is_byte_string(param)) {
    if (cbor_value_dup_byte_string(param, &setLv->fields.params.last_values, &setLv->fields.params.length, NULL) != CborNoError) {
      return ArrayParserState::Error;
    }
  }

  return ArrayParserState::LeaveArray;
}

MessageDecoder::ArrayParserState MessageDecoder::decodeOtaUpdateCmdDown(CborValue * param, Message * message) {
  OtaUpdateCmdDown * ota = (OtaUpdateCmdDown *) message;

  // Message is composed 4 parameters: id, url, initialSha, finalSha
  if (!copyCBORStringToArray(param, ota->fields.params.id, sizeof(ota->fields.params.id))) {
    return ArrayParserState::Error;
  }

  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORStringToArray(param, ota->fields.params.url, sizeof(ota->fields.params.url))) {
    return ArrayParserState::Error;
  }

  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORByteToArray(param, ota->fields.params.initialSha256, sizeof(ota->fields.params.initialSha256))) {
    return ArrayParserState::Error;
  }

  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORByteToArray(param, ota->fields.params.finalSha256, sizeof(ota->fields.params.finalSha256))) {
    return ArrayParserState::Error;
  }

  return ArrayParserState::LeaveArray;
}

MessageDecoder::ArrayParserState MessageDecoder::handle_Param(CborValue * param, Message * message) {

  switch (message->id)
  {
  case CommandID::ThingGetIdCmdDownId:
    return MessageDecoder::decodeThingGetIdCmdDown(param, message);

  case CommandID::TimezoneCommandDownId:
    return MessageDecoder::decodeTimezoneCommandDown(param, message);

  case CommandID::ThingGetLastValueCmdDownId:
    return MessageDecoder::decodeThingGetLastValueCmdDown(param, message);

  case CommandID::OtaUpdateCmdDownId:
    return MessageDecoder::decodeOtaUpdateCmdDown(param, message);

  default:
    return ArrayParserState::MessageNotSupported;
  }

  return ArrayParserState::LeaveArray;
}
