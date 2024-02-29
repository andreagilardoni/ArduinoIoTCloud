#pragma once

#include "OTAInterface.h"
#include <Arduino_ESP32_OTA.h>

class ESP32OTACloudProcess: public OTACloudProcessInterface {
public:
  ESP32OTACloudProcess(MessageStream *ms, ConnectionHandler* connection_handler=nullptr);

  virtual bool isOtaCapable() override;
protected:
  virtual OTACloudProcessInterface::State resume(Message* msg=nullptr) override;

  // we are overriding the method of startOTA in order to download ota file on ESP32
  virtual OTACloudProcessInterface::State startOTA() override;

  // we start the download and decompress process
  virtual State fetch();

  // when the download is completed we verify for integrity and correctness of the downloaded binary
  // virtual State verifyOTA(Message* msg=nullptr); // TODO this may be performed inside download

  // whene the download is correctly finished we set the mcu to use the newly downloaded binary
  virtual State flashOTA();

  // we reboot the device
  virtual State reboot();

  // write the decompressed char buffer of the incoming ota
  virtual int writeFlash(uint8_t* const buffer, size_t len) override {};

  virtual void reset() override;
// private:
public:
  Arduino_ESP32_OTA ota;
};
