/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <Arduino_ConnectionHandler.h>

#if defined(ARDUINO_UNOR4_WIFI)
  #include <WiFiSSLClient.h>
#elif defined(BOARD_STM32H7)
  #ifdef BOARD_HAS_WIFI
  #include <WiFiSSLClient.h>
  #endif // BOARD_HAS_WIFI

  #ifdef BOARD_HAS_ETHERNET
  #include "EthernetSSLClient.h"
  #endif // BOARD_HAS_ETHERNET
#elif defined(ARDUINO_PORTENTA_C33)
  #include "EthernetSSLClient.h"
  #include <WiFiSSLClient.h>
#elif defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  #include <WiFiClientSecure.h>

  namespace connectionHandler {
    using WiFiSSLClient=WiFiClientSecure;
  }
#endif

namespace connectionHandler {
  inline Client* getNewSSLClient(NetworkAdapter net) {
    switch(net) {
#ifdef BOARD_HAS_WIFI
    case NetworkAdapter::WIFI:
      return new WiFiSSLClient();
#endif // BOARD_HAS_WIFI
#ifdef BOARD_HAS_ETHERNET
    case NetworkAdapter::ETHERNET:
      return new EthernetSSLClient();
#endif // BOARD_HAS_ETHERNET
#ifdef BOARD_HAS_NB
    case NetworkAdapter::NB:
      return new NBSSLClient();
#endif // BOARD_HAS_NB
#ifdef BOARD_HAS_GSM
    case NetworkAdapter::GSM:
      return new GSMSSLClient();
#endif // BOARD_HAS_GSM
#ifdef BOARD_HAS_CATM1_NBIOT
    case NetworkAdapter::CATM1:
      return new GSMSSLClient();
#endif // BOARD_HAS_CATM1_NBIOT
    default:
      return nullptr;
    }
  }
}
