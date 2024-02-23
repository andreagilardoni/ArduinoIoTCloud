/*
   This file is part of ArduinoIoTCloud.

   Copyright 2019 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

#ifndef ARDUINO_IOT_CLOUD_H
#define ARDUINO_IOT_CLOUD_H

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#include <Arduino_ConnectionHandler.h>

#if defined(DEBUG_ERROR) || defined(DEBUG_WARNING) || defined(DEBUG_INFO) || defined(DEBUG_DEBUG) || defined(DEBUG_VERBOSE)
#  include <Arduino_DebugUtils.h>
#endif

#include "AIoTC_Const.h"
#include "utility/time/TimeService.h"

/******************************************************************************
   TYPEDEF
 ******************************************************************************/

typedef enum
{
  READ      = 0x01,
  WRITE     = 0x02,
  READWRITE = READ | WRITE
} permissionType;

enum class ArduinoIoTConnectionStatus
{
  IDLE,
  CONNECTING,
  CONNECTED,
  DISCONNECTED,
  RECONNECTING,
  ERROR,
};

enum class ArduinoIoTCloudEvent : size_t
{
  SYNC = 0, CONNECT = 1, DISCONNECT = 2
};

typedef void (*OnCloudEventCallback)(void);

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudClass
{
  public:

             ArduinoIoTCloudClass();
    virtual ~ArduinoIoTCloudClass() { }


    virtual void update        () = 0;
    virtual int  connected     () = 0;
    virtual void printDebugInfo() = 0;
    virtual void push          () = 0;

            bool setTimestamp(String const & prop_name, unsigned long const timestamp);

    inline ConnectionHandler * getConnection()          { return _connection; }

    inline unsigned long getInternalTime()              { return _time_service.getTime(); }
    inline unsigned long getLocalTime()                 { return _time_service.getLocalTime(); }

    void addCallback(ArduinoIoTCloudEvent const event, OnCloudEventCallback callback);

#define addProperty( v, ...) getThing().addPropertyReal(v, #v, __VA_ARGS__)

  protected:

    ConnectionHandler * _connection;
    TimeServiceClass & _time_service;
    String _lib_version;

    void execCloudEventCallback(ArduinoIoTCloudEvent const event);

  private:

    OnCloudEventCallback _cloud_event_callback[3];
    bool _thing_id_outdated;
};

#ifdef HAS_TCP
  #include "ArduinoIoTCloudTCP.h"
#elif defined(HAS_LORA)
  #include "ArduinoIoTCloudLPWAN.h"
#endif

// declaration for boards without debug library
void setDebugMessageLevel(int const level);

#endif
