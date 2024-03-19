/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "AIoTC_Config.h"
#if defined(BOARD_STM32H7) && OTA_ENABLED
// #include "OTAPortentaH7.h"
#include "utility/watchdog/Watchdog.h"
#include <STM32H747_System.h>

static bool findProgramLength(DIR * dir, uint32_t & program_length);

// template<portenta::StorageType storage, uint32_t data_offset>
static const char UPDATE_FILE_NAME[] = "/fs/UPDATE.BIN";

template<portenta::StorageType storage, uint32_t data_offset>
STM32H7OTACloudProcess<storage, data_offset>::STM32H7OTACloudProcess(MessageStream *ms, Client* client)
: OTACloudProcessInterface(ms, client)
, decompressed(nullptr)
, _bd_raw_qspi(nullptr)
, _program_length(0)
, _bd(nullptr)
#ifdef ARDUINO_PORTENTA_OTA_SDMMC_SUPPORT
, _block_device(nullptr)
#endif // ARDUINO_PORTENTA_OTA_SDMMC_SUPPORT
, _fs(nullptr)
, cert_bd_qspi(nullptr)
, cert_fs_qspi(nullptr) {

}

template<portenta::StorageType storage, uint32_t data_offset>
STM32H7OTACloudProcess<storage, data_offset>::~STM32H7OTACloudProcess() {
  if(decompressed != nullptr) {
    fclose(decompressed);
    decompressed = nullptr;
  }

  storageClean();
}

template<portenta::StorageType storage, uint32_t data_offset>
OTACloudProcessInterface::State STM32H7OTACloudProcess<storage, data_offset>::resume(Message* msg) {
  return OtaBegin;
}

template<portenta::StorageType storage, uint32_t data_offset>
void STM32H7OTACloudProcess<storage, data_offset>::update() {
  OTACloudProcessInterface::update();
  watchdog_reset(); // FIXME this should npot be performed here
}

template<portenta::StorageType storage, uint32_t data_offset>
int STM32H7OTACloudProcess<storage, data_offset>::writeFlash(uint8_t* const buffer, size_t len) {
  if (decompressed == nullptr) {
    return -1;
  }
  return fwrite(buffer, sizeof(uint8_t), len, decompressed);
}

template<portenta::StorageType storage, uint32_t data_offset>
OTACloudProcessInterface::State STM32H7OTACloudProcess<storage, data_offset>::startOTA() {
  if (!isOtaCapable()) {
    return NoCapableBootloaderFail;
  }

  if (!caStorageInit()) {
    return CaStorageInitFail;
  }

  if (!caStorageOpen()) {
    return CaStorageOpenFail;
  }

  /* Initialize the QSPI memory for OTA handling. */
  if (!storageInit()) {
    return OtaStorageInitFail;
  }

  // this could be useless, since we are writing over it
  remove(UPDATE_FILE_NAME);
  remove("/fs/UPDATE.BIN.LZSS");

  decompressed = fopen(UPDATE_FILE_NAME, "wb");

  // start the download if the setup for ota storage is successful
  return OTACloudProcessInterface::startOTA();
}


template<portenta::StorageType storage, uint32_t data_offset>
OTACloudProcessInterface::State STM32H7OTACloudProcess<storage, data_offset>::flashOTA() {
  fclose(decompressed);
  decompressed = nullptr;

  /* Schedule the firmware update. */
  if(!storageOpen()) {
    return OtaStorageOpenFail;
  }

  // this sets the registries in RTC to load the firmware from the storage selected at the next reboot
  STM32H747::writeBackupRegister(RTCBackup::DR0, 0x07AA);
  STM32H747::writeBackupRegister(RTCBackup::DR1, storage);
  STM32H747::writeBackupRegister(RTCBackup::DR2, data_offset);
  STM32H747::writeBackupRegister(RTCBackup::DR3, _program_length);

  return Reboot;
}

template<portenta::StorageType storage, uint32_t data_offset>
OTACloudProcessInterface::State STM32H7OTACloudProcess<storage, data_offset>::reboot() {
  // TODO save information about the progress reached in the ota

  // This command reboots the mcu
  NVIC_SystemReset();

  return Resume; // This won't ever be reached
}

template<portenta::StorageType storage, uint32_t data_offset>
void STM32H7OTACloudProcess<storage, data_offset>::reset() {

  remove(UPDATE_FILE_NAME);

  storageClean();
}

template<portenta::StorageType storage, uint32_t data_offset>
void STM32H7OTACloudProcess<storage, data_offset>::storageClean() {
  DEBUG_VERBOSE(F("storage clean"));

  if(decompressed != nullptr) {
    fclose(decompressed);
    decompressed = nullptr;
  }

  // TODO close storage
  if(cert_bd_qspi != nullptr) {
    delete cert_bd_qspi;
    cert_bd_qspi = nullptr;
  }
  if(cert_fs_qspi != nullptr) {
    delete cert_fs_qspi;
    cert_fs_qspi = nullptr;
  }

  if(_fs != nullptr) {
    _fs->unmount();
    delete _fs;
    _fs = nullptr;
  }

  if(_bd != nullptr) {
    delete _bd;
    _bd = nullptr;
  }

#ifdef ARDUINO_PORTENTA_OTA_SDMMC_SUPPORT
  if(_block_device != nullptr) {
    delete _block_device;
    _block_device = nullptr;
  }
#endif // ARDUINO_PORTENTA_OTA_SDMMC_SUPPORT
}

template<portenta::StorageType storage, uint32_t data_offset>
bool STM32H7OTACloudProcess<storage, data_offset>::isOtaCapable() {
  #define BOOTLOADER_ADDR   (0x8000000)
  uint32_t bootloader_data_offset = 0x1F000;
  uint8_t* bootloader_data = (uint8_t*)(BOOTLOADER_ADDR + bootloader_data_offset);
  uint8_t currentBootloaderVersion = bootloader_data[1];
  if (currentBootloaderVersion < 22)
    return false;
  else
    return true;
}

template<portenta::StorageType storage, uint32_t data_offset>
bool STM32H7OTACloudProcess<storage, data_offset>::caStorageInit() {
  _bd_raw_qspi = mbed::BlockDevice::get_default_instance();

  if (_bd_raw_qspi->init() != QSPIF_BD_ERROR_OK) {
    DEBUG_ERROR(F("Error: QSPI init failure."));
    return false;
  }

  cert_bd_qspi = new mbed::MBRBlockDevice(_bd_raw_qspi, 1);
  cert_fs_qspi = new mbed::FATFileSystem("wlan");
  int const err_mount =  cert_fs_qspi->mount(cert_bd_qspi);
  if (err_mount) {
    DEBUG_ERROR(F("Error while mounting the certificate filesystem. Err = %d"), err_mount);
    return false;
  }
  return true;
}

template<portenta::StorageType storage, uint32_t data_offset>
bool STM32H7OTACloudProcess<storage, data_offset>::caStorageOpen() {
  FILE* fp = fopen("/wlan/cacert.pem", "r");
  if (!fp) {
    DEBUG_ERROR(F("Error while opening the certificate file."));
    return false;
  }
  fclose(fp);

  return true;
}

template<portenta::StorageType storage, uint32_t data_offset>
bool STM32H7OTACloudProcess<storage, data_offset>::storageInit() {
  int err_mount=1;

  if constexpr (storage == portenta::QSPI_FLASH_FATFS) {
    _fs = new mbed::FATFileSystem("fs");
    err_mount = _fs->mount(_bd_raw_qspi);

  } else if constexpr (storage == portenta::QSPI_FLASH_FATFS_MBR) {
    _bd = new mbed::MBRBlockDevice(_bd_raw_qspi, data_offset);
    _fs = new mbed::FATFileSystem("fs");
    err_mount = _fs->mount(_bd);

#ifdef ARDUINO_PORTENTA_OTA_SDMMC_SUPPORT
  } else if constexpr (storage == portenta::SD_FATFS) {
    _block_device = new SDMMCBlockDevice();
    _fs = new mbed::FATFileSystem("fs");
    err_mount =  _fs->mount(_block_device);

  } else if constexpr (storage == portenta::SD_FATFS_MBR) {
    _block_device = new SDMMCBlockDevice();
    _bd = new mbed::MBRBlockDevice(reinterpret_cast<mbed::BlockDevice *>(_block_device), data_offset);
    _fs = new mbed::FATFileSystem("fs");
    err_mount =  _fs->mount(_bd);
#endif // ARDUINO_PORTENTA_OTA_SDMMC_SUPPORT
  }

  if (!err_mount) {
    return true;
  }
  DEBUG_ERROR(F("Error while mounting the filesystem. Err = %d"), err_mount);
  return false;
}

template<portenta::StorageType storage, uint32_t data_offset>
bool STM32H7OTACloudProcess<storage, data_offset>::storageOpen() {
  DIR * dir = NULL;
  if ((dir = opendir("/fs")) != NULL)
  {
    if (findProgramLength(dir, _program_length))
    {
      closedir(dir);
      return true;
    }
    closedir(dir);
  }

  return false;
}

bool findProgramLength(DIR * dir, uint32_t & program_length) {
  struct dirent * entry = NULL;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, "UPDATE.BIN") == 0) { // FIXME use constants
      struct stat stat_buf;
      stat("/fs/UPDATE.BIN", &stat_buf);
      program_length = stat_buf.st_size;
      return true;
    }
  }

  return false;
}

// extern uint32_t __stext = ~0;
extern uint32_t __etext;
extern uint32_t _sdata;
extern uint32_t _edata;

template<portenta::StorageType storage, uint32_t data_offset>
void* STM32H7OTACloudProcess<storage, data_offset>::appStartAddress() {
  return (void*)0x8040000;
  // return &__stext;
}

template<portenta::StorageType storage, uint32_t data_offset>
uint32_t STM32H7OTACloudProcess<storage, data_offset>::appSize() {
  return ((&__etext - (uint32_t*)appStartAddress()) + (&_edata - &_sdata))*sizeof(void*);
}


#endif // defined(BOARD_STM32H7) && OTA_ENABLED