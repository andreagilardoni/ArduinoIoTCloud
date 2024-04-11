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

#include <AIoTC_Config.h>

#if OTA_ENABLED
#include "OTAInterface.h"
#include "../OTA.h"

extern "C" unsigned long getTime();

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

#ifdef DEBUG_VERBOSE
const char* const OTACloudProcessInterface::STATE_NAMES[] = { // used only for debug purposes
  "Resume",
  "OtaBegin",
  "Idle",
  "OtaAvailable",
  "StartOTA",
  "Fetch",
  "FlashOTA",
  "Reboot",
  "Fail",
  "NoCapableBootloaderFail",
  "NoOtaStorageFail",
  "OtaStorageInitFail",
  "OtaStorageOpenFail",
  "OtaHeaderLengthFail",
  "OtaHeaderCrcFail",
  "OtaHeaterMagicNumberFail",
  "ParseHttpHeaderFail",
  "UrlParseErrorFail",
  "ServerConnectErrorFail",
  "HttpHeaderErrorFail",
  "OtaDownloadFail",
  "OtaHeaderTimeoutFail",
  "HttpResponseFail",
  "OtaStorageEndFail",
  "StorageConfigFail",
  "LibraryFail",
  "ModemFail",
  "ErrorOpenUpdateFileFail",
  "ErrorWriteUpdateFileFail",
  "ErrorReformatFail",
  "ErrorUnmountFail",
  "ErrorRenameFail",
};
#endif // DEBUG_VERBOSE

OTACloudProcessInterface::OTACloudProcessInterface(MessageStream *ms)
: CloudProcess(ms)
, policies(None)
, state(Resume)
, previous_state(Resume)
, context(nullptr) {
}

OTACloudProcessInterface::~OTACloudProcessInterface() {
  clean();
}

void OTACloudProcessInterface::handleMessage(Message* msg) {

  if ((state >= OtaAvailable || state < 0) && previous_state != state) {
    reportStatus();
  }

  // this allows to do status report only when the state changes
  previous_state = state;

  switch(state) {
  case Resume:        updateState(resume(msg));    break;
  case OtaBegin:      updateState(otaBegin());     break;
  case Idle:          updateState(idle(msg));      break;
  case OtaAvailable:  updateState(otaAvailable()); break;
  case StartOTA:      updateState(startOTA());     break;
  case Fetch:         updateState(fetch());        break;
  case FlashOTA:      updateState(flashOTA());     break;
  case Reboot:        updateState(reboot());       break;
  default:            updateState(fail()); // all the states that are not defined are failures
  }
}

OTACloudProcessInterface::State OTACloudProcessInterface::otaBegin() {
  if(!isOtaCapable()) {
    // FIXME What do we have to do in this case?
    DEBUG_WARNING("OTA is not available on this board");
    return OtaBegin;
  }

  struct OtaBeginUp msg = {
    OtaBeginUpId,
    {}
  };

  SHA256 sha256_calc;
  calculateSHA256(sha256_calc);

  sha256_calc.finalize(sha256);
  memcpy(msg.params.sha, sha256, SHA256::HASH_SIZE);

  DEBUG_VERBOSE("calculated SHA256: 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
    this->sha256[0], this->sha256[1], this->sha256[2], this->sha256[3], this->sha256[4], this->sha256[5], this->sha256[6], this->sha256[7], this->sha256[8], this->sha256[9], this->sha256[10], this->sha256[11], this->sha256[12], this->sha256[13], this->sha256[14], this->sha256[15], this->sha256[16], this->sha256[17], this->sha256[18], this->sha256[19], this->sha256[20], this->sha256[21], this->sha256[22], this->sha256[23], this->sha256[24], this->sha256[25], this->sha256[26], this->sha256[27], this->sha256[28], this->sha256[29], this->sha256[30], this->sha256[31]
    );

  deliver((Message*)&msg);
  // TODO msg object do not exist after this call make sure it gets copied somewhere

  return Idle;
}

void OTACloudProcessInterface::calculateSHA256(SHA256& sha256_calc) {
  DEBUG_VERBOSE("calculate sha on: 0x%X len 0x%X  %d", appStartAddress(), appSize(), appSize());
  auto res = appFlashOpen();
  if(!res) {
    // TODO return error
    return;
  }

  sha256_calc.begin();
  sha256_calc.update(
    reinterpret_cast<const uint8_t*>(appStartAddress()),
    appSize());
  appFlashClose();
}

OTACloudProcessInterface::State OTACloudProcessInterface::idle(Message* msg) {
  // if a msg arrived, it may be an OTAavailable, then go to otaAvailable
  // otherwise do nothing
  if(msg!=nullptr && msg->id == OtaUpdateCmdDownId) {
    // save info coming from this message
    assert(context == nullptr); // This should never fail

    struct OtaUpdateCmdDown* ota_msg = (struct OtaUpdateCmdDown*)msg;

    context = new OtaContext(
      ota_msg->params.id, ota_msg->params.url,
      ota_msg->params.initialSha256, ota_msg->params.finalSha256
      );

    // TODO verify that initialSha256 is the sha256 on board
    // TODO verify that final sha is not the current sha256 (?)
    return OtaAvailable;
  }

  return Idle;
}

OTACloudProcessInterface::State OTACloudProcessInterface::otaAvailable() {
  // depending on the policy decided on this device the ota process can start immediately
  // or wait for confirmation from the user
  if((policies & (ApprovalRequired | Approved)) == ApprovalRequired ) {
    return OtaAvailable;
  } else {
    policies &= ~Approved;
    return StartOTA;
  } // TODO add an abortOTA command? in this case delete the context
}

OTACloudProcessInterface::State OTACloudProcessInterface::fail() {
  reset();
  clean();

  return Idle;
}

void OTACloudProcessInterface::clean() {
  // free the context pointer
  if(context != nullptr) {
    delete context;
    context = nullptr;
  }
}

void OTACloudProcessInterface::reportStatus() {
  if(context == nullptr) {
    // FIXME handle this case: ota not in progress
    return;
  }

  struct OtaProgressCmdUp msg = {
    OtaProgressCmdUpId,
    {}
  };

  memcpy(msg.params.id, context->id, ID_SIZE);
  msg.params.state      = state>=0 ? state : State::Fail;
  msg.params.time       = getTime()*1e6 + context->report_couter++;
  msg.params.state_data = static_cast<int32_t>(state<0? state : 0);

  deliver((Message*)&msg);
  // TODO msg object do not exist after this call make sure it gets copied somewhere
}

OTACloudProcessInterface::OtaContext::OtaContext(
    uint8_t id[ID_SIZE], const char* url,
    uint8_t* initialSha256, uint8_t* finalSha256
    ) : url((char*) malloc(strlen(url) + 1))
    , report_couter(0) {

  memcpy(this->id, id, ID_SIZE);
  strcpy(this->url, url);
  memcpy(this->initialSha256, initialSha256, 32);
  memcpy(this->finalSha256, finalSha256, 32);
}

OTACloudProcessInterface::OtaContext::~OtaContext() {
  free(url);
}

#endif /* OTA_ENABLED */
