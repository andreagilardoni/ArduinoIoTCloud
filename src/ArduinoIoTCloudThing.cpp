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

#include <ArduinoIoTCloudTCP.h>

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoIoTCloudThing::ArduinoIoTCloudThing()
: _state{State::RequestLastValues}
, _connection_attempt(0,0)
, _tz_offset{0}
, _tz_offset_property{nullptr}
, _tz_dst_until{0}
, _tz_dst_until_property{nullptr}
{

}

void ArduinoIoTCloudThing::begin(onSendMessageUpstreamCallbackFunc cb)
{
  _send_message_upstream = cb;

  Property* p;
  p = new CloudWrapperInt(_tz_offset);
  _tz_offset_property = &addPropertyToContainer(getPropertyContainer(), *p, "tz_offset", Permission::ReadWrite, -1).writeOnDemand();
  p = new CloudWrapperUnsignedInt(_tz_dst_until);
  _tz_dst_until_property = &addPropertyToContainer(getPropertyContainer(), *p, "tz_dst_until", Permission::ReadWrite, -1).writeOnDemand();
}

void ArduinoIoTCloudThing::update()
{
  /* Run through the state machine. */
  State next_state = _state;
  switch (_state)
  {
  case State::RequestLastValues:    next_state = handle_RequestLastValues();    break;
  case State::Connected:            next_state = handle_Connected();            break;
  case State::Disconnect:           next_state = handle_Disconnect();           break;
  }
  _state = next_state;
}

int  ArduinoIoTCloudThing::connected()
{

}

void ArduinoIoTCloudThing::printDebugInfo()
{

}

void ArduinoIoTCloudThing::push()
{
  requestUpdateForAllProperties(getPropertyContainer());
}

bool ArduinoIoTCloudThing::setTimestamp(String const & prop_name, unsigned long const timestamp)
{
  Property * p = getProperty(getPropertyContainer(), prop_name);

  if (p == nullptr)
  {
    return false;
  }

  p->setTimestamp(timestamp);

  return true;
}

void ArduinoIoTCloudThing::sendMessageUpstream(ArduinoIoTCloudProcess::Event id)
{
  if (_send_message_upstream)
  {
    _send_message_upstream(id);
  }
}

ArduinoIoTCloudThing::State ArduinoIoTCloudThing::handle_RequestLastValues()
{
  /* Check whether or not we need to send a new request. */
  if (_connection_attempt.isRetry() && !_connection_attempt.isExpired())
  {
    return State::RequestLastValues;
  }

  /* Track the number of times a get-last-values request was sent to the cloud.
   * If no data is received within a certain number of retry-requests it's a better
   * strategy to disconnect and re-establish connection from the ground up.
   */
  if (_connection_attempt.getRetryCount() > AIOT_CONFIG_LASTVALUES_SYNC_MAX_RETRY_CNT)
  {
    return State::Disconnect;
  }

  _connection_attempt.retry();

  /* Send message upstream to inform infrastructure we need to request thing last values */
  sendMessageUpstream(Event::RequestlastValues);

  return State::RequestLastValues;
}

ArduinoIoTCloudThing::State ArduinoIoTCloudThing::handle_Connected()
{
  /* Check if a primitive property wrapper is locally changed.
  * This function requires an existing time service which in
  * turn requires an established connection. Not having that
  * leads to a wrong time set in the time service which inhibits
  * the connection from being established due to a wrong data
  * in the reconstructed certificate.
  */
  updateTimestampOnLocallyChangedProperties(getPropertyContainer());

  /* Configure Time service with timezone data:
  * _tz_offset [offset + dst]
  * _tz_dst_until [posix timestamp until _tz_offset is valid]
  */
  if (_tz_offset_property->isDifferentFromCloud() || _tz_dst_until_property->isDifferentFromCloud())
  {
    _tz_offset_property->fromCloudToLocal();
    _tz_dst_until_property->fromCloudToLocal();
    TimeService.setTimeZoneData(_tz_offset, _tz_dst_until);
  }

  /* Check if any properties need encoding and send them to
  * the cloud if necessary.
  */
  sendMessageUpstream(Event::SendProperties);

  if (getTime() > _tz_dst_until)
  {
    return State::RequestLastValues;
  }

  return State::Connected;
}

ArduinoIoTCloudThing::State ArduinoIoTCloudThing::handle_Disconnect()
{
  /* Inform Infra we need to disconnect */
  sendMessageUpstream(Event::Disconnect);

  /* Reset attempt struct for the nex retry after disconnection */
  _connection_attempt.begin(AIOT_CONFIG_TIMEOUT_FOR_LASTVALUES_SYNC_ms);

  return State::RequestLastValues;
}
