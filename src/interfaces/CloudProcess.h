/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_IOT_CLOUD_PROCESS
#define ARDUINO_IOT_CLOUD_PROCESS

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudProcess
{
  public:

    virtual void update   () = 0;
    virtual int  connected() = 0;
};

#endif /* ARDUINO_IOT_CLOUD_PROCESS */
