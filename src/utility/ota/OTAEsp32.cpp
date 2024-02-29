#include "AIoTC_Config.h"
#if defined(ARDUINO_ARCH_ESP32) && OTA_ENABLED
#include "OTAEsp32.h"

ESP32OTACloudProcess::ESP32OTACloudProcess(MessageStream *ms, ConnectionHandler* connection_handler)
: OTACloudProcessInterface(ms, connection_handler) {

}


OTACloudProcessInterface::State ESP32OTACloudProcess::resume(Message* msg) {
  return OtaBegin;
}

OTACloudProcessInterface::State ESP32OTACloudProcess::startOTA() {
  Arduino_ESP32_OTA::Error ota_err = Arduino_ESP32_OTA::Error::None;

  if ((ota_err = ota.begin()) != Arduino_ESP32_OTA::Error::None) {
    DEBUG_ERROR("Arduino_ESP32_OTA::begin() failed with %d", static_cast<int>(ota_err));
    return OtaDownloadFail; // FIXME
  }

  return Fetch;
}

OTACloudProcessInterface::State ESP32OTACloudProcess::fetch() {
  Arduino_ESP32_OTA::Error ota_err = Arduino_ESP32_OTA::Error::None;

  /* Download the OTA file from the web storage location. */
  int const ota_download = ota.download(this->context->url);
  if (ota_download <= 0) {
    DEBUG_ERROR("Arduino_ESP_OTA::download() failed with %d", ota_download);
    // return ota_download; // FIXME
    return OtaDownloadFail;
  }
  DEBUG_VERBOSE("Arduino_ESP_OTA::download() %d bytes downloaded", static_cast<int>(ota_download));

  return FlashOTA;
}

OTACloudProcessInterface::State ESP32OTACloudProcess::flashOTA() {
  Arduino_ESP32_OTA::Error ota_err = Arduino_ESP32_OTA::Error::None;

  if ((ota_err = ota.update()) != Arduino_ESP32_OTA::Error::None) {
    DEBUG_ERROR("Arduino_ESP_OTA::update() failed with %d", static_cast<int>(ota_err));
    return OtaDownloadFail; // FIXME
  }

  return Reboot;
}

OTACloudProcessInterface::State ESP32OTACloudProcess::reboot() {
  ota.reset();

  return Idle; // we won't reach this
}

void ESP32OTACloudProcess::reset() {
}

bool ESP32OTACloudProcess::isOtaCapable() {
  return Arduino_ESP32_OTA::isCapable();
}

#endif // defined(ARDUINO_ARCH_ESP32) && OTA_ENABLED