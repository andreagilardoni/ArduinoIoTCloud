/*
   This file is part of ArduinoIoTCloud.

   Copyright 2020 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

#ifndef ARDUINO_CBOR_MESSAGE_ENCODER_H_
#define ARDUINO_CBOR_MESSAGE_ENCODER_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#undef max
#undef min
#include <list>

#include "../models/models.h"
#include "lib/tinycbor/cbor-lib.h"

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class MessageEncoder
{

public:
    static CborError encode(Message * message, uint8_t * data, size_t const size, int & bytes_encoded);

private:

  MessageEncoder() { }
  MessageEncoder(CborEncoder const &) { }

  enum class EncoderState
  {
    EncodeTag,
    EncodeArray,
    EncodeParam,
    CloseArray,
    MessageNotSupported,
    Complete,
    Error
  };

  static EncoderState handle_EncodeTag(CborEncoder * encoder, Message * message);
  static EncoderState handle_EncodeArray(CborEncoder * encoder, CborEncoder * array_encoder, Message * message);
  static EncoderState handle_EncodeParam(CborEncoder * array_encoder, Message * message);
  static EncoderState handle_CloseArray(CborEncoder * encoder, CborEncoder * array_encoder);

  // Message specific encoders
  static CborError encodeThingGetIdCmdUp(CborEncoder * array_encoder, Message * message);
  static CborError encodeOtaBeginUp(CborEncoder * array_encoder, Message * message);
  static CborError encodeDeviceBeginCmdUp(CborEncoder * array_encoder, Message * message);
  static CborError encodeOtaProgressCmdUp(CborEncoder * array_encoder, Message * message);
};

#endif /* ARDUINO_CBOR_MESSAGE_ENCODER_H_ */
