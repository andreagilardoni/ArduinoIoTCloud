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
 * TYPEDEF
 ******************************************************************************/

enum class ArduinoIoTCloudProcessEvent
{
  /* Device Events */
  SendCapabilities,
  GetThingId,
  ThingId,
  AttachThing,

  /* Thing Events */
  RequestlastValues = 256,
  LastValues,
  SendProperties,

  /* Ota Events */
  OtaUrl = 512,
  OtaReq,
  OtaConfirm,
  OtaStart,
  OtaError,

  /* Generic Events */
  Disconnect
};

typedef void (*deliverCallbackFunc)(ArduinoIoTCloudProcessEvent);

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudProcess
{
  public:

    virtual void begin(deliverCallbackFunc cb) = 0;
    virtual void update() = 0;
    virtual int  connected() = 0;
    virtual void handleMessage(ArduinoIoTCloudProcessEvent ev, char* msg) = 0;

  protected:

    deliverCallbackFunc _deliver;

};

#endif /* ARDUINO_IOT_CLOUD_PROCESS */
