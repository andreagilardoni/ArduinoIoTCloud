#include <AIoTC_Config.h>

#if defined(ARDUINO_ARCH_SAMD) && OTA_ENABLED
#include "OTASamd.h"

#include <Arduino_DebugUtils.h>
#if OTA_STORAGE_SNU
#  include <SNU.h>
#  include <WiFiNINA.h> /* WiFiStorage */
#endif

SAMDOTACloudProcess::SAMDOTACloudProcess(MessageStream *ms, ConnectionHandler* connection_handler)
: OTACloudProcessInterface(ms, connection_handler){

}

OTACloudProcessInterface::State SAMDOTACloudProcess::resume(Message* msg) {
  return OtaBegin;
}

OTACloudProcessInterface::State SAMDOTACloudProcess::startOTA() {
  reset();
  return Fetch;
}

OTACloudProcessInterface::State SAMDOTACloudProcess::fetch() {
#if OTA_STORAGE_SNU
  uint8_t nina_ota_err_code = 0;
  if (!WiFiStorage.downloadOTA(this->context->url, &nina_ota_err_code)) {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s error download to nina: %d", __FUNCTION__, nina_ota_err_code);
    return OtaDownloadFail;
  }
#endif // OTA_STORAGE_SNU

  return FlashOTA;
}

OTACloudProcessInterface::State SAMDOTACloudProcess::flashOTA() {
  return Reboot;
}

OTACloudProcessInterface::State SAMDOTACloudProcess::reboot() {
}

void SAMDOTACloudProcess::reset() {
#if OTA_STORAGE_SNU
  WiFiStorage.remove("/fs/UPDATE.BIN.LZSS");
  WiFiStorage.remove("/fs/UPDATE.BIN.LZSS.TMP");
#endif // OTA_STORAGE_SNU
}

bool SAMDOTACloudProcess::isOtaCapable() {
#if OTA_STORAGE_SNU
  if (String(WiFi.firmwareVersion()) < String("1.4.1")) {
    DEBUG_WARNING("ArduinoIoTCloudTCP::%s In order to be ready for cloud OTA, NINA firmware needs to be >= 1.4.1, current %s", __FUNCTION__, WiFi.firmwareVersion());
    return false;
  } else {
    return true;
  }
#endif
  return false;
}


#endif // defined(ARDUINO_ARCH_SAMD) && OTA_ENABLED