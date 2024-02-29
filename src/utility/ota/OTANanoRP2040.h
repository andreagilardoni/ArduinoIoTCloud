#pragma once

#include "OTAInterface.h"

#include "FATFileSystem.h"
#include "FlashIAPBlockDevice.h"

class NANO_RP2040OTACloudProcess: public OTACloudProcessInterface {
public:
  NANO_RP2040OTACloudProcess(MessageStream *ms, ConnectionHandler* connection_handler=nullptr);
  ~NANO_RP2040OTACloudProcess();

  virtual bool isOtaCapable() override;
protected:
  virtual OTACloudProcessInterface::State resume(Message* msg=nullptr) override;

  virtual OTACloudProcessInterface::State startOTA() override;

  // whene the download is correctly finished we set the mcu to use the newly downloaded binary
  virtual OTACloudProcessInterface::State flashOTA() override;

  // we reboot the device
  virtual OTACloudProcessInterface::State reboot() override;

  // write the decompressed char buffer of the incoming ota
  virtual int writeFlash(uint8_t* const buffer, size_t len) override;

  virtual void reset() override;
private:
  FlashIAPBlockDevice flash;
  FILE* decompressed;
  mbed::FATFileSystem* fs;
  static const char UPDATE_FILE_NAME[];

  int close_fs();
};
