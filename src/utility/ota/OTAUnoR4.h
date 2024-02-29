#pragma once

#include "OTAInterface.h"
#include "OTAUpdate.h"
#include "r_flash_lp.h"

class UNOR4OTACloudProcess: public OTACloudProcessInterface {
public:
  UNOR4OTACloudProcess(MessageStream *ms, ConnectionHandler* connection_handler=nullptr);

  bool isOtaCapable() override;
protected:
  virtual OTACloudProcessInterface::State resume(Message* msg=nullptr) override;

  // we are overriding the method of startOTA in order to download ota file on ESP32
  virtual OTACloudProcessInterface::State startOTA() override;

  // we start the download and decompress process
  virtual OTACloudProcessInterface::State fetch() override;

  // whene the download is correctly finished we set the mcu to use the newly downloaded binary
  virtual OTACloudProcessInterface::State flashOTA();

  // we reboot the device
  virtual OTACloudProcessInterface::State reboot();

  // write the decompressed char buffer of the incoming ota
  virtual int writeFlash(uint8_t* const buffer, size_t len) override {};

  virtual void reset() override;

  constexpr void* appStartAddress();
  uint32_t appSize();

  bool appFlashOpen();
  bool appFlashClose();

public:
  // used to access to flash memory for sha256 calculation
  flash_lp_instance_ctrl_t  ctrl;
  flash_cfg_t               cfg;

  OTAUpdate ota;
  static const char UPDATE_FILE_NAME[];
};
