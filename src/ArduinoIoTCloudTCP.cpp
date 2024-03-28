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

#include <AIoTC_Config.h>

#ifdef HAS_TCP

#include <ArduinoIoTCloudTCP.h>

#if OTA_ENABLED
  #include "ota/OTA.h"
#endif

#include <algorithm>
#include "cbor/CBOREncoder.h"
#include "cbor/CBORDecoder.h"
#include "utility/watchdog/Watchdog.h"
#include <typeinfo> 

/******************************************************************************
   LOCAL MODULE FUNCTIONS
 ******************************************************************************/

unsigned long getTime()
{
  return ArduinoCloud.getInternalTime();
}

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoIoTCloudTCP::ArduinoIoTCloudTCP()
: _state{State::ConnectPhy}
, _device_id{""}
, _thing_id{""}
, _connection_attempt(0,0)
, _mqtt_data_buf{0}
, _mqtt_data_len{0}
, _mqtt_data_request_retransmit{false}
#ifdef BOARD_HAS_SECRET_KEY
, _password("")
#endif
, _mqttClient{nullptr}
, _messageTopicOut("")
, _messageTopicIn("")
, _dataTopicOut("")
, _dataTopicIn("")
, _message_stream(std::bind(&ArduinoIoTCloudTCP::sendMessage, this, std::placeholders::_1))
, _thing(&_message_stream)
, _device(&_message_stream)
#if OTA_ENABLED
, _ota(&_message_stream)
, _ask_user_before_executing_ota{false}
, _get_ota_confirmation{nullptr}
#endif /* OTA_ENABLED */
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int ArduinoIoTCloudTCP::begin(ConnectionHandler & connection, bool const enable_watchdog, String brokerAddress, uint16_t brokerPort)
{
  _connection = &connection;
  _brokerAddress = brokerAddress;
#ifdef BOARD_HAS_SECRET_KEY
  _brokerPort = _password.length() ? DEFAULT_BROKER_PORT_USER_PASS_AUTH : brokerPort;
#else
  _brokerPort = brokerPort;
#endif

  /* Setup broker TLS client */
  _brokerClient.begin(connection);

#if  OTA_ENABLED
  /* Setup OTA TLS client */
  _otaClient.begin(connection);
#endif

  /* Setup TimeService */
  _time_service.begin(_connection);

  /* Setup retry timers */
  _connection_attempt.begin(AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms, AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms);
  return begin(enable_watchdog, _brokerAddress, _brokerPort);
}

int ArduinoIoTCloudTCP::begin(bool const enable_watchdog, String brokerAddress, uint16_t brokerPort)
{
  _brokerAddress = brokerAddress;
  _brokerPort = brokerPort;

#if defined(BOARD_HAS_SECRET_KEY)
  /* If board is not configured for username and password login */
  if(!_password.length())
  {
#endif

#if defined(BOARD_HAS_SECURE_ELEMENT)
    if (!_selement.begin())
    {
      DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not initialize secure element.", __FUNCTION__);
  #if defined(ARDUINO_UNOWIFIR4)
      if (String(WiFi.firmwareVersion()) < String("0.4.1")) {
        DEBUG_ERROR("ArduinoIoTCloudTCP::%s In order to read device certificate, WiFi firmware needs to be >= 0.4.1, current %s", __FUNCTION__, WiFi.firmwareVersion());
      }
  #endif
      return 0;
    }
    if (!SElementArduinoCloudDeviceId::read(_selement, getDeviceId(), SElementArduinoCloudSlot::DeviceId))
    {
      DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not read device id.", __FUNCTION__);
      return 0;
    }
  #if !defined(BOARD_HAS_OFFLOADED_ECCX08)
    if (!SElementArduinoCloudCertificate::read(_selement, _cert, SElementArduinoCloudSlot::CompressedCertificate))
    {
      DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not read device certificate.", __FUNCTION__);
      return 0;
    }
    _brokerClient.setEccSlot(static_cast<int>(SElementArduinoCloudSlot::Key), _cert.bytes(), _cert.length());
    #if  OTA_ENABLED
    _otaClient.setEccSlot(static_cast<int>(SElementArduinoCloudSlot::Key), _cert.bytes(), _cert.length());
    #endif
  #endif
#endif

#if defined(BOARD_HAS_SECRET_KEY)
  }
#endif

  _mqttClient.setClient(_brokerClient);

#ifdef BOARD_HAS_SECRET_KEY
  if(_password.length())
  {
    _mqttClient.setUsernamePassword(getDeviceId(), _password);
  }
#endif

  _mqttClient.onMessage(ArduinoIoTCloudTCP::onMessage);
  _mqttClient.setKeepAliveInterval(30 * 1000);
  _mqttClient.setConnectionTimeout(1500);
  _mqttClient.setId(getDeviceId().c_str());

  _messageTopicOut = getTopic_messageout();
  _messageTopicIn  = getTopic_messagein();

  _thing.begin();
  _device.begin();

#if  OTA_ENABLED
  _ota.setClient(&_otaClient);
#endif // OTA_ENABLED

#ifdef BOARD_HAS_OFFLOADED_ECCX08
  if (String(WiFi.firmwareVersion()) < String("1.4.4")) {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s In order to connect to Arduino IoT Cloud, NINA firmware needs to be >= 1.4.4, current %s", __FUNCTION__, WiFi.firmwareVersion());
    return 0;
  }
#endif /* BOARD_HAS_OFFLOADED_ECCX08 */

#if defined(ARDUINO_UNOWIFIR4)
  if (String(WiFi.firmwareVersion()) < String("0.2.0")) {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s In order to connect to Arduino IoT Cloud, WiFi firmware needs to be >= 0.2.0, current %s", __FUNCTION__, WiFi.firmwareVersion());
  }
#endif

  /* Since we do not control what code the user inserts
   * between ArduinoIoTCloudTCP::begin() and the first
   * call to ArduinoIoTCloudTCP::update() it is wise to
   * set a rather large timeout at first.
   */
#if defined (ARDUINO_ARCH_SAMD) || defined (ARDUINO_ARCH_MBED)
  if (enable_watchdog) {
    /* Initialize watchdog hardware */
    watchdog_enable();
    /* Setup callbacks to feed the watchdog during offloaded network operations (connection/download)*/
    watchdog_enable_network_feed(_connection->getInterface());
  }
#endif

  return 1;
}

void ArduinoIoTCloudTCP::update()
{
  /* Feed the watchdog. If any of the functions called below
   * get stuck than we can at least reset and recover.
   */
#if defined (ARDUINO_ARCH_SAMD) || defined (ARDUINO_ARCH_MBED)
  watchdog_reset();
#endif

  /* Run through the state machine. */
  State next_state = _state;
  switch (_state)
  {
  case State::ConnectPhy:           next_state = handle_ConnectPhy();           break;
  case State::SyncTime:             next_state = handle_SyncTime();             break;
  case State::ConnectMqttBroker:    next_state = handle_ConnectMqttBroker();    break;
  case State::Connected:            next_state = handle_Connected();            break;
  case State::Disconnect:           next_state = handle_Disconnect();           break;
  }
  _state = next_state;

  /* This watchdog feed is actually needed only by the RP2040 Connect because its
   * maximum watchdog window is 8389 ms; despite this we feed it for all
   * supported ARCH to keep code aligned.
   */
#if defined (ARDUINO_ARCH_SAMD) || defined (ARDUINO_ARCH_MBED)
  watchdog_reset();
#endif

  /* Check for new data from the MQTT client. */
  if (_mqttClient.connected())
    _mqttClient.poll();
}

int ArduinoIoTCloudTCP::connected()
{
  return _mqttClient.connected();
}

void ArduinoIoTCloudTCP::printDebugInfo()
{
  DEBUG_INFO("***** Arduino IoT Cloud - configuration info *****");
  DEBUG_INFO("Device ID: %s", getDeviceId().c_str());
  DEBUG_INFO("MQTT Broker: %s:%d", _brokerAddress.c_str(), _brokerPort);
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_ConnectPhy()
{

  if (_connection->check() == NetworkConnectionState::CONNECTED)
  {
    if (!_connection_attempt.isRetry() || (_connection_attempt.isRetry() && _connection_attempt.isExpired()))
      return State::SyncTime;
  }

  return State::ConnectPhy;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_SyncTime()
{
  if (TimeServiceClass::isTimeValid(getTime()))
  {
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s internal clock configured to posix timestamp %d", __FUNCTION__, getTime());
    return State::ConnectMqttBroker;
  }

  return State::ConnectPhy;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_ConnectMqttBroker()
{
  if (_mqttClient.connect(_brokerAddress.c_str(), _brokerPort))
  {
    _mqttClient.subscribe(getTopic_messagein());
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s connected to %s:%d", __FUNCTION__, _brokerAddress.c_str(), _brokerPort);
    /* Reconfigure timers for next state */
    _connection_attempt.begin(AIOT_CONFIG_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms, AIOT_CONFIG_MAX_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms);
    return State::Connected;
  }

  /* Can't connect to the broker. Wait: 2s -> 4s -> 8s -> 16s -> 32s -> 32s ... */
  _connection_attempt.retry();

  DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not connect to %s:%d", __FUNCTION__, _brokerAddress.c_str(), _brokerPort);
  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s %d next connection attempt in %d ms", __FUNCTION__, _connection_attempt.getRetryCount(), _connection_attempt.getWaitTime());
  /* Go back to ConnectPhy and retry to get time from network (invalid time for SSL handshake?)*/
  return State::ConnectPhy;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_Connected()
{
  if (!_mqttClient.connected() || !_device.connected())
  {
    return State::Disconnect;
  }

  _device.update();

#if  OTA_ENABLED
  _ota.update();
#endif // OTA_ENABLED

  if(_device.isAttached())
  {
    _thing.update();
  }

  /* Retransmit data in case there was a lost transaction due
  * to phy layer or MQTT connectivity loss.
  */
  if (_mqtt_data_request_retransmit && (_mqtt_data_len > 0)) {
    write(_dataTopicOut, _mqtt_data_buf, _mqtt_data_len);
    _mqtt_data_request_retransmit = false;
  }
  return State::Connected;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_Disconnect()
{
  if (!_mqttClient.connected()) {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s MQTT client connection lost", __FUNCTION__);
  } else {
    _mqttClient.unsubscribe(_dataTopicIn);
    /* TODO add device topic */
    _mqttClient.stop();
  }

  Message message;
  message.id = Reset;
  _device.handleMessage(&message);
  _thing.handleMessage(&message);
  DEBUG_INFO("Disconnected from Arduino IoT Cloud");
  execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);

  /* Setup timer for broker connection and restart */
  _connection_attempt.begin(AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms, AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms);
  return State::ConnectPhy;
}

void ArduinoIoTCloudTCP::onMessage(int length)
{
  ArduinoCloud.handleMessage(length);
}

void ArduinoIoTCloudTCP::handleMessage(int length)
{
  String topic = _mqttClient.messageTopic();

  byte bytes[length];

  for (int i = 0; i < length; i++) {
    bytes[i] = _mqttClient.read();
  }

  /* Topic for user input data */
  if (_dataTopicIn == topic) {
    CBORDecoder::decode(getThing().getPropertyContainer(), (uint8_t*)bytes, length);
  }

  /* Topic for device commands */
  if (_messageTopicIn == topic) {
    CommandDown command;
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] received %d bytes", __FUNCTION__, millis(), length);
    CBORMessageDecoder decoder;

    size_t buffer_length = length;
    if (decoder.decode((Message*)&command, bytes, buffer_length) != Decoder::Status::Error) {
      DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] received command id %d", __FUNCTION__, millis(), command.c.id);
      switch (command.c.id)
      {
        case CommandID::ThingGetIdCmdDownId:
        {
          ThingGetIdCmdDown * msg = (ThingGetIdCmdDown *)&command;
          _thing_id = String(msg->params.thing_id);
          DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] device configuration received", __FUNCTION__, millis());
          _device.handleMessage((Message*)&command);
        }
        break;

        case CommandID::ThingGetLastValueCmdDownId:
        {
          DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] last values received", __FUNCTION__, millis());
          ThingGetLastValueCmdDown * msg = (ThingGetLastValueCmdDown*)&command;
          CBORDecoder::decode(getThing().getPropertyContainer(), (uint8_t*)msg->params.last_values, msg->params.length, true);
          _thing.handleMessage((Message*)&command);
          execCloudEventCallback(ArduinoIoTCloudEvent::SYNC);
        }
        break;

#if OTA_ENABLED
        case CommandID::OtaUpdateCmdDownId:
        {
          DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] ota update received", __FUNCTION__, millis());
          _ota.handleMessage((Message*)&command);
        }
#endif

        default:
        break;
      }
    }
  }
}

void ArduinoIoTCloudTCP::sendMessage(Message * msg)
{
  switch (msg->id)
  {
    case SendProperties:
    return sendThingPropertiesToCloud();
    break;

    case AttachThing:
    return attachThing();
    break;
    default:
    break;
  }

  uint8_t data[MQTT_TRANSMIT_BUFFER_SIZE];
  size_t bytes_encoded = sizeof(data);
  CBORMessageEncoder encoder;

  if (encoder.encode(msg, data, bytes_encoded) == Encoder::Status::Complete &&
      bytes_encoded > 0) {
    write(_messageTopicOut, data, bytes_encoded);
  } else {
    DEBUG_ERROR("error encoding %d", msg->id);
  }
}

void ArduinoIoTCloudTCP::sendPropertyContainerToCloud(String const topic, PropertyContainer & container, unsigned int & current_property_index)
{
  int bytes_encoded = 0;
  uint8_t data[MQTT_TRANSMIT_BUFFER_SIZE];

  if (CBOREncoder::encode(container, data, sizeof(data), bytes_encoded, current_property_index, false) == CborNoError)
  {
    if (bytes_encoded > 0)
    {
      /* If properties have been encoded store them in the back-up buffer
       * in order to allow retransmission in case of failure.
       */
      _mqtt_data_len = bytes_encoded;
      memcpy(_mqtt_data_buf, data, _mqtt_data_len);
      /* Transmit the properties to the MQTT broker */
      write(topic, _mqtt_data_buf, _mqtt_data_len);
    }
  }
}

void ArduinoIoTCloudTCP::sendThingPropertiesToCloud()
{
  sendPropertyContainerToCloud(_dataTopicOut, _thing.getPropertyContainer(), _thing.getPropertyContainerIndex());
}


void ArduinoIoTCloudTCP::attachThing()
{
  _dataTopicIn    = getTopic_datain();
  _dataTopicOut   = getTopic_dataout();
  if (!_mqttClient.subscribe(_dataTopicIn))
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _dataTopicIn.c_str());
    DEBUG_ERROR("Check your thing configuration, and press the reset button on your board.");
    return;
  }

  DEBUG_INFO("Connected to Arduino IoT Cloud");
  DEBUG_INFO("Thing ID: %s", getThingId().c_str());
  execCloudEventCallback(ArduinoIoTCloudEvent::CONNECT);
}

int ArduinoIoTCloudTCP::write(String const topic, byte const data[], int const length)
{
  if (_mqttClient.beginMessage(topic, length, false, 0)) {
    if (_mqttClient.write(data, length)) {
      if (_mqttClient.endMessage()) {
        return 1;
      }
    }
  }
  return 0;
}

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/

ArduinoIoTCloudTCP ArduinoCloud;

#endif
