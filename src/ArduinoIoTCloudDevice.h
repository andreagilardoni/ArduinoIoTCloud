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

class ArduinoIoTCloudDevice: public CloudProcess, public ArduinoIoTCloudPropertiesClass
{
  public:

             ArduinoIoTCloudDevice(MessageStream* stream);
    virtual ~ArduinoIoTCloudDevice() { }

    virtual void begin();
    virtual void update() override;
    virtual int  connected();
    virtual void handleMessage(Message* m) override;

    inline bool isAttached() { return _attached; };
    inline bool hasThingId() { return !_thing_id.isEmpty(); }

  private:

    enum class State
    {
      Disconnected,
      Init,
      SendCapabilities,
      ProcessThingId,
      Connected,
    };

    State _state;
    TimedAttempt _connection_attempt;
    String _thing_id;
    bool _attached;
    Message _message;



    State handle_Init();
    State handle_SendCapabilities();
    State handle_ProcessThingId();
    State handle_Connected();
    State handle_Disconnected();
};

#endif /* ARDUINO_IOT_CLOUD_DEVICE_H */
