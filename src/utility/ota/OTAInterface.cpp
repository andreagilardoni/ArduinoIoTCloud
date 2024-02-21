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

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

// #include <AIoTC_Config.h>

#if OTA_ENABLED

#include "OTA.h"
// #include <Arduino_DebugUtils.h>

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

OTACloudProcessInterface::STATE_NAMES = {
  "Resume",
  "Idle",
  "OtaAvailable",
  "StartOTA",
  "Fetch",
  "VerifyOTA",
  "FlashOTA",
  "Reboot",
  "DownloadFail",
  "CRCValidationFail",
  "Fail"
};

OTACloudProcessInterface::OTACloudProcessInterface(Client* client)
: context(nullptr), state(Resume), previous_state(Resume), client(client) {

}

void OTACloudProcessInterface::handleMessage(Message* msg) {

  // we can put conditions that states must follow here, or go into failed state
  // we can receive an abort OTA message here and return to idle
  // if(msg != nullptr && state == Fetch) { // We cannot receive Messages while in Fetch state
  // }

  switch(state) {
  case Resume:        updateState(resume(msg));    break;
  case Idle:          updateState(idle(msg));      break;
  case OtaAvailable:  updateState(otaAvailable()); break;
  case StartOTA:      updateState(startOTA());     break;
  case Fetch:         updateState(fetch());        break;
  case FlashOTA:      updateState(flashOTA());     break;
  case Reboot:        updateState(reboot());       break;
  default:            updateState(fail()); // all the states that are not defined are failures
  }
  // TODO verify that the ota was performed successfully

  if(!OTA_SUCCESS) {
    // TODO report some info about the failed ota

    return Fail;
  }

  // TODO report that the ota performed was successful
  return Idle
}

State OTACloudProcessInterface::idle(Message* msg) {
  // if a msg arrived, it may be an OTAavailable, then go to otaAvailable
  // otherwise do nothing
  if(msg!=nullptr && msg.commandId == OTAavailable) {
    // save info coming from this message
    assert(context == nullptr); // This should never fail
    context = (ota_context_t) malloc(sizeof(ota_context_t)); // TODO deallocate the struct, and decide where to do that

    context->id            = (const char*)     malloc(strlen(msg.id) + 1);
    context->url           = (const char*)     malloc(strlen(msg.url) + 1);
    context->initialSha256 = (const uint8_t*)  malloc(32); // 32 is the fixed size for a sha256
    context->finalSha256   = (const uint8_t*)  malloc(32);

    strcpy(context->id, msg.id);
    strcpy(context->url, msg.url);
    memcpy(context->initialSha256, msg.initialSha256, 32);
    memcpy(context->finalSha256, msg.finalSha256, 32);

    context->calculatedCrc32     = 0xFFFFFFFF
    context->state               = OtaDownloadHeader
    context->headerCopiedBytes   = 0;

    return OtaAvailable;
  }

  return Idle;
}

State OTACloudProcessInterface::otaAvailable() {
  // depending on the policy decided on this device the ota process can start immediately
  // or wait for confirmation from the user
  if(policies & (ApprovalRequired | Approved) == ApprovalRequired ) {
    return OtaAvailable;
  } else {
    policies &= ~Confirmation;
    return StartOTA;
  } // TODO add an abortOTA command? in this case delete the context
}

State OTACloudProcessInterface::startOTA() {
  // Report that we are starting ota
  statusReport();

  assert(client != nullptr, "ERROR client wasn't set in OTACloudProcess");

  // parse the url
  // make the http get request
  client->connect(context->url);

  return Fetch;
}

State OTACloudProcessInterface::fetch() {
  size_t buf_len = 100; // TODO should this be constexpr?
  static uint8_t  buffer[buf_len];
  int res = client->read(buffer, len);

  assert(context != nullptr); // This should never fail

  // TODO move this to a dedicated funtion, so that we chan use it as a callback instead of using memcpy
  for(char* cursor=(char*)buffer; cursor<buffer+buf_len; ) {
    switch(context->downloadState) {
    case OtaDownloadHeader:

      uint32_t copied = size < sizeof(context->header.buf) ? size : sizeof(context->header.buf);
      memcpy(context->header.buf, buffer, copied);
      cursor += copied;
      context->headerCopiedBytes += copied;

      // when finished go to next state
      if(sizeof(context->header.buf) == context->headerCopiedBytes) {
        context->downloadState = OtaDownloadFile;

        context->calculatedCrc32 = crc_update(
          context->calculatedCrc32,
          &(context->header.header.magic_number),
          sizeof(context->header) - offsetof(OTAHeader, header.magic_number)
        );
      }

      break;
    case OtaDownloadFile:
      // TODO decompress on the fly

      context->calculatedCrc32 = crc_update(
          context->calculatedCrc32,
          cursor,
          buf_len - (cursor-buffer)
        );
      break;
    default:
      goto exit;
    }
  }

exit:
  // TODO verify that the information present in the ota header match the info in context
  // TODO verify the magic number
  if(context->downloadState == OtaDownloadCompleted) {
    // Verify that the downloaded file size is matching the expected size ??
    // this could distinguish between consistency of the downloaded bytes and filesize

    // validate CRC
    context->calculatedCrc32 ^= 0xFFFFFFFF;
    if(context->header.crc32 == context->calculatedCrc32) {
      client->stop(); // close the client, but do not deallocate it
      return FlashOTA;
    } else {
      return CRCValidationFail;
    }
  } else if(context->downloadState == OtaDownloadError) {
    return DownloadFail;
  }
}

State OTACloudProcessInterface::fail() {
  reportStatus();
  reset();

  // TODO clean the context pointer
}

void OTACloudProcessInterface::reportStatus() {
  if(context == nullptr) {
    // FIXME handle this case: ota not in progress
    return;
  }

  union OTAStatusReport msg = {
    OTAStatusReportId,
    context->id,
    "unknown", // FIXME put the proper expected value
    millis(),  // FIXME put the proper expected value
    context->report_couter++
  };

  deliver((Message*)&msg);
  // TODO msg object do not extist after this call make sure it gets copied somewhere
}

#endif /* OTA_ENABLED */
