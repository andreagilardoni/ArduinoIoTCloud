#pragma once

#include "OTAInterface.h"
#include <Arduino_DebugUtils.h>


class SAMDOTACloudProcess: public OTACloudProcessInterface {
public:
  SAMDOTACloudProcess(MessageStream *ms, ConnectionHandler* connection_handler=nullptr);

  virtual bool isOtaCapable() override;
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

};
