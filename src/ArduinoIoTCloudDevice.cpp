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
 ArduinoIoTCloudDevice::ArduinoIoTCloudDevice()
: _state{State::SendCapabilities}
, _connection_attempt(0,0)
#if OTA_ENABLED
, _ota_cap{false}
, _ota_error{static_cast<int>(OTAError::None)}
, _ota_img_sha256{"Inv."}
, _ota_url{""}
, _ota_req{false}
#endif
{

}

void ArduinoIoTCloudDevice::begin(deliverCallbackFunc cb)
{
  _deliver = cb;

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
  case State::SendCapabilities:     next_state = handle_SendCapabilities();    break;
  case State::RequestThingId:       next_state = handle_RequestThingId();      break;
  case State::ProcessThingId:       next_state = handle_ProcessThingId();      break;
  case State::AttachThing:          next_state = handle_AttachThing();         break;
  case State::Connected:            next_state = handle_Connected();           break;
  case State::Disconnect:           next_state = handle_Disconnect();          break;
  }
  _state = next_state;
}

int ArduinoIoTCloudDevice::connected()
{

}

void ArduinoIoTCloudDevice::handleMessage(ArduinoIoTCloudProcessEvent ev, char* msg)
{
  String message = "";

  if (msg != nullptr)
  {
    message = String(msg);
  }

  switch (ev)
  {
    /* We have received a new thing id message */
    case ArduinoIoTCloudProcessEvent::AttachThing:
    _thing_id = message;
    _state = State::ProcessThingId;
    break;

    /* We have received a reset command */
    case ArduinoIoTCloudProcessEvent::Disconnect:
    _state = State::Disconnect;
    break;

    case ArduinoIoTCloudProcessEvent::SendCapabilities:
    case ArduinoIoTCloudProcessEvent::GetThingId:
    case ArduinoIoTCloudProcessEvent::RequestlastValues:
    case ArduinoIoTCloudProcessEvent::LastValues:
    case ArduinoIoTCloudProcessEvent::SendProperties:
    case ArduinoIoTCloudProcessEvent::OtaUrl:
    case ArduinoIoTCloudProcessEvent::OtaReq:
    case ArduinoIoTCloudProcessEvent::OtaConfirm:
    case ArduinoIoTCloudProcessEvent::OtaStart:
    case ArduinoIoTCloudProcessEvent::OtaError:
    default:
    break;
  }
}

ArduinoIoTCloudDevice::State ArduinoIoTCloudDevice::handle_SendCapabilities()
{
  _deliver(ArduinoIoTCloudProcessEvent::SendCapabilities);
  return State::RequestThingId;
}

ArduinoIoTCloudDevice::State ArduinoIoTCloudDevice::handle_RequestThingId()
{
  if (_connection_attempt.isRetry() && !_connection_attempt.isExpired())
    return State::RequestThingId;

  if (_connection_attempt.isRetry())
  {
    /* Configuration not received or device not attached to a valid thing. Try to resubscribe */
    DEBUG_ERROR("CloudDevice::%s device waiting for valid thing_id %d", __FUNCTION__, getTime());
  }

  DEBUG_VERBOSE("CloudDevice::%s request device configuration %d", __FUNCTION__, getTime());

  /* If device_id is wrong the board can't connect to the broker so subscribe to device topic
   * should never happen. TODO investigate if we need to check for errors and how.
   */
  _deliver(ArduinoIoTCloudProcessEvent::GetThingId);

  /* Max retry than disconnect */
  if (_connection_attempt.getRetryCount() > AIOT_CONFIG_DEVICE_TOPIC_MAX_RETRY_CNT)
  {
    return State::Disconnect;
  }

  /* No device configuration received. Wait: 4s -> 8s -> 16s -> 32s -> 32s ...*/
  unsigned long subscribe_retry_delay = _connection_attempt.retry();
  DEBUG_VERBOSE("CloudDevice::%s %d next configuration request in %d ms", __FUNCTION__, _connection_attempt.getRetryCount(), subscribe_retry_delay);

  return State::RequestThingId;
}

ArduinoIoTCloudDevice::State ArduinoIoTCloudDevice::handle_ProcessThingId()
{
  /* Temporary solution find a better way to understand if we have a valid thing_id*/
  if (_thing_id.length() == 0)
  {
    /* Device configuration received, but invalid thing_id. Do not increase counter, but recompute delay.
    * Device not attached. Wait: 40s -> 80s -> 160s -> 320s -> 640s -> 1280s -> 1280s ...
    */
    unsigned long attach_retry_delay = _connection_attempt.reconfigure(AIOT_CONFIG_DEVICE_TOPIC_ATTACH_RETRY_DELAY_ms, AIOT_CONFIG_MAX_DEVICE_TOPIC_ATTACH_RETRY_DELAY_ms);
    DEBUG_VERBOSE("CloudDevice::%s device not attached, next configuration request in %d ms", __FUNCTION__, attach_retry_delay);
    return State::RequestThingId;
  }

  /* Received valid thing_id, reconfigure timers for next state and go on */
  _connection_attempt.begin(AIOT_CONFIG_THING_TOPICS_SUBSCRIBE_RETRY_DELAY_ms);
  return State::AttachThing;
}

ArduinoIoTCloudDevice::State ArduinoIoTCloudDevice::handle_AttachThing()
{
  if (_connection_attempt.isRetry() && !_connection_attempt.isExpired())
    return State::AttachThing;

  if (_connection_attempt.getRetryCount() > AIOT_CONFIG_THING_TOPICS_SUBSCRIBE_MAX_RETRY_CNT)
  {
    return State::Disconnect;
  }

  _connection_attempt.retry();

  /* We receive the thing from the cloud so subscribe to thing topic should never fail
   * TODO investigate if we need to check for errors and how.
   */
  _deliver(ArduinoIoTCloudProcessEvent::AttachThing);

  DEBUG_VERBOSE("CloudDevice::%s device attached to a new valid thing_id %s %d", __FUNCTION__, _thing_id.c_str(), getTime());

  _connection_attempt.begin(AIOT_CONFIG_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms, AIOT_CONFIG_MAX_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms);
  return State::Connected;
}

ArduinoIoTCloudDevice::State ArduinoIoTCloudDevice::handle_Connected()
{
#if OTA_ENABLED
  /* Check if we have received the OTA_URL property and provide
  * echo to the cloud.
  */
  if (_ota_url.length())
  {
    _deliver(ArduinoIoTCloudProcessEvent::OtaUrl);
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
      _deliver(ArduinoIoTCloudProcessEvent::OtaReq);
      /* Call member function to handle OTA request. */
      _deliver(ArduinoIoTCloudProcessEvent::OtaStart);
      /* If something fails send the OTA error to the cloud */
      _deliver(ArduinoIoTCloudProcessEvent::OtaError);
    }
  }
#endif

  return State::Connected;
}

ArduinoIoTCloudDevice::State ArduinoIoTCloudDevice::handle_Disconnect()
{
  /* Inform Infra we need to disconnect */
  _deliver(ArduinoIoTCloudProcessEvent::Disconnect);

  /* Reset attempt struct for the nex retry after disconnection */
  _connection_attempt.begin(AIOT_CONFIG_TIMEOUT_FOR_LASTVALUES_SYNC_ms);

  /* Reset thing id*/
  _thing_id = "";

  return State::SendCapabilities;
}

#endif
