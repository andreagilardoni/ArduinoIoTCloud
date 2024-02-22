/*
   Copyright (c) 2019 Arduino.  All rights reserved.
*/

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <memory>

#include <util/CBORTestUtil.h>
#include <MessageEncoder.h>

/**************************************************************************************
   TEST CODE
 **************************************************************************************/

SCENARIO("Test the encoding of command messages") {
    /************************************************************************************/

  WHEN("Encode the OtaBeginUp message")
  {
    OtaBeginUp * command = new OtaBeginUp();
    uint8_t sha[SHA256_SIZE] = {0x01, 0x02, 0x03, 0x04};
    memcpy(command->fields.params.sha, sha, SHA256_SIZE);

    Message * msg = (Message*) command;
    msg->id = CommandID::OtaBeginUpId;

    uint8_t buffer[256];
    int bytes_encoded = 0;

    CborError err = MessageEncoder::encode(msg, buffer, sizeof(buffer), bytes_encoded);

    // Test the encoding is 
    // DA 00010000                             # tag(65536)
    //    81                                   # array(1)
    //       58 20                             # bytes(32)
    //          01020304
    THEN("The encoding is successful") {
      REQUIRE(err == CborNoError);
      REQUIRE(bytes_encoded == 40);
      REQUIRE(buffer[0] == (uint8_t)0xDA);
      REQUIRE(buffer[1] == (uint8_t)0x00);
      REQUIRE(buffer[2] == (uint8_t)0x01);
      REQUIRE(buffer[3] == (uint8_t)0x00);
      REQUIRE(buffer[4] == (uint8_t)0x00);
      REQUIRE(buffer[5] == (uint8_t)0x81);
      REQUIRE(buffer[6] == (uint8_t)0x58);
      REQUIRE(buffer[7] == (uint8_t)0x20);
      REQUIRE(buffer[8] == (uint8_t)0x01);
      REQUIRE(buffer[9] == (uint8_t)0x02);
      REQUIRE(buffer[10] == (uint8_t)0x03);
      REQUIRE(buffer[11] == (uint8_t)0x04);
    }
    delete command;
  }


  /************************************************************************************/

  WHEN("Encode the ThingGetIdCmdUp message")
  {
    ThingGetIdCmdUp * command = new ThingGetIdCmdUp();
    String thing_id = "thing_id";
    strcpy(command->fields.params.thing_id, thing_id.c_str());

    Message * message = (Message *) command;
    message->id = CommandID::ThingGetIdCmdUpId;

    uint8_t buffer[256];
    int bytes_encoded = 0;

    CborError err = MessageEncoder::encode(message, buffer, sizeof(buffer), bytes_encoded);

    // Test the encoding is 
    // DA 00010300               # tag(66304)
    //    81                     # array(1)
    //       68                  # text(8)
    //          7468696E675F6964 # "thing_id"

    THEN("The encoding is successful") {
      REQUIRE(err == CborNoError);
      REQUIRE(bytes_encoded == 15);
      REQUIRE(buffer[0] == (uint8_t)0xDA);
      REQUIRE(buffer[1] == (uint8_t)0x00);
      REQUIRE(buffer[2] == (uint8_t)0x01);
      REQUIRE(buffer[3] == (uint8_t)0x03);
      REQUIRE(buffer[4] == (uint8_t)0x00);
      REQUIRE(buffer[5] == (uint8_t)0x81);
      REQUIRE(buffer[6] == (uint8_t)0x68);
      REQUIRE(buffer[7] == (uint8_t)0x74);
      REQUIRE(buffer[8] == (uint8_t)0x68);
      REQUIRE(buffer[9] == (uint8_t)0x69);
      REQUIRE(buffer[10] == (uint8_t)0x6E);
      REQUIRE(buffer[11] == (uint8_t)0x67);
      REQUIRE(buffer[12] == (uint8_t)0x5F);
      REQUIRE(buffer[13] == (uint8_t)0x69);
      REQUIRE(buffer[14] == (uint8_t)0x64);
    }
    delete message;
  }

  /************************************************************************************/

  WHEN("Encode the ThingGetLastValueCmdUpId message")
  {
    Message * message = new Message();
    message->id = CommandID::ThingGetLastValueCmdUpId;

    uint8_t buffer[256];
    int bytes_encoded = 0;

    CborError err = MessageEncoder::encode(message, buffer, sizeof(buffer), bytes_encoded);

    // Test the encoding is 
    // DA 00010500 # tag(66816)
    //    80       # array(0)
    THEN("The encoding is successful") {
      REQUIRE(err == CborNoError);
      REQUIRE(bytes_encoded == 6);
      REQUIRE(buffer[0] == (uint8_t)0xDA);
      REQUIRE(buffer[1] == (uint8_t)0x00);
      REQUIRE(buffer[2] == (uint8_t)0x01);
      REQUIRE(buffer[3] == (uint8_t)0x05);
      REQUIRE(buffer[4] == (uint8_t)0x00);
      REQUIRE(buffer[5] == (uint8_t)0x80);
    }
    delete message;
  }

    /************************************************************************************/

  WHEN("Encode the DeviceBeginCmdUpId message")
  {
    DeviceBeginCmdUp * command = new DeviceBeginCmdUp();
    String lib_version = "2.0.0";
    strcpy(command->fields.params.lib_version, lib_version.c_str());

    Message * message = (Message *) command;
    message->id = CommandID::DeviceBeginCmdUpId;

    uint8_t buffer[256];
    int bytes_encoded = 0;

    CborError err = MessageEncoder::encode(message, buffer, sizeof(buffer), bytes_encoded);

    // Test the encoding is 
    // DA 00010700         # tag(67328)
    //    81               # array(1)
    //       65            # text(5)
    //          322E302E30 # "2.0.0"
    THEN("The encoding is successful") {
      REQUIRE(err == CborNoError);
      REQUIRE(bytes_encoded == 12);
      REQUIRE(buffer[0] == (uint8_t)0xDA);
      REQUIRE(buffer[1] == (uint8_t)0x00);
      REQUIRE(buffer[2] == (uint8_t)0x01);
      REQUIRE(buffer[3] == (uint8_t)0x07);
      REQUIRE(buffer[4] == (uint8_t)0x00);
      REQUIRE(buffer[5] == (uint8_t)0x81);
      REQUIRE(buffer[6] == (uint8_t)0x65);
      REQUIRE(buffer[7] == (uint8_t)0x32);
      REQUIRE(buffer[8] == (uint8_t)0x2E);
      REQUIRE(buffer[9] == (uint8_t)0x30);
      REQUIRE(buffer[10] == (uint8_t)0x2E);
      REQUIRE(buffer[11] == (uint8_t)0x30);
    }
    delete message;
  }

  /************************************************************************************/

  WHEN("Encode the OtaProgressCmdUp message")
  {
    OtaProgressCmdUp * command = new OtaProgressCmdUp();
    command->fields.params.count = 1;
    command->fields.params.time = 2;
    String s = "id";
    strcpy(command->fields.params.id, s.c_str());
    s = "state";
    strcpy(command->fields.params.state, s.c_str());

    Message * message = (Message *) command;
    message->id = CommandID::OtaProgressCmdUpId;

    uint8_t buffer[256];
    int bytes_encoded = 0;

    CborError err = MessageEncoder::encode(message, buffer, sizeof(buffer), bytes_encoded);

    // Test the encoding is 
    // DA 00010200         # tag(66048)
    //    84               # array(4)
    //       62            # text(2)
    //          6964       # "id"
    //       65            # text(5)
    //          7374617465 # "state"
    //       02            # unsigned(2)
    //       01            # unsigned(1)
    THEN("The encoding is successful") {
      REQUIRE(err == CborNoError);
      REQUIRE(bytes_encoded == 17);
      REQUIRE(buffer[0] == (uint8_t)0xDA);
      REQUIRE(buffer[1] == (uint8_t)0x00);
      REQUIRE(buffer[2] == (uint8_t)0x01);
      REQUIRE(buffer[3] == (uint8_t)0x02);
      REQUIRE(buffer[4] == (uint8_t)0x00);
      REQUIRE(buffer[5] == (uint8_t)0x84);
      REQUIRE(buffer[6] == (uint8_t)0x62);
      REQUIRE(buffer[7] == (uint8_t)0x69);
      REQUIRE(buffer[8] == (uint8_t)0x64);
      REQUIRE(buffer[9] == (uint8_t)0x65);
      REQUIRE(buffer[10] == (uint8_t)0x73);
      REQUIRE(buffer[11] == (uint8_t)0x74);
      REQUIRE(buffer[12] == (uint8_t)0x61);
      REQUIRE(buffer[13] == (uint8_t)0x74);
      REQUIRE(buffer[14] == (uint8_t)0x65);
      REQUIRE(buffer[15] == (uint8_t)0x02);
      REQUIRE(buffer[16] == (uint8_t)0x01);
    }

    delete message;
  }

  /************************************************************************************/

  WHEN("Encode the TimezoneCommandUpId message")
  {
    Message * message = new Message();
    message->id = CommandID::TimezoneCommandUpId;

    uint8_t buffer[256];
    int bytes_encoded = 0;

    CborError err = MessageEncoder::encode(message, buffer, sizeof(buffer), bytes_encoded);

    // Test the encoding is 
    // DA 00010800 # tag(67584)
    //    80       # array(0)
    THEN("The encoding is successful") {
      REQUIRE(err == CborNoError);
      REQUIRE(bytes_encoded == 6);
      REQUIRE(buffer[0] == (uint8_t)0xDA);
      REQUIRE(buffer[1] == (uint8_t)0x00);
      REQUIRE(buffer[2] == (uint8_t)0x01);
      REQUIRE(buffer[3] == (uint8_t)0x08);
      REQUIRE(buffer[4] == (uint8_t)0x00);
      REQUIRE(buffer[5] == (uint8_t)0x80);
    }
    delete message;
  }

}
