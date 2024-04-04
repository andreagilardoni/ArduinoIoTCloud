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
#include "../OTATypes.h"
#include "tls/utility/SHA256.h"

#include <interfaces/CloudProcess.h>
#include <Arduino_DebugUtils.h>

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class OTACloudProcessInterface: public CloudProcess {
public:
  OTACloudProcessInterface(MessageStream *ms);

  virtual ~OTACloudProcessInterface();

  enum State: int16_t {
    Resume                    = 0,
    OtaBegin                  = 1,
    Idle                      = 2,
    OtaAvailable              = 3,
    StartOTA                  = 4,
    Fetch                     = 5,
    FlashOTA                  = 6,
    Reboot                    = 7,
    Fail                      = 8,

    // Error states that may generically happen on all board
    NoCapableBootloaderFail   = static_cast<State>(ota::OTAError::NoCapableBootloader),
    NoOtaStorageFail          = static_cast<State>(ota::OTAError::NoOtaStorage),
    OtaStorageInitFail        = static_cast<State>(ota::OTAError::OtaStorageInit),
    OtaStorageOpenFail        = static_cast<State>(ota::OTAError::OtaStorageOpen),
    OtaHeaderLengthFail       = static_cast<State>(ota::OTAError::OtaHeaderLength),
    OtaHeaderCrcFail          = static_cast<State>(ota::OTAError::OtaHeaderCrc),
    OtaHeaterMagicNumberFail  = static_cast<State>(ota::OTAError::OtaHeaterMagicNumber),
    ParseHttpHeaderFail       = static_cast<State>(ota::OTAError::ParseHttpHeader),
    UrlParseErrorFail         = static_cast<State>(ota::OTAError::UrlParseError),
    ServerConnectErrorFail    = static_cast<State>(ota::OTAError::ServerConnectError),
    HttpHeaderErrorFail       = static_cast<State>(ota::OTAError::HttpHeaderError),
    OtaDownloadFail           = static_cast<State>(ota::OTAError::OtaDownload),
    OtaHeaderTimeoutFail      = static_cast<State>(ota::OTAError::OtaHeaderTimeout),
    HttpResponseFail          = static_cast<State>(ota::OTAError::HttpResponse),
    OtaStorageEndFail         = static_cast<State>(ota::OTAError::OtaStorageEnd),
    StorageConfigFail         = static_cast<State>(ota::OTAError::StorageConfig),
    LibraryFail               = static_cast<State>(ota::OTAError::Library),
    ModemFail                 = static_cast<State>(ota::OTAError::Modem),
    ErrorOpenUpdateFileFail   = static_cast<State>(ota::OTAError::ErrorOpenUpdateFile), // FIXME fopen
    ErrorWriteUpdateFileFail  = static_cast<State>(ota::OTAError::ErrorWriteUpdateFile), // FIXME fwrite
    ErrorReformatFail         = static_cast<State>(ota::OTAError::ErrorReformat),
    ErrorUnmountFail          = static_cast<State>(ota::OTAError::ErrorUnmount),
    ErrorRenameFail           = static_cast<State>(ota::OTAError::ErrorRename),
    CaStorageInitFail         = static_cast<State>(ota::OTAError::CaStorageInit),
    CaStorageOpenFail         = static_cast<State>(ota::OTAError::CaStorageOpen),
  };

#ifdef DEBUG_VERBOSE
  static const char* const STATE_NAMES[];
#endif // DEBUG_VERBOSE

  enum OtaFlags: uint16_t {
    None              = 0,
    ApprovalRequired  = 1,
    Approved          = 1<<1
  };

  virtual void handleMessage(Message*);
  // virtual CloudProcess::State getState();
  // virtual void hook(State s, void* action);
  virtual void update() { handleMessage(nullptr); }

  inline void approveOta()                      { policies |= Approved; }
  inline void setOtaPolicies(uint16_t policies) { this->policies = policies; }

  inline State getState() { return state; }

  virtual bool isOtaCapable() = 0;
protected:
  // The following methods represent the FSM actions performed in each state

  // the first state is 'resume', where we try to understand if we are booting after a ota
  // the objective is to understand the result and report it to the cloud
  virtual State resume(Message* msg=nullptr) = 0;

  // This state is used to send to the cloud the initial information on the fw present on the mcu
  // this state will only send the sha256 of the board fw and go into Idle state
  virtual State otaBegin();

  // this state is the normal state where no action has to be performed. We may poll the cloud
  // for updates in this state
  virtual State idle(Message* msg=nullptr);

  // we go in this state if there is an ota available, depending on the policies implemented we may
  // start the ota or wait for an user interaction
  virtual State otaAvailable();

  // we start the process of ota update and wait for the server to respond with the ota url and other info
  virtual State startOTA() = 0;

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

  uint16_t policies; // TODO getter and setters for this

  inline void updateState(State s) {
    if(state!=s) {
      DEBUG_VERBOSE("OTAInterface: state change to %s from %s",
        STATE_NAMES[s < 0? Fail - s : s],
        STATE_NAMES[state < 0? Fail - state : state]);
      previous_state = state; state = s;
    }
  }

  // This method is called to report the current state of the OtaClass
  void reportStatus();

  // in order to calculate the SHA256 we need to get the start and end address of the Application,
  // The Implementation of this class have to implement them.
  // The calculation is performed during the otaBegin phase
  virtual void* appStartAddress() = 0;
  virtual uint32_t appSize()      = 0;

  // some architecture require to explicitly open the flash in order to read it
  virtual bool appFlashOpen() = 0;
  virtual bool appFlashClose() = 0;

  // sha256 is going to be used in the ota process for validation, avoid calculating it twice
  uint8_t sha256[SHA256::HASH_SIZE];

  // calculateSHA256 method is overridable for platforms that do not support access through pointer to program memory
  virtual void calculateSHA256(SHA256&); // FIXME return error
private:
  void clean();

  State state, previous_state;

protected:
  struct OtaContext {
    OtaContext(
      uint8_t id[ID_SIZE], const char* url,
      uint8_t initialSha256[32], uint8_t finalSha256[32]);
    ~OtaContext();

    // parameters present in the ota available message that are of interest of the process
    uint8_t     id[ID_SIZE];
    char* const url;
    uint8_t     initialSha256[32];
    uint8_t     finalSha256[32];
    uint16_t    report_couter;
  } *context;
};

#endif // OTA_ENABLED
