#pragma once
#include "OTAInterface.h"
#include <Arduino_Portenta_OTA.h>

class STM32H7OTACloudProcess: public OTACloudProcessInterface {
public:
  STM32H7OTACloudProcess(MessageStream *ms, ConnectionHandler* connection_handler=nullptr);
  ~STM32H7OTACloudProcess();
  void update() override;

  virtual bool isOtaCapable() override;
protected:
  virtual OTACloudProcessInterface::State resume(Message* msg=nullptr) override;

  // we are overriding the method of startOTA in order to open the destination file for the ota download
  virtual OTACloudProcessInterface::State startOTA() override;

  // whene the download is correctly finished we set the mcu to use the newly downloaded binary
  virtual OTACloudProcessInterface::State flashOTA() override;

  // we reboot the device
  virtual OTACloudProcessInterface::State reboot() override;

  // write the decompressed char buffer of the incoming ota
  virtual int writeFlash(uint8_t* const buffer, size_t len) override;

  virtual void reset() override;
private:
  FILE* decompressed;
  Arduino_Portenta_OTA_QSPI ota_portenta_qspi;
  static const char UPDATE_FILE_NAME[];
};
