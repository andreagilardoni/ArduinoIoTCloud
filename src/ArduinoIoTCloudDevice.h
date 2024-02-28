/*
   This file is part of ArduinoIoTCloud.

   Copyright 2024 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of ArduinoIoTCloud.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

#ifndef ARDUINO_IOT_CLOUD_DEVICE_H
#define ARDUINO_IOT_CLOUD_DEVICE_H

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <interfaces/CloudProcess.h>
#include <ArduinoIoTCloudProperties.h>
#include <utility/time/TimedAttempt.h>

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudDevice: public ArduinoIoTCloudProcess, public ArduinoIoTCloudPropertiesClass
{
  public:

             ArduinoIoTCloudDevice();
    virtual ~ArduinoIoTCloudDevice() { }

    virtual void begin(deliverCallbackFunc cb) override;
    virtual void update() override;
    virtual int  connected() override;
    virtual void handleMessage(ArduinoIoTCloudProcessEvent ev, char* msg) override;

  private:

    enum class State
    {
      SendCapabilities,
      RequestThingId,
      ProcessThingId,
      AttachThing,
      Connected,
      Disconnect,
    };

    State _state;
    TimedAttempt _connection_attempt;
    String _thing_id;

#if OTA_ENABLED
    bool _ota_cap;
    int _ota_error;
    String _ota_img_sha256;
    String _ota_url;
    bool _ota_req;
#endif

    State handle_SendCapabilities();
    State handle_RequestThingId();
    State handle_ProcessThingId();
    State handle_AttachThing();
    State handle_Connected();
    State handle_Disconnect();
};

#endif /* ARDUINO_IOT_CLOUD_DEVICE_H */
