/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <AIoTC_Config.h>

#if defined(ARDUINO_UNOR4_WIFI) && OTA_ENABLED
#include "OTAUnoR4.h"

#include <Arduino_DebugUtils.h>
#include "tls/utility/SHA256.h"
#include "fsp_common_api.h"
#include "r_flash_lp.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

const char UNOR4OTACloudProcess::UPDATE_FILE_NAME[] = "/update.bin";

UNOR4OTACloudProcess::UNOR4OTACloudProcess(MessageStream *ms, ConnectionHandler* connection_handler)
: OTACloudProcessInterface(ms, connection_handler){

}

OTACloudProcessInterface::State UNOR4OTACloudProcess::resume(Message* msg) {
  return OtaBegin;
}

OTACloudProcessInterface::State UNOR4OTACloudProcess::startOTA() {
  OTAUpdate::Error ota_err = OTAUpdate::Error::None;

  // Open fs for ota
  if((ota_err = ota.begin(UPDATE_FILE_NAME)) != OTAUpdate::Error::None) {
    DEBUG_ERROR("OTAUpdate::begin() failed with %d", static_cast<int>(ota_err));
    return OtaDownloadFail; // FIXME
  }

  return Fetch;
}

OTACloudProcessInterface::State UNOR4OTACloudProcess::fetch() {
  OTAUpdate::Error ota_err = OTAUpdate::Error::None;

  int const ota_download = ota.download(this->context->url,UPDATE_FILE_NAME);
  if (ota_download <= 0) {
    DEBUG_ERROR("OTAUpdate::download() failed with %d", ota_download);
    return OtaDownloadFail;
  }
  DEBUG_VERBOSE("OTAUpdate::download() %d bytes downloaded", static_cast<int>(ota_download));

  if ((ota_err = ota.verify()) != OTAUpdate::Error::None) {
    DEBUG_ERROR("OTAUpdate::verify() failed with %d", static_cast<int>(ota_err));
    return OtaHeaderCrcFail; // FIXME
  }

  return FlashOTA;
}

OTACloudProcessInterface::State UNOR4OTACloudProcess::flashOTA() {
  OTAUpdate::Error ota_err = OTAUpdate::Error::None;

  /* Flash new firmware */
  if ((ota_err = ota.update(UPDATE_FILE_NAME)) != OTAUpdate::Error::None) { // This reboots the MCU
    DEBUG_ERROR("OTAUpdate::update() failed with %d", static_cast<int>(ota_err));
    return OtaDownloadFail; // FIXME
  }
}

OTACloudProcessInterface::State UNOR4OTACloudProcess::reboot() {
}

void UNOR4OTACloudProcess::reset() {
}

bool UNOR4OTACloudProcess::isOtaCapable() {
  String const fv = WiFi.firmwareVersion();
  if (fv < String("0.3.0")) {
    return false;
  }
  return true;
}

extern void* __ROM_Start;
extern void* __etext;
extern void* __data_end__;
extern void* __data_start__;

constexpr void* UNOR4OTACloudProcess::appStartAddress() { return &__ROM_Start; }
uint32_t UNOR4OTACloudProcess::appSize() {
  return ((&__etext - &__ROM_Start) + (&__data_end__ - &__data_start__))*sizeof(void*);
}

bool UNOR4OTACloudProcess::appFlashOpen() {
  cfg.data_flash_bgo         = false;
  cfg.p_callback             = nullptr;
  cfg.p_context              = nullptr;
  cfg.p_extend               = nullptr;
  cfg.ipl                    = (BSP_IRQ_DISABLED);
  cfg.irq                    = FSP_INVALID_VECTOR;
  cfg.err_ipl                = (BSP_IRQ_DISABLED);
  cfg.err_irq                = FSP_INVALID_VECTOR;

  fsp_err_t rv = FSP_ERR_UNSUPPORTED;

  rv = R_FLASH_LP_Open(&ctrl,&cfg);
  DEBUG_WARNING("Flash open %X", rv);

  return rv == FSP_SUCCESS;
}

bool UNOR4OTACloudProcess::appFlashClose() {
  fsp_err_t rv = FSP_ERR_UNSUPPORTED;
  rv = R_FLASH_LP_Close(&ctrl);
  DEBUG_WARNING("Flash close %X", rv);

  return rv == FSP_SUCCESS;
}


#endif // defined(ARDUINO_UNOR4_WIFI) && OTA_ENABLED
