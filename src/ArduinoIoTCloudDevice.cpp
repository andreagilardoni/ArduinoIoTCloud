/*
   This file is part of ArduinoIoTCloud.

   Copyright 2019 ARDUINO SA (http://www.arduino.cc/)

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

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#ifdef HAS_TCP

#include <ArduinoIoTCloudTCP.h>
#if OTA_ENABLED
#include <utility/ota/OTA.h>
#endif

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/
 ArduinoIoTCloudDevice::ArduinoIoTCloudDevice(MessageStream *ms)
: CloudProcess(ms)
, _state{State::Init}
, _connection_attempt(0,0)
, _thing_id{""}
, _attached{false}
#if OTA_ENABLED
, _ota_cap{false}
, _ota_error{static_cast<int>(OTAError::None)}
, _ota_img_sha256{"Inv."}
, _ota_url{""}
, _ota_req{false}
#endif
{

}

void ArduinoIoTCloudDevice::begin()
{
#if OTA_ENABLED
  _ota_img_sha256 = OTA::getImageSHA256();
  DEBUG_VERBOSE("SHA256: HASH(%d) = %s", strlen(_ota_img_sha256.c_str()), _ota_img_sha256.c_str());
  _ota_cap = OTA::isCapable();

  Property* p;
  p = new CloudWrapperBool(_ota_cap);
  addPropertyToContainer(getPropertyContainer(), *p, "OTA_CAP", Permission::Read, -1);
  p = new CloudWrapperInt(_ota_error);
  addPropertyToContainer(getPropertyContainer(), *p, "OTA_ERROR", Permission::Read, -1);
  p = new CloudWrapperString(_ota_img_sha256);
  addPropertyToContainer(getPropertyContainer(), *p, "OTA_SHA256", Permission::Read, -1);
  p = new CloudWrapperString(_ota_url);
  addPropertyToContainer(getPropertyContainer(), *p, "OTA_URL", Permission::ReadWrite, -1);
  p = new CloudWrapperBool(_ota_req);
  addPropertyToContainer(getPropertyContainer(), *p, "OTA_REQ", Permission::ReadWrite, -1);
#endif /* OTA_ENABLED */

  _connection_attempt.begin(AIOT_CONFIG_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms, AIOT_CONFIG_MAX_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms);
}

void ArduinoIoTCloudDevice::update()
{
  /* Run through the state machine. */
  State next_state = _state;
  switch (_state)
  {
  case State::Init:                 next_state = handle_Init();                break;
  case State::SendCapabilities:     next_state = handle_SendCapabilities();    break;
  case State::ProcessThingId:       next_state = handle_ProcessThingId();      break;
  case State::Connected:            next_state = handle_Connected();           break;
  case State::Disconnected:         next_state = handle_Disconnected();        break;
  }
  _state = next_state;
}

int ArduinoIoTCloudDevice::connected()
{
 return _state != State::Disconnected ? 1 : 0;
}

void ArduinoIoTCloudDevice::handleMessage(Message* m)
{
  Serial.println((int)m->id);
  switch (m->id)
  {
    /* We have received a new thing id message */
    case AttachThing:
    Serial.println(((ThingGetIdCmdDown*)m)->params.thing_id);
    _thing_id = String(((ThingGetIdCmdDown*)m)->params.thing_id);
    _state = State::ProcessThingId;
    break;

    /* We have received a reset command */
    case Reset:
    _state = State::Init;
    break;

    case SendCapabilities:
    case GetThingId:
    case GetLastValues:
    case LastValues:
    case SendProperties:
    default:
    break;
  }
}
ArduinoIoTCloudDevice::State ArduinoIoTCloudDevice::handle_Init()
{
  /* Reset attempt struct for the nex retry after disconnection */
  _connection_attempt.begin(AIOT_CONFIG_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms, AIOT_CONFIG_MAX_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms);

  /* Reset thing id */
  _thing_id = "";

  _attached = false;

  return State::SendCapabilities;
}

ArduinoIoTCloudDevice::State ArduinoIoTCloudDevice::handle_SendCapabilities()
{
  /* Now: Sends message into device topic Will: LIB_VERSION? */
  _message.id = SendCapabilities;
  deliver(&_message);

  /* Now: Subscribe to device topic. Will: send Thing.begin() */
  _message.id = GetThingId;
  deliver(&_message);

  /* No device configuration received. Wait: 4s -> 8s -> 16s -> 32s -> 32s ...*/
  _connection_attempt.retry();
  DEBUG_VERBOSE("CloudDevice::%s not attached. %d next configuration request in %d ms", __FUNCTION__, _connection_attempt.getRetryCount(), _connection_attempt.getWaitTime());
  return State::Connected;
}

ArduinoIoTCloudDevice::State ArduinoIoTCloudDevice::handle_ProcessThingId()
{
  /* ToDo detach only if thing_id is different */
  if (_attached)
  {
    Serial.println("Disconnect");
    return State::Disconnected;
    _attached = false;
  }

  /* Temporary solution find a better way to understand if we have a valid thing_id */
  if (!hasThingId())
  {
    Serial.println("Not Attached");
    return State::Connected;
  }

  Serial.println("AttachThing");
  _message.id = AttachThing;
  deliver(&_message);
  _attached = true;
  _connection_attempt.begin(AIOT_CONFIG_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms, AIOT_CONFIG_MAX_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms);
  return State::Connected;
}

ArduinoIoTCloudDevice::State ArduinoIoTCloudDevice::handle_Connected()
{
  if (!_attached)
  {
    if (_connection_attempt.isExpired())
    {
      if (!hasThingId())
      {
        /* Device configuration received, but invalid thing_id. Do not increase counter, but recompute delay.
         * Device not attached. Wait: 4s -> 80s -> 160s -> 320s -> 640s -> 1280s -> 1280s ...
         */
        _connection_attempt.reconfigure(AIOT_CONFIG_DEVICE_TOPIC_ATTACH_RETRY_DELAY_ms, AIOT_CONFIG_MAX_DEVICE_TOPIC_ATTACH_RETRY_DELAY_ms);
        //DEBUG_VERBOSE("CloudDevice::%s not attached. %d next configuration request in %d ms", __FUNCTION__, _connection_attempt.getRetryCount(), _connection_attempt.getWaitTime());
      }
      return State::SendCapabilities;
    }

    /* Max retry than disconnect */
    if (_connection_attempt.getRetryCount() > AIOT_CONFIG_DEVICE_TOPIC_MAX_RETRY_CNT)
    {
      Serial.println("Device disconnect");
      return State::Disconnected;
    }
  }

#if OTA_ENABLED
  /* Check if we have received the OTA_URL property and provide
  * echo to the cloud.
  */
  if (_ota_url.length())
  {
    _message.id = OtaUrl;
    deliver(&_message);
    _ota_url = "";
  }

  /* Request a OTA download if the hidden property
  * OTA request has been set.
  */
  if (_ota_req)
  {
    /* TODO find a way to restore deferred OTA capabilities */
    bool const perform_ota_now = true;
    if (perform_ota_now) {
      /* Clear the error flag. */
      _ota_error = static_cast<int>(OTAError::None);
      /* Clear the request flag. */
      _ota_req = false;
      /* Transmit the cleared request flags to the cloud. */
      _message.id = OtaReq;
      deliver(&_message);
      /* Call member function to handle OTA request. */
      _message.id = OtaStart;
      deliver(&_message);
      /* If something fails send the OTA error to the cloud */
      _message.id = OtaError;
      deliver(&_message);
    }
  }
#endif

  return State::Connected;
}

ArduinoIoTCloudDevice::State ArduinoIoTCloudDevice::handle_Disconnected()
{
  return State::Disconnected;
}

#endif
