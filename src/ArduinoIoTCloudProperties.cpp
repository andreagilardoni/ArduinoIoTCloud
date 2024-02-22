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

ArduinoIoTCloudPropertiesClass::ArduinoIoTCloudPropertiesClass()
: last_checked_property_index{0}
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

/* The following methods are used for non-LoRa boards */
Property& ArduinoIoTCloudPropertiesClass::addPropertyReal(bool& property, String name, Permission const permission)
{
  return addPropertyReal(property, name, -1, permission);
}
Property& ArduinoIoTCloudPropertiesClass::addPropertyReal(float& property, String name, Permission const permission)
{
  return addPropertyReal(property, name, -1, permission);
}
Property& ArduinoIoTCloudPropertiesClass::addPropertyReal(int& property, String name, Permission const permission)
{
  return addPropertyReal(property, name, -1, permission);
}
Property& ArduinoIoTCloudPropertiesClass::addPropertyReal(unsigned int& property, String name, Permission const permission)
{
  return addPropertyReal(property, name, -1, permission);
}
Property& ArduinoIoTCloudPropertiesClass::addPropertyReal(String& property, String name, Permission const permission)
{
  return addPropertyReal(property, name, -1, permission);
}
Property& ArduinoIoTCloudPropertiesClass::addPropertyReal(Property& property, String name, Permission const permission)
{
  return addPropertyReal(property, name, -1, permission);
}

/* The following methods are used for both LoRa and non-Lora boards */
Property& ArduinoIoTCloudPropertiesClass::addPropertyReal(bool& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperBool(property);
  return addPropertyReal(*p, name, tag, permission);
}
Property& ArduinoIoTCloudPropertiesClass::addPropertyReal(float& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperFloat(property);
  return addPropertyReal(*p, name, tag, permission);
}
Property& ArduinoIoTCloudPropertiesClass::addPropertyReal(int& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperInt(property);
  return addPropertyReal(*p, name, tag, permission);
}
Property& ArduinoIoTCloudPropertiesClass::addPropertyReal(unsigned int& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperUnsignedInt(property);
  return addPropertyReal(*p, name, tag, permission);
}
Property& ArduinoIoTCloudPropertiesClass::addPropertyReal(String& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperString(property);
  return addPropertyReal(*p, name, tag, permission);
}
Property& ArduinoIoTCloudPropertiesClass::addPropertyReal(Property& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(properties, property, name, permission, tag);
}

/* The following methods are deprecated but still used for non-LoRa boards */
void ArduinoIoTCloudPropertiesClass::addPropertyReal(bool& property, String name, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperBool(property);
  addPropertyRealInternal(*p, name, -1, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudPropertiesClass::addPropertyReal(float& property, String name, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperFloat(property);
  addPropertyRealInternal(*p, name, -1, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudPropertiesClass::addPropertyReal(int& property, String name, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperInt(property);
  addPropertyRealInternal(*p, name, -1, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudPropertiesClass::addPropertyReal(unsigned int& property, String name, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperUnsignedInt(property);
  addPropertyRealInternal(*p, name, -1, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudPropertiesClass::addPropertyReal(String& property, String name, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperString(property);
  addPropertyRealInternal(*p, name, -1, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudPropertiesClass::addPropertyReal(Property& property, String name, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, -1, permission_type, seconds, fn, minDelta, synFn);
}

/* The following methods are deprecated but still used for both LoRa and non-LoRa boards */
void ArduinoIoTCloudPropertiesClass::addPropertyReal(bool& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperBool(property);
  addPropertyRealInternal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudPropertiesClass::addPropertyReal(float& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperFloat(property);
  addPropertyRealInternal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudPropertiesClass::addPropertyReal(int& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperInt(property);
  addPropertyRealInternal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudPropertiesClass::addPropertyReal(unsigned int& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperUnsignedInt(property);
  addPropertyRealInternal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudPropertiesClass::addPropertyReal(String& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperString(property);
  addPropertyRealInternal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudPropertiesClass::addPropertyReal(Property& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}

void ArduinoIoTCloudPropertiesClass::addPropertyRealInternal(Property& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Permission permission = Permission::ReadWrite;
  if (permission_type == READ) {
    permission = Permission::Read;
  } else if (permission_type == WRITE) {
    permission = Permission::Write;
  } else {
    permission = Permission::ReadWrite;
  }

  if (seconds == ON_CHANGE) {
    addPropertyToContainer(properties, property, name, permission, tag).publishOnChange(minDelta, Property::DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS).onUpdate(fn).onSync(synFn);
  } else {
    addPropertyToContainer(properties, property, name, permission, tag).publishEvery(seconds).onUpdate(fn).onSync(synFn);
  }
}


