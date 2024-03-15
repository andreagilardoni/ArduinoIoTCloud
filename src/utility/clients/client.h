/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <Arduino_ConnectionHandler.h>

namespace connectionHandler {
  inline Client* getNewClient(NetworkAdapter net) {
    switch(net) {
#ifdef BOARD_HAS_WIFI
    case NetworkAdapter::WIFI:
      return new WiFiClient();
#endif // BOARD_HAS_WIFI
#ifdef BOARD_HAS_ETHERNET
    case NetworkAdapter::ETHERNET:
      return new EthernetClient();
#endif // BOARD_HAS_ETHERNET
#ifdef BOARD_HAS_NB
    case NetworkAdapter::NB:
      return new NBClient();
#endif // BOARD_HAS_NB
#ifdef BOARD_HAS_GSM
    case NetworkAdapter::GSM:
      return new GSMClient();
#endif // BOARD_HAS_GSM
#ifdef BOARD_HAS_CATM1_NBIOT
    case NetworkAdapter::CATM1:
      return new GSMClient();
#endif // BOARD_HAS_CATM1_NBIOT
    default:
      return nullptr;
    }
  }
}
