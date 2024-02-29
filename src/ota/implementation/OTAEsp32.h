/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "ota/interface/OTAInterface.h"

class ESP32OTACloudProcess: public OTACloudProcessInterface {
public:
  ESP32OTACloudProcess(MessageStream *ms, ConnectionHandler* connection_handler=nullptr);

  virtual bool isOtaCapable() override;
protected:
  virtual OTACloudProcessInterface::State resume(Message* msg=nullptr) override;

  // we are overriding the method of startOTA in order to download ota file on ESP32
  virtual OTACloudProcessInterface::State startOTA() override;

  // when the download is completed we verify for integrity and correctness of the downloaded binary
  // virtual State verifyOTA(Message* msg=nullptr); // TODO this may be performed inside download

  // whene the download is correctly finished we set the mcu to use the newly downloaded binary
  virtual State flashOTA();

  // we reboot the device
  virtual State reboot();

  void reset() override {};

  // write the decompressed char buffer of the incoming ota
  virtual int writeFlash(uint8_t* const buffer, size_t len) override;

  void* appStartAddress();
  uint32_t appSize();
  bool appFlashOpen();
  bool appFlashClose() { return true; };

  void calculateSHA256(SHA256&) override;
private:
  const esp_partition_t *rom_partition;
};
