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

#pragma once

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#if OTA_ENABLED
#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <Arduino_ConnectionHandler.h>
#include <interfaces/CloudProcess.h>
#include "utility/lzss/lzss.h"

union HeaderVersion {
  struct __attribute__((packed)) {
    uint32_t header_version    :  6;
    uint32_t compression       :  1;
    uint32_t signature         :  1;
    uint32_t spare             :  4;
    uint32_t payload_target    :  4;
    uint32_t payload_major     :  8;
    uint32_t payload_minor     :  8;
    uint32_t payload_patch     :  8;
    uint32_t payload_build_num : 24;
  } field;
  uint8_t buf[sizeof(field)];
  static_assert(sizeof(buf) == 8, "Error: sizeof(HEADER.VERSION) != 8");
};

union OTAHeader {
  struct __attribute__((packed)) {
    uint32_t len;
    uint32_t crc32;
    uint32_t magic_number;
    HeaderVersion hdr_version;
  } header;
  uint8_t buf[sizeof(header)];
  static_assert(sizeof(buf) == 20, "Error: sizeof(HEADER) != 20");
};

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class OTACloudProcessInterface: public CloudProcess {
public:
  OTACloudProcessInterface();
  OTACloudProcessInterface(const OTACloudProcessInterface&) = delete;
  OTACloudProcessInterface(OTACloudProcessInterface&&) = delete;

  ~OTACloudProcessInterface();

  enum State: int16_t {
    Resume,
    Idle,
    OtaAvailable,
    StartOTA,
    Fetch,
    VerifyOTA,
    FlashOTA,
    Reboot,

    // Error states that may generically happen on all board
    NoCapableBootloaderFail   = -1,
    NoOtaStorageFail          = -2,
    OtaStorageInitFail        = -3,
    OtaStorageOpenFail        = -4,
    OtaHeaderLengthFail       = -5,
    OtaHeaderCrcFail          = -6,
    OtaHeaterMagicNumberFail  = -7,
    ParseHttpHeaderFail       = -8,
    UrlParseErrorFail         = -9,
    ServerConnectErrorFail    = -10,
    HttpHeaderErrorFail       = -11,
    OtaDownloadFail           = -12,
    OtaHeaderTimeoutFail      = -13,
    HttpResponseFail          = -14,
    OtaStorageEndFail         = -15,
    StorageConfigFail         = -16,
    LibraryFail               = -17,
    ModemFail                 = -18,
    ErrorOpenUpdateFileFail   = -19,
    ErrorWriteUpdateFileFail  = -20,
    ErrorReformatFail         = -21,
    ErrorUnmountFail          = -22,
    ErrorRenameFail           = -23,
  };

  static constexpr char* const STATE_NAMES[];

  enum OtaFlags: uint16_t {
    None              = 0,
    ApprovalRequired  = 1,
    Approved          = 1<<1
  };

  virtual void handleMessage(Message*);
  // virtual CloudProcess::State getState();
  // virtual void hook(State s, void* action);
  virtual inline void update() { handleMessage(nullptr); }

  inline void approveOta() { policies |= Confirmation; }

  inline virtual void setClient(Client* c) { client = c; }
protected:
  // The following methods represent the FSM actions performed in each state

  // the first state is 'resume', where we try to understand if we are booting after a ota
  // the objective is to understand the result and report it to the cloud
  virtual State resume(Message* msg=nullptr) = 0;

  // this state is the normal state where no action has to be performed. We may poll the cloud
  // for updates in this state
  virtual State idle(Message* msg=nullptr);

  // we go in this state if there is an ota available, depending on the policies implemented we may
  // start the ota or wait for an user interaction
  virtual State otaAvailable();

  // we start the process of ota update and wait for the server to respond with the ota url and other info
  virtual State startOTA();

  // we start the download and decompress process
  virtual State fetch() = 0;

  // whene the download is correctly finished we set the mcu to use the newly downloaded binary
  virtual State flashOTA() = 0;

  // we reboot the device
  virtual State reboot() = 0;

  // if any of the steps described fail we get into this state and report to the cloud what happened
  // then we go back to idle state
  virtual State fail();

  // TODO virtual =0 method used to reset the status of the failed ota operation
  // TODO depending on the state in which we failed we may have different fail context
  virtual void reset() = 0;

  // this method will be called by fetch to write the ota binary on the device flash
  // this method is directly called by fetch.
  // This method will be overridden by the actual board implementation
  virtual int writeFlash(uint8_t* const buffer, size_t len) = 0;

  OtaPolicies policies; // TODO getter and setters for this

  inline void updateState(State s) { if(state!=s) { sprevious_state = state; state = s; } }

  // This method is called to report the current state of the OtaClass
  void reportStatus();
private:
  void clean();
  void parseOta(uint8_t* buffer, size_t buf_len);

  State state, previous_state;
  Client*     client;
  SSLClient*  ssl_client
  HttpClient* http_client;

  enum OTADownloadState: uint8_t {
    OtaDownloadHeader,
    OtaDownloadFile,
    OtaDownloadCompleted,
    OtaDownloadError
  };

protected:
  struct OtaContext {
    OtaContext(
      const char* id, const char* url,
      uint8_t initialSha256[32], uint8_t finalSha256[32],
      std::function<void(uint8_t)> putc);
    ~OtaContext();
    // parameters present in the ota available message that are of interest of the process
    const char* id;
    ParsedUrl   url; // TODO replace with parsed url struct
    uint8_t     initialSha256[32];
    uint8_t     finalSha256[32];

    OTAHeader         header;
    OTADownloadState  downloadState;
    uint32_t          calculatedCrc32;
    uint32_t          headerCopiedBytes;
    uint32_t          downloadedSize;

    // LZSS decoder
    LZSSDecoder       decoder;

    uint16_t          report_couter;
  } *context;
};



#endif /* OTA_ENABLED */