/*
   Copyright (c) 2024 Arduino.  All rights reserved.
*/

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <catch.hpp>
#include <string.h>

#include <memory>

#include <util/CBORTestUtil.h>
#include <MessageDecoder.h>

/******************************************************************************
   TEST CODE
 ******************************************************************************/

SCENARIO("Test the decoding of command messages") {
  /****************************************************************************/

  WHEN("Decode the ThingUpdateCmdId message")
  {
    CommandDown command;
    /*
      DA 00010400                             # tag(66560)
          81                                  # array(1)
              78 24                           # text(36)
                65343439346435352D383732612D346664322D393634362D393266383739343933393463 # "e4494d55-872a-4fd2-9646-92f87949394c"
    */
    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x04, 0x00, 0x81, 0x78, 0x24,
                               0x65, 0x34, 0x34, 0x39, 0x34, 0x64, 0x35, 0x35,
                               0x2D, 0x38, 0x37, 0x32, 0x61, 0x2D, 0x34, 0x66,
                               0x64, 0x32, 0x2D, 0x39, 0x36, 0x34, 0x36, 0x2D,
                               0x39, 0x32, 0x66, 0x38, 0x37, 0x39, 0x34, 0x39,
                               0x33, 0x39, 0x34, 0x63};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);
    const char *thingIdToMatch = "e4494d55-872a-4fd2-9646-92f87949394c";

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(strcmp(command.thingUpdateCmd.params.thing_id, thingIdToMatch) == 0);
      REQUIRE(command.c.id == ThingUpdateCmdId);
    }
  }

  /****************************************************************************/
  WHEN("Decode the ThingDetachCmd message")
  {
    CommandDown command;
    /*
      DA 00011000                             # tag(69632)
        81                                    # array(1)
            78 24                             # text(36)
              65343439346435352D383732612D346664322D393634362D393266383739343933393463 # "e4494d55-872a-4fd2-9646-92f87949394c"
    */
    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x10, 0x00, 0x81, 0x78, 0x24,
                               0x65, 0x34, 0x34, 0x39, 0x34, 0x64, 0x35, 0x35,
                               0x2D, 0x38, 0x37, 0x32, 0x61, 0x2D, 0x34, 0x66,
                               0x64, 0x32, 0x2D, 0x39, 0x36, 0x34, 0x36, 0x2D,
                               0x39, 0x32, 0x66, 0x38, 0x37, 0x39, 0x34, 0x39,
                               0x33, 0x39, 0x34, 0x63};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);
    const char *thingIdToMatch = "e4494d55-872a-4fd2-9646-92f87949394c";

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(strcmp(command.thingDetachCmd.params.thing_id, thingIdToMatch) == 0);
      REQUIRE(command.c.id == ThingDetachCmdId);
    }
  }

   /************************************************************************************/

  WHEN("Decode the ThingUpdateCmdId message containing a number instead of a string")
  {
    CommandDown command;
    /*
      DA 00010400                             # tag(66560)
          81                                  # array(1)
              1A 65DCB821                     # unsigned(1708963873)
    */
    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x04, 0x00, 0x81, 0x1A, 0x65,
                               0xDC, 0xB8, 0x21};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/
  WHEN("Decode the ThingDetachCmd message containing a number instead of a string")
  {
    CommandDown command;
    /*
      DA 00011000                             # tag(69632)
        81                                    # array(1)
              1A 65DCB821                     # unsigned(1708963873)
    */
    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x10, 0x00, 0x81, 0x1A, 0x65,
                               0xDC, 0xB8, 0x21};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/

  WHEN("Decode the TimezoneCommandDown message")
  {
    CommandDown command;

    /*
      DA 00010764       # tag(67840)
        82              # array(2)
            1A 65DCB821 # unsigned(1708963873)
            1A 78ACA191 # unsigned(2024579473)
    */

    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x09, 0x00, 0x82, 0x1A, 0x65,
                               0xDC, 0xB8, 0x21, 0x1A, 0x78, 0xAC, 0xA1, 0x91};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.timezoneCommandDown.params.offset == (uint32_t)1708963873);
      REQUIRE(command.timezoneCommandDown.params.until == (uint32_t)2024579473);
      REQUIRE(command.c.id == TimezoneCommandDownId);
    }
  }

  /****************************************************************************/

  WHEN("Decode the LastValuesUpdateCmd message")
  {
    CommandDown command;

    /*
      DA 00010600                        # tag(67072)
        81                               # array(1)
            4D                           # bytes(13)
              00010203040506070809101112 # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\u0010\u0011\u0012"

    */

    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x06, 0x00, 0x81, 0x4D, 0x00,
                               0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                               0x09, 0x10, 0x11, 0x12};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.lastValuesUpdateCmd.params.length == 13);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[0] == (uint8_t)0x00);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[1] == (uint8_t)0x01);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[2] == (uint8_t)0x02);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[3] == (uint8_t)0x03);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[4] == (uint8_t)0x04);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[5] == (uint8_t)0x05);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[6] == (uint8_t)0x06);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[7] == (uint8_t)0x07);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[8] == (uint8_t)0x08);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[9] == (uint8_t)0x09);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[10] == (uint8_t)0x10);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[11] == (uint8_t)0x11);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[12] == (uint8_t)0x12);
      REQUIRE(command.c.id == LastValuesUpdateCmdId);
    }
    free(command.lastValuesUpdateCmd.params.last_values);
  }

  /****************************************************************************/

  WHEN("Decode the OtaUpdateCmdDown message")
  {
    CommandDown command;

    /*
      DA 00010100                             # tag(65792)
        84                                    # array(4)
            50                                # bytes(16)
              C73CB045F9C2434585AFFA36A307BFE7"\xC7<\xB0E\xF9\xC2CE\x85\xAF\xFA6\xA3\a\xBF\xE7"
            78 72                             # text(141)
              68747470733A2F2F626F617264732D69
              6E742E6F6E69756472612E63632F7374
              6F726167652F6669726D776172652F76
              312F6466316561633963376264363334
              37336666666231313766393837333730
              33653465633935353933316532363766
              32363236326230393439626331366463
              3439                            # "https://boards-int.oniudra.cc/storage/firmware/v1/df1eac9c7bd63473fffb117f9873703e4ec955931e267f26262b0949bc16dc49"
            58 20                             # bytes(32)
              00000000000000000000000000000000
              00000000000000000000000000000000# "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000"
            58 20                             # bytes(32)
              DF1EAC9C7BD63473FFFB117F9873703E
              4EC955931E267F26262B0949BC16DC49# "\xDF\u001E\xAC\x9C{\xD64s\xFF\xFB\u0011\u007F\x98sp>N\xC9U\x93\u001E&\u007F&&+\tI\xBC\u0016\xDCI"

    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x01, 0x00, 0x84, 0x50, 0xc7,
                               0x3c, 0xb0, 0x45, 0xf9, 0xc2, 0x43, 0x45, 0x85,
                               0xaf, 0xfa, 0x36, 0xa3, 0x07, 0xbf, 0xe7, 0x78,
                               0x72, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f,
                               0x2f, 0x62, 0x6f, 0x61, 0x72, 0x64, 0x73, 0x2d,
                               0x69, 0x6e, 0x74, 0x2e, 0x6f, 0x6e, 0x69, 0x75,
                               0x64, 0x72, 0x61, 0x2e, 0x63, 0x63, 0x2f, 0x73,
                               0x74, 0x6f, 0x72, 0x61, 0x67, 0x65, 0x2f, 0x66,
                               0x69, 0x72, 0x6d, 0x77, 0x61, 0x72, 0x65, 0x2f,
                               0x76, 0x31, 0x2f, 0x64, 0x66, 0x31, 0x65, 0x61,
                               0x63, 0x39, 0x63, 0x37, 0x62, 0x64, 0x36, 0x33,
                               0x34, 0x37, 0x33, 0x66, 0x66, 0x66, 0x62, 0x31,
                               0x31, 0x37, 0x66, 0x39, 0x38, 0x37, 0x33, 0x37,
                               0x30, 0x33, 0x65, 0x34, 0x65, 0x63, 0x39, 0x35,
                               0x35, 0x39, 0x33, 0x31, 0x65, 0x32, 0x36, 0x37,
                               0x66, 0x32, 0x36, 0x32, 0x36, 0x32, 0x62, 0x30,
                               0x39, 0x34, 0x39, 0x62, 0x63, 0x31, 0x36, 0x64,
                               0x63, 0x34, 0x39, 0x58, 0x20, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x20, 0xdf,
                               0x1e, 0xac, 0x9c, 0x7b, 0xd6, 0x34, 0x73, 0xff,
                               0xfb, 0x11, 0x7f, 0x98, 0x73, 0x70, 0x3e, 0x4e,
                               0xc9, 0x55, 0x93, 0x1e, 0x26, 0x7f, 0x26, 0x26,
                               0x2b, 0x09, 0x49, 0xbc, 0x16, 0xdc, 0x49};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    uint8_t otaIdToMatch[ID_SIZE] = { 0xC7, 0x3C, 0xB0, 0x45, 0xF9, 0xC2, 0x43, 0x45,
                                      0x85, 0xAF, 0xFA, 0x36, 0xA3, 0x07, 0xBF, 0xE7};
    const char *urlToMatch   = "https://boards-int.oniudra.cc/storage/firmware/v1/df1eac9c7bd63473fffb117f9873703e4ec955931e267f26262b0949bc16dc49";

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(memcmp(command.otaUpdateCmdDown.params.id, otaIdToMatch, ID_SIZE) == 0);
      REQUIRE(strcmp(command.otaUpdateCmdDown.params.url, urlToMatch) == 0);
      // Initial SHA256 check
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[0] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[1] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[2] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[3] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[4] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[5] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[6] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[7] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[8] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[9] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[10] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[11] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[12] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[13] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[14] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[15] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[16] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[17] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[18] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[19] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[20] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[21] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[22] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[23] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[24] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[25] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[26] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[27] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[28] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[29] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[30] == (uint8_t)0x00);
      REQUIRE(command.otaUpdateCmdDown.params.initialSha256[31] == (uint8_t)0x00);

      // Final SHA256 check
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[0] == (uint8_t)0xdf);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[1] == (uint8_t)0x1e);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[2] == (uint8_t)0xac);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[3] == (uint8_t)0x9c);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[4] == (uint8_t)0x7b);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[5] == (uint8_t)0xd6);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[6] == (uint8_t)0x34);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[7] == (uint8_t)0x73);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[8] == (uint8_t)0xff);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[9] == (uint8_t)0xfb);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[10] == (uint8_t)0x11);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[11] == (uint8_t)0x7f);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[12] == (uint8_t)0x98);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[13] == (uint8_t)0x73);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[14] == (uint8_t)0x70);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[15] == (uint8_t)0x3e);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[16] == (uint8_t)0x4e);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[17] == (uint8_t)0xc9);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[18] == (uint8_t)0x55);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[19] == (uint8_t)0x93);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[20] == (uint8_t)0x1e);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[21] == (uint8_t)0x26);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[22] == (uint8_t)0x7f);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[23] == (uint8_t)0x26);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[24] == (uint8_t)0x26);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[25] == (uint8_t)0x2b);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[26] == (uint8_t)0x09);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[27] == (uint8_t)0x49);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[28] == (uint8_t)0xbc);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[29] == (uint8_t)0x16);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[30] == (uint8_t)0xdc);
      REQUIRE(command.otaUpdateCmdDown.params.finalSha256[31] == (uint8_t)0x49);

      REQUIRE(command.c.id == OtaUpdateCmdDownId);
    }
  }

/****************************************************************************/

  WHEN("Decode the OtaUpdateCmdDown message with out of order fields 1")
  {
    CommandDown command;

    /*
      DA 00010100                             # tag(65792)
        84                                    # array(4)
            78 72                             # text(141)
              68747470733A2F2F626F617264732D69
              6E742E6F6E69756472612E63632F7374
              6F726167652F6669726D776172652F76
              312F6466316561633963376264363334
              37336666666231313766393837333730
              33653465633935353933316532363766
              32363236326230393439626331366463
              3439                            # "https://boards-int.oniudra.cc/storage/firmware/v1/df1eac9c7bd63473fffb117f9873703e4ec955931e267f26262b0949bc16dc49"
            50                                # bytes(16)
              C73CB045F9C2434585AFFA36A307BFE7"\xC7<\xB0E\xF9\xC2CE\x85\xAF\xFA6\xA3\a\xBF\xE7"
            58 20                             # bytes(32)
              00000000000000000000000000000000
              00000000000000000000000000000000# "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000"
            58 20                             # bytes(32)
              DF1EAC9C7BD63473FFFB117F9873703E
              4EC955931E267F26262B0949BC16DC49# "\xDF\u001E\xAC\x9C{\xD64s\xFF\xFB\u0011\u007F\x98sp>N\xC9U\x93\u001E&\u007F&&+\tI\xBC\u0016\xDCI"

    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x01, 0x00, 0x84, 0x78, 0x72,
                               0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f, 0x2f,
                               0x62, 0x6f, 0x61, 0x72, 0x64, 0x73, 0x2d, 0x69,
                               0x6e, 0x74, 0x2e, 0x6f, 0x6e, 0x69, 0x75, 0x64,
                               0x72, 0x61, 0x2e, 0x63, 0x63, 0x2f, 0x73, 0x74,
                               0x6f, 0x72, 0x61, 0x67, 0x65, 0x2f, 0x66, 0x69,
                               0x72, 0x6d, 0x77, 0x61, 0x72, 0x65, 0x2f, 0x76,
                               0x31, 0x2f, 0x64, 0x66, 0x31, 0x65, 0x61, 0x63,
                               0x39, 0x63, 0x37, 0x62, 0x64, 0x36, 0x33, 0x34,
                               0x37, 0x33, 0x66, 0x66, 0x66, 0x62, 0x31, 0x31,
                               0x37, 0x66, 0x39, 0x38, 0x37, 0x33, 0x37, 0x30,
                               0x33, 0x65, 0x34, 0x65, 0x63, 0x39, 0x35, 0x35,
                               0x39, 0x33, 0x31, 0x65, 0x32, 0x36, 0x37, 0x66,
                               0x32, 0x36, 0x32, 0x36, 0x32, 0x62, 0x30, 0x39,
                               0x34, 0x39, 0x62, 0x63, 0x31, 0x36, 0x64, 0x63,
                               0x34, 0x39, 0x50, 0xc7, 0x3c, 0xb0, 0x45, 0xf9,
                               0xc2, 0x43, 0x45, 0x85, 0xaf, 0xfa, 0x36, 0xa3,
                               0x07, 0xbf, 0xe7, 0x58, 0x20, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x20, 0xdf,
                               0x1e, 0xac, 0x9c, 0x7b, 0xd6, 0x34, 0x73, 0xff,
                               0xfb, 0x11, 0x7f, 0x98, 0x73, 0x70, 0x3e, 0x4e,
                               0xc9, 0x55, 0x93, 0x1e, 0x26, 0x7f, 0x26, 0x26,
                               0x2b, 0x09, 0x49, 0xbc, 0x16, 0xdc, 0x49};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

/****************************************************************************/

  WHEN("Decode the OtaUpdateCmdDown message with out of order fields 2")
  {
    CommandDown command;

    /*
      DA 00010100                             # tag(65792)
        84                                    # array(4)
            50                                # bytes(16)
              C73CB045F9C2434585AFFA36A307BFE7"\xC7<\xB0E\xF9\xC2CE\x85\xAF\xFA6\xA3\a\xBF\xE7"
            58 20                             # bytes(32)
              00000000000000000000000000000000
              00000000000000000000000000000000# "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000"
            78 72                             # text(141)
              68747470733A2F2F626F617264732D69
              6E742E6F6E69756472612E63632F7374
              6F726167652F6669726D776172652F76
              312F6466316561633963376264363334
              37336666666231313766393837333730
              33653465633935353933316532363766
              32363236326230393439626331366463
              3439                            # "https://boards-int.oniudra.cc/storage/firmware/v1/df1eac9c7bd63473fffb117f9873703e4ec955931e267f26262b0949bc16dc49"
            58 20                             # bytes(32)
              DF1EAC9C7BD63473FFFB117F9873703E
              4EC955931E267F26262B0949BC16DC49# "\xDF\u001E\xAC\x9C{\xD64s\xFF\xFB\u0011\u007F\x98sp>N\xC9U\x93\u001E&\u007F&&+\tI\xBC\u0016\xDCI"

    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x01, 0x00, 0x84, 0x50, 0xc7,
                               0x3c, 0xb0, 0x45, 0xf9, 0xc2, 0x43, 0x45, 0x85,
                               0xaf, 0xfa, 0x36, 0xa3, 0x07, 0xbf, 0xe7, 0x58,
                               0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x78, 0x72, 0x68, 0x74, 0x74, 0x70, 0x73,
                               0x3a, 0x2f, 0x2f, 0x62, 0x6f, 0x61, 0x72, 0x64,
                               0x73, 0x2d, 0x69, 0x6e, 0x74, 0x2e, 0x6f, 0x6e,
                               0x69, 0x75, 0x64, 0x72, 0x61, 0x2e, 0x63, 0x63,
                               0x2f, 0x73, 0x74, 0x6f, 0x72, 0x61, 0x67, 0x65,
                               0x2f, 0x66, 0x69, 0x72, 0x6d, 0x77, 0x61, 0x72,
                               0x65, 0x2f, 0x76, 0x31, 0x2f, 0x64, 0x66, 0x31,
                               0x65, 0x61, 0x63, 0x39, 0x63, 0x37, 0x62, 0x64,
                               0x36, 0x33, 0x34, 0x37, 0x33, 0x66, 0x66, 0x66,
                               0x62, 0x31, 0x31, 0x37, 0x66, 0x39, 0x38, 0x37,
                               0x33, 0x37, 0x30, 0x33, 0x65, 0x34, 0x65, 0x63,
                               0x39, 0x35, 0x35, 0x39, 0x33, 0x31, 0x65, 0x32,
                               0x36, 0x37, 0x66, 0x32, 0x36, 0x32, 0x36, 0x32,
                               0x62, 0x30, 0x39, 0x34, 0x39, 0x62, 0x63, 0x31,
                               0x36, 0x64, 0x63, 0x34, 0x39, 0x58, 0x20, 0xdf,
                               0x1e, 0xac, 0x9c, 0x7b, 0xd6, 0x34, 0x73, 0xff,
                               0xfb, 0x11, 0x7f, 0x98, 0x73, 0x70, 0x3e, 0x4e,
                               0xc9, 0x55, 0x93, 0x1e, 0x26, 0x7f, 0x26, 0x26,
                               0x2b, 0x09, 0x49, 0xbc, 0x16, 0xdc, 0x49};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

/****************************************************************************/

  WHEN("Decode the OtaUpdateCmdDown message with corrupted fields 1")
  {
    CommandDown command;

    /*
      DA 00010100                             # tag(65792)
        84                                    # array(4)
            50                                # bytes(16)
              C73CB045F9C2434585AFFA36A307BFE7"\xC7<\xB0E\xF9\xC2CE\x85\xAF\xFA6\xA3\a\xBF\xE7"
            78 72                             # text(141)
              68747470733A2F2F626F617264732D69
              6E742E6F6E69756472612E63632F7374
              6F726167652F6669726D776172652F76
              312F6466316561633963376264363334
              37336666666231313766393837333730
              33653465633935353933316532363766
              32363236326230393439626331366463
              3439                            # "https://boards-int.oniudra.cc/storage/firmware/v1/df1eac9c7bd63473fffb117f9873703e4ec955931e267f26262b0949bc16dc49"
            1A 65DCB821                       # unsigned(1708963873)
            58 20                             # bytes(32)
              DF1EAC9C7BD63473FFFB117F9873703E
              4EC955931E267F26262B0949BC16DC49# "\xDF\u001E\xAC\x9C{\xD64s\xFF\xFB\u0011\u007F\x98sp>N\xC9U\x93\u001E&\u007F&&+\tI\xBC\u0016\xDCI"

    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x01, 0x00, 0x84, 0x50, 0xc7,
                               0x3c, 0xb0, 0x45, 0xf9, 0xc2, 0x43, 0x45, 0x85,
                               0xaf, 0xfa, 0x36, 0xa3, 0x07, 0xbf, 0xe7, 0x78,
                               0x72, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f,
                               0x2f, 0x62, 0x6f, 0x61, 0x72, 0x64, 0x73, 0x2d,
                               0x69, 0x6e, 0x74, 0x2e, 0x6f, 0x6e, 0x69, 0x75,
                               0x64, 0x72, 0x61, 0x2e, 0x63, 0x63, 0x2f, 0x73,
                               0x74, 0x6f, 0x72, 0x61, 0x67, 0x65, 0x2f, 0x66,
                               0x69, 0x72, 0x6d, 0x77, 0x61, 0x72, 0x65, 0x2f,
                               0x76, 0x31, 0x2f, 0x64, 0x66, 0x31, 0x65, 0x61,
                               0x63, 0x39, 0x63, 0x37, 0x62, 0x64, 0x36, 0x33,
                               0x34, 0x37, 0x33, 0x66, 0x66, 0x66, 0x62, 0x31,
                               0x31, 0x37, 0x66, 0x39, 0x38, 0x37, 0x33, 0x37,
                               0x30, 0x33, 0x65, 0x34, 0x65, 0x63, 0x39, 0x35,
                               0x35, 0x39, 0x33, 0x31, 0x65, 0x32, 0x36, 0x37,
                               0x66, 0x32, 0x36, 0x32, 0x36, 0x32, 0x62, 0x30,
                               0x39, 0x34, 0x39, 0x62, 0x63, 0x31, 0x36, 0x64,
                               0x63, 0x34, 0x39, 0x1A, 0x65, 0xDC, 0xB8, 0x21,
                               0x58, 0x20, 0xdf, 0x1e, 0xac, 0x9c, 0x7b, 0xd6,
                               0x34, 0x73, 0xff, 0xfb, 0x11, 0x7f, 0x98, 0x73,
                               0x70, 0x3e, 0x4e, 0xc9, 0x55, 0x93, 0x1e, 0x26,
                               0x7f, 0x26, 0x26, 0x2b, 0x09, 0x49, 0xbc, 0x16,
                               0xdc, 0x49};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

/****************************************************************************/

  WHEN("Decode the OtaUpdateCmdDown message with corrupted fields 2")
  {
    CommandDown command;

    /*
      DA 00010100                             # tag(65792)
        84                                    # array(4)
            50                                # bytes(16)
              C73CB045F9C2434585AFFA36A307BFE7"\xC7<\xB0E\xF9\xC2CE\x85\xAF\xFA6\xA3\a\xBF\xE7"
            78 72                             # text(141)
              68747470733A2F2F626F617264732D69
              6E742E6F6E69756472612E63632F7374
              6F726167652F6669726D776172652F76
              312F6466316561633963376264363334
              37336666666231313766393837333730
              33653465633935353933316532363766
              32363236326230393439626331366463
              3439                            # "https://boards-int.oniudra.cc/storage/firmware/v1/df1eac9c7bd63473fffb117f9873703e4ec955931e267f26262b0949bc16dc49"
            58 20                             # bytes(32)
              DF1EAC9C7BD63473FFFB117F9873703E
              4EC955931E267F26262B0949BC16DC49# "\xDF\u001E\xAC\x9C{\xD64s\xFF\xFB\u0011\u007F\x98sp>N\xC9U\x93\u001E&\u007F&&+\tI\xBC\u0016\xDCI"
            1A 65DCB821                       # unsigned(1708963873)

    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x01, 0x00, 0x84, 0x50, 0xc7,
                               0x3c, 0xb0, 0x45, 0xf9, 0xc2, 0x43, 0x45, 0x85,
                               0xaf, 0xfa, 0x36, 0xa3, 0x07, 0xbf, 0xe7, 0x78,
                               0x72, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f,
                               0x2f, 0x62, 0x6f, 0x61, 0x72, 0x64, 0x73, 0x2d,
                               0x69, 0x6e, 0x74, 0x2e, 0x6f, 0x6e, 0x69, 0x75,
                               0x64, 0x72, 0x61, 0x2e, 0x63, 0x63, 0x2f, 0x73,
                               0x74, 0x6f, 0x72, 0x61, 0x67, 0x65, 0x2f, 0x66,
                               0x69, 0x72, 0x6d, 0x77, 0x61, 0x72, 0x65, 0x2f,
                               0x76, 0x31, 0x2f, 0x64, 0x66, 0x31, 0x65, 0x61,
                               0x63, 0x39, 0x63, 0x37, 0x62, 0x64, 0x36, 0x33,
                               0x34, 0x37, 0x33, 0x66, 0x66, 0x66, 0x62, 0x31,
                               0x31, 0x37, 0x66, 0x39, 0x38, 0x37, 0x33, 0x37,
                               0x30, 0x33, 0x65, 0x34, 0x65, 0x63, 0x39, 0x35,
                               0x35, 0x39, 0x33, 0x31, 0x65, 0x32, 0x36, 0x37,
                               0x66, 0x32, 0x36, 0x32, 0x36, 0x32, 0x62, 0x30,
                               0x39, 0x34, 0x39, 0x62, 0x63, 0x31, 0x36, 0x64,
                               0x63, 0x34, 0x39, 0x58, 0x20, 0xdf, 0x1e, 0xac,
                               0x9c, 0x7b, 0xd6, 0x34, 0x73, 0xff, 0xfb, 0x11,
                               0x7f, 0x98, 0x73, 0x70, 0x3e, 0x4e, 0xc9, 0x55,
                               0x93, 0x1e, 0x26, 0x7f, 0x26, 0x26, 0x2b, 0x09,
                               0x49, 0xbc, 0x16, 0xdc, 0x49, 0x1A, 0x65, 0xDC,
                               0xB8, 0x21};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/

  WHEN("Decode the OtaBeginUp message")
  {
    CommandDown command;
    /*
      DA 00010000                             # tag(65536)
         81                                   # array(1)
            58 20                             # bytes(32)
               01020304
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x00, 0x00, 0x81, 0x58, 0x20,
                               0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful - OtaBeginUp is not supported") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/

  WHEN("Decode the ThingBeginCmd message")
  {
    CommandDown command;
    /*
      DA 00010300               # tag(66304)
         81                     # array(1)
            68                  # text(8)
               7468696E675F6964 # "thing_id"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x03, 0x00, 0x81, 0x68, 0x74,
                               0x68, 0x69, 0x6e, 0x67, 0x5f, 0x69, 0x64};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful - ThingBeginCmd is not supported") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/

  WHEN("Decode the LastValuesBeginCmd message")
  {
    CommandDown command;
    /*
       DA 00010500 # tag(66816)
         80        # array(0)
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x05, 0x00, 0x80};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful - LastValuesBeginCmd is not supported") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/

  WHEN("Decode the DeviceBeginCmd message")
  {
    CommandDown command;
    /*
      DA 00010700         # tag(67328)
         81               # array(1)
            65            # text(5)
               322E302E30 # "2.0.0"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x07, 0x00, 0x81, 0x65, 0x32,
                               0x2e, 0x30, 0x2e, 0x30};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful - DeviceBeginCmd is not supported") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/

  WHEN("Decode the OtaProgressCmdUp message")
  {
    CommandDown command;
    /*
      DA 00010200                             # tag(66048)
         84                                   # array(4)
            50                                # bytes(16)
               000102030405060708090A0B0C0D0E0F # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\n\u000b\f\r\u000e\u000f"
            E1                                # primitive(1)
            20                                # negative(0)
            18 64                             # unsigned(100)
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x02, 0x00, 0x84, 0x50, 0x00,
                               0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                               0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xe1,
                               0x20, 0x18, 0x64};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful - OtaProgressCmdUp is not supported") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/

  WHEN("Decode the TimezoneCommandUp message")
  {
    CommandDown command;
    /*
      DA 00010800 # tag(67584)
         80       # array(0)
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x08, 0x00, 0x80};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful - TimezoneCommandUp is not supported") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/

  WHEN("Decode a message with invalid CBOR tag")
  {
    CommandDown command;

    /*
      DA ffffffff       # invalid tag
        82              # array(2)
            1A 65DCB821 # unsigned(1708963873)
            1A 78ACA191 # unsigned(2024579473)
    */

    uint8_t const payload[] = {0xDA, 0xff, 0xff, 0xff, 0xff, 0x82, 0x1A, 0x65,
                               0xDC, 0xB8, 0x21, 0x1A, 0x78, 0xAC, 0xA1, 0x91};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/

  WHEN("Decode a message not starting with a CBOR tag")
  {
    CommandDown command;

    /*
        82              # array(2)
            1A 65DCB821 # unsigned(1708963873)
            1A 78ACA191 # unsigned(2024579473)
    */

    uint8_t const payload[] = {0x82, 0x1A, 0x65, 0xDC, 0xB8, 0x21, 0x1A, 0x78,
                               0xAC, 0xA1, 0x91};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/

  WHEN("Decode an invalid CBOR message")
  {
    CommandDown command;

    uint8_t const payload[] = {0xFF};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning timestamp message")
  {
    ProvisioningCommandDown command;

    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x02, 0x81, 0x1B, 0x00, 0x00, 
    0x00, 0x00, 0x67, 0x06, 0x6F, 0xE4};
    /*
    DA 00012002             # tag(73730)
      81                    # array(1)
        1B 0000000067066FE4 # unsigned(1728475108)
    */
    uint64_t ts = 1728475108;
    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningTimestamp);
      REQUIRE(command.provisioningTimestamp.params.timestamp == ts);
    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning command message")
  {
    ProvisioningCommandDown command;
    /*
      DA 00012003 # tag(73731)
        81       # array(1)
          18 64 # unsigned(100)
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x03, 0x81, 0x18, 0x64};
    uint8_t commandId = 100;
    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningCommands);
      REQUIRE(command.provisioningCommands.params.cmd == commandId);
    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning wifi configuration message")
  {
    ProvisioningCommandDown command;
    /*
      DA 00012004                      # tag(73732)
        82                             # array(2)
          65                           # text(5)
            5353494431                 # "SSID1"
          6D                           # text(13)
            50415353574F52445353494431 # "PASSWORDSSID1"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x04, 0x82, 0x65, 0x53,
                               0x53, 0x49, 0x44, 0x31, 0x6d, 0x50, 0x41, 0x53,
                               0x53, 0x57, 0x4f, 0x52, 0x44, 0x53, 0x53, 0x49,
                               0x44, 0x31};
    char ssid[] = "SSID1";
    char password[] = "PASSWORDSSID1";
    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningWifiConfig);
      REQUIRE(strcmp(command.provisioningWifiConfig.params.ssid, ssid) == 0);
      REQUIRE(strcmp(command.provisioningWifiConfig.params.pwd, password) == 0);
    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning LoRaWan configuration message")
  {
    ProvisioningCommandDown command;
    /*
      DA 00012005             # tag(73733)
   85                   # array(5)
      67                # text(7)
         41505045554931 # "APPEUI1"
      66                # text(6)
         4150504B4559   # "APPKEY"
      05                # unsigned(5)
      65                # text(5)
         3031313130     # "01110"
      61                # text(1)
         41             # "A"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x05, 0x85, 0x67, 0x41,
                               0x50, 0x50, 0x45, 0x55, 0x49, 0x31, 0x66, 0x41,
                               0x50, 0x50, 0x4B, 0x45, 0x59, 0x05, 0x65, 0x30,
                               0x31, 0x31, 0x31, 0x30, 0x61, 0x41};
    char appeui[] = "APPEUI1";
    char appkey[] = "APPKEY";
    char channelMask[] = "01110";
    char deviceType[] = "A";
    uint8_t band = 5;
    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningLoRaConfig);
      REQUIRE(strcmp(command.provisioningLoRaConfig.params.appeui, appeui) == 0);
      REQUIRE(strcmp(command.provisioningLoRaConfig.params.appkey, appkey) == 0);
      REQUIRE(command.provisioningLoRaConfig.params.band == band);
      REQUIRE(strcmp(command.provisioningLoRaConfig.params.channelMask, channelMask) == 0);
      REQUIRE(strcmp(command.provisioningLoRaConfig.params.deviceClass, deviceType) == 0);
    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning GSM complete configuration message")
  {
    ProvisioningCommandDown command;
    /*
      DA 00012006                        # tag(73734)
        84                               # array(4)
          68                             # text(8)
            3132333435363738             # "12345678"
          6E                             # text(14)
            61706E2E61726475696E6F2E6363 # "apn.arduino.cc"
          68                             # text(8)
            5445535455534552             # "TESTUSER"
          6C                             # text(12)
            5445535450415353574F5244     # "TESTPASSWORD"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x06, 0x84, 0x68, 0x31,
                               0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x6e,
                               0x61, 0x70, 0x6e, 0x2e, 0x61, 0x72, 0x64, 0x75,
                               0x69, 0x6e, 0x6f, 0x2e, 0x63, 0x63, 0x68, 0x54,
                               0x45, 0x53, 0x54, 0x55, 0x53, 0x45, 0x52, 0x6c,
                               0x54, 0x45, 0x53, 0x54, 0x50, 0x41, 0x53, 0x53,
                               0x57, 0x4f, 0x52, 0x44};
    char pin[] = "12345678";
    char apn[] = "apn.arduino.cc";
    char username[] = "TESTUSER";
    char password[] = "TESTPASSWORD";

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningGSMConfig);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.pin, pin) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.apn, apn) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.login, username) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.pass, password) == 0);

    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning GSM configuration message with empty PIN")
  {
    ProvisioningCommandDown command;
    /*
      DA 00012006                          # tag(73734)
        84                                 # array(4)
            60                             # text(0)
                                           # ""
            6E                             # text(14)
              61706E2E61726475696E6F2E6363 # "apn.arduino.cc"
            68                             # text(8)
              5445535455534552             # "TESTUSER"
            6C                             # text(12)
              5445535450415353574F5244     # "TESTPASSWORD"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x06, 0x84, 0x60, 0x6e, 0x61,
                               0x70, 0x6e, 0x2e, 0x61, 0x72, 0x64, 0x75, 0x69, 0x6e,
                               0x6f, 0x2e, 0x63, 0x63, 0x68, 0x54, 0x45, 0x53, 0x54,
                               0x55, 0x53, 0x45, 0x52, 0x6c, 0x54, 0x45, 0x53, 0x54,
                               0x50, 0x41, 0x53, 0x53, 0x57, 0x4f, 0x52, 0x44};
    char pin[] = "";
    char apn[] = "apn.arduino.cc";
    char username[] = "TESTUSER";
    char password[] = "TESTPASSWORD";

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningGSMConfig);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.pin, pin) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.apn, apn) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.login, username) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.pass, password) == 0);

    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning GSM configuration message with PIN, Login and Password empty")
  {
    ProvisioningCommandDown command;
    /*
      DA 00012006                          # tag(73734)
        84                                 # array(4)
            60                             # text(0)
                                           # ""
            6E                             # text(14)
              61706E2E61726475696E6F2E6363 # "apn.arduino.cc"
            60                             # text(0)
                                           # ""
            60                             # text(0)
                                           # ""
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x06, 0x84, 0x60, 0x6e, 0x61,
                               0x70, 0x6e, 0x2e, 0x61, 0x72, 0x64, 0x75, 0x69, 0x6e,
                               0x6f, 0x2e, 0x63, 0x63, 0x60, 0x60};
    char pin[] = "";
    char apn[] = "apn.arduino.cc";
    char username[] = "";
    char password[] = "";

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningGSMConfig);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.pin, pin) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.apn, apn) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.login, username) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.pass, password) == 0);

    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning NB-IoT complete configuration message")
  {
    ProvisioningCommandDown command;
    /*
      DA 00012006                        # tag(73734)
        84                               # array(4)
          68                             # text(8)
            3132333435363738             # "12345678"
          6E                             # text(14)
            61706E2E61726475696E6F2E6363 # "apn.arduino.cc"
          68                             # text(8)
            5445535455534552             # "TESTUSER"
          6C                             # text(12)
            5445535450415353574F5244     # "TESTPASSWORD"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x07, 0x84, 0x68, 0x31,
                               0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x6e,
                               0x61, 0x70, 0x6e, 0x2e, 0x61, 0x72, 0x64, 0x75,
                               0x69, 0x6e, 0x6f, 0x2e, 0x63, 0x63, 0x68, 0x54,
                               0x45, 0x53, 0x54, 0x55, 0x53, 0x45, 0x52, 0x6c,
                               0x54, 0x45, 0x53, 0x54, 0x50, 0x41, 0x53, 0x53,
                               0x57, 0x4f, 0x52, 0x44};
    char pin[] = "12345678";
    char apn[] = "apn.arduino.cc";
    char username[] = "TESTUSER";
    char password[] = "TESTPASSWORD";

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningNBIOTConfig);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.pin, pin) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.apn, apn) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.login, username) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.pass, password) == 0);

    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning CATM1 complete configuration message")
  {
    ProvisioningCommandDown command;
    /*
    DA 00012008                          # tag(73736)
      85                                 # array(5)
          68                             # text(8)
            3132333435363738             # "12345678"
          84                             # array(4)
            01                           # unsigned(1)
            02                           # unsigned(2)
            1A 00080000                  # unsigned(524288)
            1A 08000000                  # unsigned(134217728)
          6E                             # text(14)
            61706E2E61726475696E6F2E6363 # "apn.arduino.cc"
          68                             # text(8)
            5445535455534552             # "TESTUSER"
          6C                             # text(12)
            5445535450415353574F5244     # "TESTPASSWORD"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x08, 0x85, 0x68, 0x31,
                               0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x84,
                               0x01, 0x02, 0x1A, 0x00, 0x08, 0x00, 0x00, 0x1A,
                               0x08, 0x00, 0x00, 0x00, 0x6E, 0x61, 0x70, 0x6E,
                               0x2E, 0x61, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F,
                               0x2E, 0x63, 0x63, 0x68, 0x54, 0x45, 0x53, 0x54,
                               0x55, 0x53, 0x45, 0x52, 0x6C, 0x54, 0x45, 0x53,
                               0x54, 0x50, 0x41, 0x53, 0x53, 0x57, 0x4F, 0x52,
                               0x44};
    char pin[] = "12345678";
    char apn[] = "apn.arduino.cc";
    char username[] = "TESTUSER";
    char password[] = "TESTPASSWORD";
    uint32_t band[] = {0x01, 0x02, 0x80000, 0x8000000};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);
    
    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningCATM1Config);
      REQUIRE(strcmp(command.provisioningCATM1Config.params.pin, pin) == 0);
      REQUIRE(strcmp(command.provisioningCATM1Config.params.apn, apn) == 0);
      REQUIRE(strcmp(command.provisioningCATM1Config.params.login, username) == 0);
      REQUIRE(strcmp(command.provisioningCATM1Config.params.pass, password) == 0);
      REQUIRE(memcmp(command.provisioningCATM1Config.params.band, band, sizeof(band)) == 0);
    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning CATM1 configuration message with only one band")
  {
    ProvisioningCommandDown command;
    /*
    DA 00012008                           # tag(73736)
      85                                  # array(5)
          68                              # text(8)
            3132333435363738              # "12345678"
          81                              # array(1)
            01                            # unsigned(1)
          6E                              # text(14)
            61706E2E61726475696E6F2E6363  # "apn.arduino.cc"
          68                              # text(8)
            5445535455534552              # "TESTUSER"
          6C                              # text(12)
            5445535450415353574F5244      # "TESTPASSWORD"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x08, 0x85, 0x68, 0x31,
                               0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x81,
                               0x01, 0x6E, 0x61, 0x70, 0x6E, 0x2E, 0x61, 0x72,
                               0x64, 0x75, 0x69, 0x6E, 0x6F, 0x2E, 0x63, 0x63,
                               0x68, 0x54, 0x45, 0x53, 0x54, 0x55, 0x53, 0x45,
                               0x52, 0x6C, 0x54, 0x45, 0x53, 0x54, 0x50, 0x41,
                               0x53, 0x53, 0x57, 0x4F, 0x52, 0x44};
    char pin[] = "12345678";
    char apn[] = "apn.arduino.cc";
    char username[] = "TESTUSER";
    char password[] = "TESTPASSWORD";
    uint32_t band[] = {0x01, 0x00, 0x00, 0x00};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);
    
    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningCATM1Config);
      REQUIRE(strcmp(command.provisioningCATM1Config.params.pin, pin) == 0);
      REQUIRE(strcmp(command.provisioningCATM1Config.params.apn, apn) == 0);
      REQUIRE(strcmp(command.provisioningCATM1Config.params.login, username) == 0);
      REQUIRE(strcmp(command.provisioningCATM1Config.params.pass, password) == 0);
      REQUIRE(memcmp(command.provisioningCATM1Config.params.band, band, sizeof(band)) == 0);
    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning CATM1 configuration message without band, username, password")
  {
    ProvisioningCommandDown command;
    /*
    DA 00012008                           # tag(73736)
      85                                  # array(5)
          68                              # text(8)
            3132333435363738              # "12345678"
          80                              # array(0)
          6E                              # text(14)
            61706E2E61726475696E6F2E6363  # "apn.arduino.cc"
          60                              # text(0)
                                          # ""
          60                              # text(0)
                                          # ""
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x08, 0x85, 0x68, 0x31,
                               0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x80,
                               0x6E, 0x61, 0x70, 0x6E, 0x2E, 0x61, 0x72, 0x64,
                               0x75, 0x69, 0x6E, 0x6F, 0x2E, 0x63, 0x63, 0x60,
                               0x60};
    char pin[] = "12345678";
    char apn[] = "apn.arduino.cc";
    char username[] = "";
    char password[] = "";
    uint32_t band[] = {0x00, 0x00, 0x00, 0x00};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);
    
    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningCATM1Config);
      REQUIRE(strcmp(command.provisioningCATM1Config.params.pin, pin) == 0);
      REQUIRE(strcmp(command.provisioningCATM1Config.params.apn, apn) == 0);
      REQUIRE(strcmp(command.provisioningCATM1Config.params.login, username) == 0);
      REQUIRE(strcmp(command.provisioningCATM1Config.params.pass, password) == 0);
      REQUIRE(memcmp(command.provisioningCATM1Config.params.band, band, sizeof(band)) == 0);
    }
  }
  /****************************************************************************/
  WHEN("Decode a provisioning CATM1 configuration message with more bands than allowed")
  {
    ProvisioningCommandDown command;
    /*
    DA 00012008                           # tag(73736)
      85                                  # array(5)
          68                              # text(8)
            3132333435363738              # "12345678"
          85                              # array(5)
            01                            # unsigned(1)
            02                            # unsigned(2)
            1A 00080000                   # unsigned(524288)
            1A 08000000                   # unsigned(134217728)
            1A 08000000                   # unsigned(134217728)
          6E                              # text(14)
            61706E2E61726475696E6F2E6363  # "apn.arduino.cc"
          68                              # text(8)
            5445535455534552              # "TESTUSER"
          6C                              # text(12)
            5445535450415353574F5244      # "TESTPASSWORD"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x08, 0x85, 0x68, 0x31,
                               0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x85,
                               0x01, 0x02, 0x1A, 0x00, 0x08, 0x00, 0x00, 0x1A,
                               0x08, 0x00, 0x00, 0x00, 0x1A, 0x08, 0x00, 0x00,
                               0x00, 0x6E, 0x61, 0x70, 0x6E, 0x2E, 0x61, 0x72,
                               0x64, 0x75, 0x69, 0x6E, 0x6F, 0x2E, 0x63, 0x63,
                               0x68, 0x54, 0x45, 0x53, 0x54, 0x55, 0x53, 0x45,
                               0x52, 0x6C, 0x54, 0x45, 0x53, 0x54, 0x50, 0x41,
                               0x53, 0x53, 0x57, 0x4F, 0x52, 0x44};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);
    
    THEN("The decode is error") {
      REQUIRE(err == Decoder::Status::Error);
    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning Ethernet complete configuration message with IPv4")
  {
    ProvisioningCommandDown command;
    /*
    DA 00012009       # tag(73737)
      86              # array(6)
          58 04       # bytes(4)
            C0A80002  # "\xC0\xA8\u0000\u0002"
          58 04       # bytes(4)
            08080808  # "\b\b\b\b"
          58 04       # bytes(4)
            C0A80101  # "\xC0\xA8\u0001\u0001"
          58 04       # bytes(4)
            FFFFFF00  # "\xFF\xFF\xFF\u0000"
          0F          # unsigned(15)
          18 C8       # unsigned(200)
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x09, 0x86, 0x58, 0x04,
                               0xC0, 0xA8, 0x00, 0x02, 0x58, 0x04, 0x08, 0x08,
                               0x08, 0x08, 0x58, 0x04, 0xC0, 0xA8, 0x01, 0x01,
                               0x58, 0x04, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0x18,
                               0xC8};

    uint8_t ip[] = {192, 168, 0, 2};
    uint8_t dns[] = {8, 8, 8, 8};
    uint8_t gateway[] = {192, 168, 1, 1};
    uint8_t netmask[] = {255, 255, 255, 0};
    uint32_t timeout = 15;
    uint32_t responseTimeout = 200;

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);
    
    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningEthernetConfig);
      REQUIRE(command.provisioningEthernetConfig.params.ip.type == ProvisioningIPStruct::IPType::IPV4);
      REQUIRE(memcmp(command.provisioningEthernetConfig.params.ip.ip, ip, sizeof(ip)) == 0);
      REQUIRE(command.provisioningEthernetConfig.params.dns.type == ProvisioningIPStruct::IPType::IPV4);
      REQUIRE(memcmp(command.provisioningEthernetConfig.params.dns.ip, dns, sizeof(dns)) == 0);
      REQUIRE(command.provisioningEthernetConfig.params.gateway.type == ProvisioningIPStruct::IPType::IPV4);
      REQUIRE(memcmp(command.provisioningEthernetConfig.params.gateway.ip, gateway, sizeof(gateway)) == 0);
      REQUIRE(command.provisioningEthernetConfig.params.netmask.type == ProvisioningIPStruct::IPType::IPV4);
      REQUIRE(memcmp(command.provisioningEthernetConfig.params.netmask.ip, netmask, sizeof(netmask)) == 0);
      REQUIRE(command.provisioningEthernetConfig.params.timeout == timeout);
      REQUIRE(command.provisioningEthernetConfig.params.response_timeout == responseTimeout);
    }
  }

  /****************************************************************************/

  WHEN("Decode a provisioning Ethernet complete configuration message with IPv6")
  {
    ProvisioningCommandDown command;
    /*
    DA 00012009                              # tag(73737)
      86                                     # array(6)
          58 10                              # bytes(16)
            1A4FA7A9928F7B1CEC3B1ECD88580D1E # "\u001AO\xA7\xA9\x92\x8F{\u001C\xEC;\u001E͈X\r\u001E"
          58 10                              # bytes(16)
            21F63B22996F5B7225D9E024F036B5D2 # "!\xF6;\"\x99o[r%\xD9\xE0$\xF06\xB5\xD2"
          58 10                              # bytes(16)
            2EC227F1F19A0C11471B84AF9610B017 # ".\xC2'\xF1\xF1\x9A\f\u0011G\e\x84\xAF\x96\u0010\xB0\u0017"
          58 10                              # bytes(16)
            FFFFFFFFFFFFFFFF0000000000000000 # "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000"
          0F                                 # unsigned(15)
          18 C8                              # unsigned(200)
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x09, 0x86, 0x58, 0x10,
                               0x1A, 0x4F, 0xA7, 0xA9, 0x92, 0x8F, 0x7B, 0x1C,
                               0xEC, 0x3B, 0x1E, 0xCD, 0x88, 0x58, 0x0D, 0x1E,
                               0x58, 0x10, 0x21, 0xF6, 0x3B, 0x22, 0x99, 0x6F,
                               0x5B, 0x72, 0x25, 0xD9, 0xE0, 0x24, 0xF0, 0x36,
                               0xB5, 0xD2, 0x58, 0x10, 0x2E, 0xC2, 0x27, 0xF1,
                               0xF1, 0x9A, 0x0C, 0x11, 0x47, 0x1B, 0x84, 0xAF,
                               0x96, 0x10, 0xB0, 0x17, 0x58, 0x10, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x18,
                               0xC8};
                               

    uint8_t ip[] = {0x1a, 0x4f, 0xa7, 0xa9, 0x92, 0x8f, 0x7b, 0x1c, 0xec, 0x3b, 0x1e, 0xcd, 0x88, 0x58, 0x0d, 0x1e};
    uint8_t dns[] = {0x21, 0xf6, 0x3b, 0x22, 0x99, 0x6f, 0x5b, 0x72, 0x25, 0xd9, 0xe0, 0x24, 0xf0, 0x36, 0xb5, 0xd2};
    uint8_t gateway[] = {0x2e, 0xc2, 0x27, 0xf1, 0xf1, 0x9a, 0x0c, 0x11, 0x47, 0x1b, 0x84, 0xaf, 0x96, 0x10, 0xb0, 0x17};
    uint8_t netmask[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint32_t timeout = 15;
    uint32_t responseTimeout = 200;

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningEthernetConfig);
      REQUIRE(command.provisioningEthernetConfig.params.ip.type == ProvisioningIPStruct::IPType::IPV6);
      REQUIRE(memcmp(command.provisioningEthernetConfig.params.ip.ip, ip, sizeof(ip)) == 0);
      REQUIRE(command.provisioningEthernetConfig.params.dns.type == ProvisioningIPStruct::IPType::IPV6);
      REQUIRE(memcmp(command.provisioningEthernetConfig.params.dns.ip, dns, sizeof(dns)) == 0);
      REQUIRE(command.provisioningEthernetConfig.params.gateway.type == ProvisioningIPStruct::IPType::IPV6);
      REQUIRE(memcmp(command.provisioningEthernetConfig.params.gateway.ip, gateway, sizeof(gateway)) == 0);
      REQUIRE(command.provisioningEthernetConfig.params.netmask.type == ProvisioningIPStruct::IPType::IPV6);
      REQUIRE(memcmp(command.provisioningEthernetConfig.params.netmask.ip, netmask, sizeof(netmask)) == 0);
      REQUIRE(command.provisioningEthernetConfig.params.timeout == timeout);
      REQUIRE(command.provisioningEthernetConfig.params.response_timeout == responseTimeout);
    }
  }

  /****************************************************************************/

   WHEN("Decode a provisioning Cellular complete configuration message")
  {
    ProvisioningCommandDown command;
    /*
      DA 00012012                        # tag(73746)
        84                               # array(4)
          68                             # text(8)
            3132333435363738             # "12345678"
          6E                             # text(14)
            61706E2E61726475696E6F2E6363 # "apn.arduino.cc"
          68                             # text(8)
            5445535455534552             # "TESTUSER"
          6C                             # text(12)
            5445535450415353574F5244     # "TESTPASSWORD"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x20, 0x12, 0x84, 0x68, 0x31,
                               0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x6e,
                               0x61, 0x70, 0x6e, 0x2e, 0x61, 0x72, 0x64, 0x75,
                               0x69, 0x6e, 0x6f, 0x2e, 0x63, 0x63, 0x68, 0x54,
                               0x45, 0x53, 0x54, 0x55, 0x53, 0x45, 0x52, 0x6c,
                               0x54, 0x45, 0x53, 0x54, 0x50, 0x41, 0x53, 0x53,
                               0x57, 0x4f, 0x52, 0x44};
    char pin[] = "12345678";
    char apn[] = "apn.arduino.cc";
    char username[] = "TESTUSER";
    char password[] = "TESTPASSWORD";

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.c.id == CommandId::ProvisioningCellularConfig);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.pin, pin) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.apn, apn) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.login, username) == 0);
      REQUIRE(strcmp(command.provisioningCellularConfig.params.pass, password) == 0);

    }
  }

}
