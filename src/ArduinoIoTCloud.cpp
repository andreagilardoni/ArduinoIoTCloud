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

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <ArduinoIoTCloud.h>

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoIoTCloudClass::ArduinoIoTCloudClass()
: _connection{nullptr}
, _time_service(TimeService)
, _lib_version{AIOT_CONFIG_LIB_VERSION}
, _cloud_event_callback{nullptr}
, _thing_id_outdated{false}
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/
void ArduinoIoTCloudClass::push()
{
  requestUpdateForAllProperties(getThing().getPropertyContainer());
}

bool ArduinoIoTCloudClass::setTimestamp(String const & prop_name, unsigned long const timestamp)
{
  Property * p = getProperty(getThing().getPropertyContainer(), prop_name);

  if (p == nullptr)
    return false;

  p->setTimestamp(timestamp);

  return true;
}

void ArduinoIoTCloudClass::addCallback(ArduinoIoTCloudEvent const event, OnCloudEventCallback callback)
{
  _cloud_event_callback[static_cast<size_t>(event)] = callback;
}

/******************************************************************************
 * PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

void ArduinoIoTCloudClass::execCloudEventCallback(ArduinoIoTCloudEvent const event)
{
  OnCloudEventCallback callback = _cloud_event_callback[static_cast<size_t>(event)];
  if (callback) {
    (*callback)();
  }
}

__attribute__((weak)) void setDebugMessageLevel(int const /* level */)
{
  /* do nothing */
}

