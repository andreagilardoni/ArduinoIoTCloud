#include "AIoTC_Config.h"
#if defined(ARDUINO_ARCH_ESP32) && OTA_ENABLED
#include "OTAEsp32.h"
#include <esp_ota_ops.h>

ESP32OTACloudProcess::ESP32OTACloudProcess(MessageStream *ms, ConnectionHandler* connection_handler)
: OTACloudProcessInterface(ms, connection_handler), rom_partition(nullptr) {

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

void* ESP32OTACloudProcess::appStartAddress() {
  return nullptr;
}
uint32_t ESP32OTACloudProcess::appSize() {
  return ESP.getSketchSize();
}

bool ESP32OTACloudProcess::appFlashOpen() {
  rom_partition = esp_ota_get_running_partition();

  if(rom_partition == nullptr) {
    return false;
  }

  return true;
}

void ESP32OTACloudProcess::calculateSHA256(SHA256& sha256_calc) {
  if(!appFlashOpen()) {
    return; // TODO error reporting
  }

  sha256_calc.begin();

  uint8_t b[SPI_FLASH_SEC_SIZE];
  if(b == nullptr) {
    DEBUG_ERROR("ESP32::SHA256 Not enough memory to allocate buffer");
    return; // TODO error reporting
  }

  uint32_t       read_bytes = 0;
  uint32_t const app_size   = ESP.getSketchSize();
  for(uint32_t a = rom_partition->address; read_bytes < app_size; ) {
    /* Check if we are reading last sector and compute used size */
    uint32_t const read_size = read_bytes + SPI_FLASH_SEC_SIZE < app_size ?
      SPI_FLASH_SEC_SIZE : app_size - read_bytes;

    /* Use always 4 bytes aligned reads */
    if (!ESP.flashRead(a, reinterpret_cast<uint32_t*>(b), (read_size + 3) & ~3)) {
      DEBUG_ERROR("ESP32::SHA256 Could not read data from flash");
      return;
    }
    sha256_calc.update(b, read_size);
    a += read_size;
    read_bytes += read_size;
  }

  appFlashClose();
}

#endif // defined(ARDUINO_ARCH_ESP32) && OTA_ENABLED