#include "AIoTC_Config.h"
#if defined(BOARD_STM32H7) && OTA_ENABLED
#include "OTAPortentaH7.h"
#include "utility/watchdog/Watchdog.h"
#include <stm32h7xx_hal_rtc_ex.h>

const char STM32H7OTACloudProcess::UPDATE_FILE_NAME[] = "/fs/UPDATE.BIN";

STM32H7OTACloudProcess::STM32H7OTACloudProcess(MessageStream *ms, ConnectionHandler* connection_handler)
: OTACloudProcessInterface(ms, connection_handler)
, decompressed(nullptr)
, ota_portenta_qspi(QSPI_FLASH_FATFS_MBR, 2) {

}

STM32H7OTACloudProcess::~STM32H7OTACloudProcess() {
  if(decompressed != nullptr) {
    fclose(decompressed);
    decompressed = nullptr;
  }
}

OTACloudProcessInterface::State STM32H7OTACloudProcess::resume(Message* msg) {
  return OtaBegin;
}

void STM32H7OTACloudProcess::update() {
  OTACloudProcessInterface::update();
  watchdog_reset();
}

int STM32H7OTACloudProcess::writeFlash(uint8_t* const buffer, size_t len) {
  if (decompressed == nullptr) {
    return -1;
  }
  return fwrite(buffer, sizeof(uint8_t), len, decompressed);
}

OTACloudProcessInterface::State STM32H7OTACloudProcess::startOTA() {
  // OTACloudProcessInterface::State new_state = OTACloudProcessInterface::startOTA();

  // if(new_state >= 0) {
    Arduino_Portenta_OTA::Error ota_portenta_err = Arduino_Portenta_OTA::Error::None;

#if defined (ARDUINO_PORTENTA_OTA_HAS_WATCHDOG_FEED)
    // ota_portenta_qspi.setFeedWatchdogFunc(watchdog_reset); // FIXME understand how to integrate this, it breaks stuff
#endif // defined (ARDUINO_PORTENTA_OTA_HAS_WATCHDOG_FEED)

    watchdog_reset();

    /* Initialize the QSPI memory for OTA handling. */
    if((ota_portenta_err = ota_portenta_qspi.begin()) != Arduino_Portenta_OTA::Error::None) {
      DEBUG_ERROR("Arduino_Portenta_OTA_QSPI::begin() failed with %d", static_cast<int>(ota_portenta_err));

      return OtaStorageOpenFail; // FIXME is this the correct value?
    }
    remove("/fs/UPDATE.BIN");
    remove("/fs/UPDATE.BIN.LZSS");

    decompressed = fopen(STM32H7OTACloudProcess::UPDATE_FILE_NAME, "wb");
  // }

  // return new_state;
  // start the download if the setup for ota storage is successfull
  return OTACloudProcessInterface::startOTA();
}


OTACloudProcessInterface::State STM32H7OTACloudProcess::flashOTA() {
  fclose(decompressed);
  decompressed = nullptr;

  reportStatus();
  Arduino_Portenta_OTA::Error ota_portenta_err = Arduino_Portenta_OTA::Error::None;

#if defined (ARDUINO_PORTENTA_OTA_HAS_WATCHDOG_FEED)
  // ota_portenta_qspi.setFeedWatchdogFunc(watchdog_reset);
#endif

  /* Schedule the firmware update. */
  if((ota_portenta_err = ota_portenta_qspi.update()) != Arduino_Portenta_OTA::Error::None) {
    DEBUG_ERROR("Arduino_Portenta_OTA_QSPI::update() failed with %d", static_cast<int>(ota_portenta_err));
    return OtaDownloadFail; // FIXME is this the correct value?
  }

  return Reboot;
}

OTACloudProcessInterface::State STM32H7OTACloudProcess::reboot() {
  reportStatus();
  // TODO save information about the progress reached in the ota

  // This command reboots the mcu
  ota_portenta_qspi.reset();

  return Resume; // This won't ever be reached
}

void STM32H7OTACloudProcess::reset() {
  remove(STM32H7OTACloudProcess::UPDATE_FILE_NAME);
}

bool STM32H7OTACloudProcess::isOtaCapable() {
  return Arduino_Portenta_OTA::isOtaCapable();
}


#endif // defined(BOARD_STM32H7) && OTA_ENABLED