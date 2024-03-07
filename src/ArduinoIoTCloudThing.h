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

#ifndef ARDUINO_IOT_CLOUD_THING_H
#define ARDUINO_IOT_CLOUD_THING_H

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <interfaces/CloudProcess.h>
#include <ArduinoIoTCloudProperties.h>
#include <utility/time/TimedAttempt.h>

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudThing: public ArduinoIoTCloudProcess , public ArduinoIoTCloudPropertiesClass
{
  public:

             ArduinoIoTCloudThing(MessageStream* stream);
    virtual ~ArduinoIoTCloudThing() { }

    virtual void begin(deliverCallbackFunc cb) override;
    virtual void update() override;
    virtual int  connected() override;
    virtual void handleMessage(Message* m) override;

  private:

    enum class State
    {
      RequestLastValues, //Init
      Connected,         //Connected
      Disconnect,        //Error
    };

    State _state;
    TimedAttempt _connection_attempt;
    int _tz_offset;
    Property * _tz_offset_property;
    unsigned int _tz_dst_until;
    Property * _tz_dst_until_property;
    Message _message;

    State handle_RequestLastValues();
    State handle_Connected();
    State handle_Disconnect();
};

#endif /* ARDUINO_IOT_CLOUD_THING_H */