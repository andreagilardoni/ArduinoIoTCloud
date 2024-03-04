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

#ifdef BOARD_HAS_ECCX08
  #include "tls/BearSSLTrustAnchors.h"
  #include "tls/utility/CryptoUtil.h"
#endif

#ifdef BOARD_HAS_SE050
  #include "tls/AIoTCSSCert.h"
  #include "tls/utility/CryptoUtil.h"
#endif

#ifdef BOARD_HAS_OFFLOADED_ECCX08
  #include <ArduinoECCX08.h>
  #include "tls/utility/CryptoUtil.h"
#endif

#ifdef BOARD_HAS_SECRET_KEY
  #include "tls/AIoTCUPCert.h"
#endif

#if OTA_ENABLED
  #include "utility/ota/OTA.h"
#endif

#include <algorithm>
#include "cbor/CBOREncoder.h"
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
, _thing_id_property{nullptr}
, _connection_attempt(0,0)
, _mqtt_data_buf{0}
, _mqtt_data_len{0}
, _mqtt_data_request_retransmit{false}
#ifdef BOARD_HAS_ECCX08
, _sslClient(nullptr, ArduinoIoTCloudTrustAnchor, ArduinoIoTCloudTrustAnchor_NUM, getTime)
#endif
  #ifdef BOARD_HAS_SECRET_KEY
, _password("")
  #endif
, _mqttClient{nullptr}
, _messageTopicOut("")
, _messageTopicIn("")
, _deviceTopicOut("")
, _deviceTopicIn("")
, _shadowTopicOut("")
, _shadowTopicIn("")
, _dataTopicOut("")
, _dataTopicIn("")
, _message_stream(std::bind(&ArduinoIoTCloudTCP::sendMessage, this, std::placeholders::_1))
#if OTA_ENABLED
, _ask_user_before_executing_ota{false}
, _get_ota_confirmation{nullptr}
, _ota(&_message_stream)
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
  _brokerPort = brokerPort;

  /* Setup TimeService */
  _time_service.begin(&connection);

  /* Setup retry timers */
  _connection_attempt.begin(AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms, AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms);
#if  OTA_ENABLED
  _ota.setConnectionHandler(_connection);
#endif // OTA_ENABLED

  return begin(enable_watchdog, _brokerAddress, _brokerPort);
}

int ArduinoIoTCloudTCP::begin(bool const enable_watchdog, String brokerAddress, uint16_t brokerPort)
{
  _brokerAddress = brokerAddress;
  _brokerPort = brokerPort;

#if defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_OFFLOADED_ECCX08) || defined(BOARD_HAS_SE050)
  if (!_crypto.begin())
  {
    DEBUG_ERROR("_crypto.begin() failed.");
    return 0;
  }
  if (!_crypto.readDeviceId(getDeviceId(), CryptoSlot::DeviceId))
  {
    DEBUG_ERROR("_crypto.readDeviceId(...) failed.");
    return 0;
  }
#endif

#if defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_SE050)
  if (!_crypto.readCert(_cert, CryptoSlot::CompressedCertificate))
  {
    DEBUG_ERROR("Cryptography certificate reconstruction failure.");
    return 0;
  }
  _sslClient.setEccSlot(static_cast<int>(CryptoSlot::Key), _cert.bytes(), _cert.length());
#endif

#if defined(BOARD_HAS_ECCX08)
  _sslClient.setClient(_connection->getClient());
#elif defined(ARDUINO_PORTENTA_C33)
  _sslClient.setClient(_connection->getClient());
  _sslClient.setCACert(AIoTSSCert);
#elif defined(BOARD_HAS_SE050)
  _sslClient.appendCustomCACert(AIoTSSCert);
#elif defined(BOARD_ESP)
  #if defined(ARDUINO_ARCH_ESP8266)
  _sslClient.setInsecure();
  #else
  _sslClient.setCACertBundle(x509_crt_bundle);
  #endif
#elif defined(ARDUINO_EDGE_CONTROL)
  _sslClient.appendCustomCACert(AIoTUPCert);
#endif

  _mqttClient.setClient(_sslClient);
#ifdef BOARD_HAS_SECRET_KEY
  _mqttClient.setUsernamePassword(getDeviceId(), _password);
#endif
  _mqttClient.onMessage(ArduinoIoTCloudTCP::onDownstreamMessage);
  _mqttClient.setKeepAliveInterval(30 * 1000);
  _mqttClient.setConnectionTimeout(1500);
  _mqttClient.setId(getDeviceId().c_str());

  _deviceTopicOut = getTopic_deviceout();
  _deviceTopicIn  = getTopic_devicein();

  _messageTopicOut = getTopic_messageout();
  _messageTopicIn  = getTopic_messagein();

  Property* p;
  p = new CloudWrapperString(_lib_version);
  addPropertyToContainer(_device.getPropertyContainer(), *p, "LIB_VERSION", Permission::Read, -1);
  p = new CloudWrapperString(_thing_id);
  _thing_id_property = &addPropertyToContainer(_device.getPropertyContainer(), *p, "thing_id", Permission::ReadWrite, -1).writeOnDemand();

  _thing.begin(onUpstreamMessage);
  _device.begin(onUpstreamMessage);

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

#if OTA_ENABLED
  _message_stream.addReceive(
    "ota", // TODO make "ota" a constant
    std::bind(&OTACloudProcessInterface::handleMessage, &_ota, std::placeholders::_1));
#endif // OTA_ENABLED

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

void ArduinoIoTCloudTCP::push()
{
  _thing.push();
}

bool ArduinoIoTCloudTCP::setTimestamp(String const & prop_name, unsigned long const timestamp)
{
  return _thing.setTimestamp(prop_name, timestamp);
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
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s connected to %s:%d", __FUNCTION__, _brokerAddress.c_str(), _brokerPort);
    /* Reconfigure timers for next state */
    _connection_attempt.begin(AIOT_CONFIG_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms, AIOT_CONFIG_MAX_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms);
    return State::Connected;
  }

  /* Can't connect to the broker. Wait: 2s -> 4s -> 8s -> 16s -> 32s -> 32s ... */
  unsigned long reconnection_retry_delay = _connection_attempt.retry();

  DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not connect to %s:%d", __FUNCTION__, _brokerAddress.c_str(), _brokerPort);
  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s %d next connection attempt in %d ms", __FUNCTION__, _connection_attempt.getRetryCount(), reconnection_retry_delay);
  /* Go back to ConnectPhy and retry to get time from network (invalid time for SSL handshake?)*/
  return State::ConnectPhy;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_Connected()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  _device.update();

#if  OTA_ENABLED
  _ota.update();
#endif // OTA_ENABLED

  if (_device.attached())
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
    _mqttClient.unsubscribe(_shadowTopicIn);
    _mqttClient.unsubscribe(_dataTopicIn);
    /* TODO add device topic */
    _mqttClient.stop();
  }

  _device.sendMessageDownstream(Event::Disconnect);
  _thing.sendMessageDownstream(Event::Disconnect);
  DEBUG_INFO("Disconnected from Arduino IoT Cloud");
  execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);

  /* Setup timer for broker connection and restart */
  _connection_attempt.begin(AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms, AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms);
  return State::ConnectPhy;
}

void ArduinoIoTCloudTCP::onDownstreamMessage(int length)
{
  ArduinoCloud.handleDownstreamMessage(length);
}

void ArduinoIoTCloudTCP::handleDownstreamMessage(int length)
{
  String topic = _mqttClient.messageTopic();

  byte bytes[length];

  for (int i = 0; i < length; i++) {
    bytes[i] = _mqttClient.read();
  }

  /* Topic for OTA properties and device configuration */
  if (_deviceTopicIn == topic) {
    CBORDecoder::decode(_device.getPropertyContainer(), (uint8_t*)bytes, length);
    /* Unlock device state machine waiting thing_id*/
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] device configuration received", __FUNCTION__, millis());
    if (_thing_id_property->isDifferentFromCloud()) {
      _thing_id_property->fromCloudToLocal();
      _device.sendMessageDownstream(Event::ThingId, _thing_id.c_str());
    }
  }

  /* Topic for user input data */
  if (_dataTopicIn == topic) {
    CBORDecoder::decode(_thing.getPropertyContainer(), (uint8_t*)bytes, length);
  }

  /* Topic for sync Thing last values on connect */
  if (_shadowTopicIn == topic) {
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] last values received", __FUNCTION__, millis());
    CBORDecoder::decode(_thing.getPropertyContainer(), (uint8_t*)bytes, length, true);
    _time_service.setTimeZoneData(_thing._tz_offset, _thing._tz_dst_until);
    execCloudEventCallback(ArduinoIoTCloudEvent::SYNC);
    /* Unlock thing state machine waiting last values */
    _thing.sendMessageDownstream(Event::LastValues);
  }

  if (_messageTopicIn == topic) {
    GenericCommand command;
    MessageDecoder::DecoderState err =  MessageDecoder::decode((Message*)&command, bytes, length);

    switch (command.command.id)
    {
      case CommandID::ThingGetIdCmdDownId:
      {
        ThingGetIdCmdDown * msg = (ThingGetIdCmdDown *)&command;
        _thing_id = msg->fields.params.thing_id;
        _device.sendMessageDownstream(Event::ThingId, msg->fields.params.thing_id);
      }
      break;

      case CommandID::ThingGetLastValueCmdDownId:
      {
        ThingGetLastValueCmdDown * msg = (ThingGetLastValueCmdDown*)&command;
        CBORDecoder::decode(_thing.getPropertyContainer(), (uint8_t*)msg->fields.params.last_values, msg->fields.params.length, true);
        _thing.sendMessageDownstream(Event::LastValues);
      }
      break;

      case CommandID::OtaUpdateCmdDownId:
      {
        _message_stream.send((Message*)&command, "ota"); // TODO make "ota" a constant
      }
      default:
      break;
    }
  }
}

int ArduinoIoTCloudTCP::onUpstreamMessage(ArduinoIoTCloudProcess::Event id)
{
  return ArduinoCloud.handleUpstreamMessage(id);
}

int ArduinoIoTCloudTCP::handleUpstreamMessage(ArduinoIoTCloudProcess::Event id)
{
  int ret = 1;
  switch (id)
  {
  case Event::SendCapabilities:
    sendDevicePropertiesToCloud();
  break;

  case Event::GetThingId:
    requestThingId();
  break;

  case Event::AttachThing:
    attachThing();
  break;

  case Event::RequestlastValues:
    requestLastValue();
  break;

  case Event::SendProperties:
    sendThingPropertiesToCloud();
  break;

  case Event::Disconnect:
    if (_state == State::Connected)
      _state = State::Disconnect;
  break;

  default:
  break;
  }

  return ret;
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

void ArduinoIoTCloudTCP::sendDevicePropertiesToCloud()
{
  PropertyContainer ro_device_container;
  unsigned int last_device_property_index = 0;

  std::list<String> ro_device_property_list {"LIB_VERSION", "OTA_CAP", "OTA_ERROR", "OTA_SHA256"};
  std::for_each(ro_device_property_list.begin(),
                ro_device_property_list.end(),
                [this, &ro_device_container ] (String const & name)
                {
                  Property* p = getProperty(this->_device.getPropertyContainer(), name);
                  if(p != nullptr)
                    addPropertyToContainer(ro_device_container, *p, p->name(), p->isWriteableByCloud() ? Permission::ReadWrite : Permission::Read);
                }
                );
  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s announce device to the Cloud %d", __FUNCTION__, _time_service.getTime());
  sendPropertyContainerToCloud(_deviceTopicOut, ro_device_container, last_device_property_index);

#if OTA_ENABLED
  OtaBeginUp command = {CommandID::OtaBeginUpId};
  memcpy(command.fields.params.sha, (uint8_t*)OTA::getImageSHA256().c_str(), SHA256_SIZE);
  sendMessage((Message*)&command);
#endif

  DeviceBeginCmdUp command2 = {CommandID::DeviceBeginCmdUpId};
  strcpy(command2.fields.params.lib_version, AIOT_CONFIG_LIB_VERSION);
  sendMessage((Message*)&command2);
}

#if OTA_ENABLED
void ArduinoIoTCloudTCP::sendDevicePropertyToCloud(String const name)
{
  PropertyContainer temp_device_container;
  unsigned int last_device_property_index = 0;

  Property* p = getProperty(this->_device.getPropertyContainer(), name);
  if(p != nullptr)
  {
    addPropertyToContainer(temp_device_container, *p, p->name(), p->isWriteableByCloud() ? Permission::ReadWrite : Permission::Read);
    sendPropertyContainerToCloud(_deviceTopicOut, temp_device_container, last_device_property_index);
  }
}
#endif // OTA_ENABLED

void ArduinoIoTCloudTCP::sendMessage(Message * msg)
{
  uint8_t data[MQTT_TRANSMIT_BUFFER_SIZE];
  int bytes_encoded = 0;
  CborError err = MessageEncoder::encode(msg, data, sizeof(data), bytes_encoded);
  write(_messageTopicOut, data, bytes_encoded);
}

void ArduinoIoTCloudTCP::requestLastValue()
{
  // Send the getLastValues CBOR message to the cloud
  // [{0: "r:m", 3: "getLastValues"}] = 81 A2 00 63 72 3A 6D 03 6D 67 65 74 4C 61 73 74 56 61 6C 75 65 73
  // Use http://cbor.me to easily generate CBOR encoding
  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s at time [%d]", __FUNCTION__, getTime());
  const uint8_t CBOR_REQUEST_LAST_VALUE_MSG[] = { 0x81, 0xA2, 0x00, 0x63, 0x72, 0x3A, 0x6D, 0x03, 0x6D, 0x67, 0x65, 0x74, 0x4C, 0x61, 0x73, 0x74, 0x56, 0x61, 0x6C, 0x75, 0x65, 0x73 };
  write(_shadowTopicOut, CBOR_REQUEST_LAST_VALUE_MSG, sizeof(CBOR_REQUEST_LAST_VALUE_MSG));

  ThingGetLastValueCmdUp command = {CommandID::ThingGetLastValueCmdUpId};
  sendMessage((Message*)&command);
}

void ArduinoIoTCloudTCP::requestThingId()
{
  if (!_mqttClient.subscribe(_messageTopicIn))
  {
    /* If device_id is wrong the board can't connect to the broker so this condition
    * should never happen.
    */
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _messageTopicIn.c_str());
  }

  if (!_mqttClient.subscribe(_deviceTopicIn))
  {
    /* If device_id is wrong the board can't connect to the broker so this condition
    * should never happen.
    */
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _deviceTopicIn.c_str());
  }

  ThingGetIdCmdUp command = {CommandID::ThingGetIdCmdUpId};
  strcpy(command.fields.params.thing_id, _thing_id.c_str());
  sendMessage((Message*)&command);
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

  _shadowTopicIn  = getTopic_shadowin();
  _shadowTopicOut = getTopic_shadowout();
  if (!_mqttClient.subscribe(_shadowTopicIn))
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _shadowTopicIn.c_str());
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
