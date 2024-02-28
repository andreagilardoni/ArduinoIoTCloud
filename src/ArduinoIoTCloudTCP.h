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

#ifndef ARDUINO_IOT_CLOUD_TCP_H
#define ARDUINO_IOT_CLOUD_TCP_H

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#include <ArduinoIoTCloud.h>

#ifdef BOARD_HAS_ECCX08
  #include "tls/BearSSLClient.h"
  #include "tls/utility/CryptoUtil.h"
#elif defined(BOARD_ESP)
  #include <WiFiClientSecure.h>
#elif defined(ARDUINO_UNOR4_WIFI)
  #include <WiFiSSLClient.h>
#elif defined(ARDUINO_PORTENTA_C33)
  #include "tls/utility/CryptoUtil.h"
  #include <SSLClient.h>
#elif defined(BOARD_HAS_SE050)
  #include "tls/utility/CryptoUtil.h"
  #include <WiFiSSLSE050Client.h>
#endif

#ifdef BOARD_HAS_OFFLOADED_ECCX08
#include "tls/utility/CryptoUtil.h"
#include <WiFiSSLClient.h>
#endif

#include <ArduinoMqttClient.h>

#include <ArduinoIoTCloudProperties.h>

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static char const DEFAULT_BROKER_ADDRESS_SECURE_AUTH[] = "mqtts-sa.iot.arduino.cc";
static uint16_t const DEFAULT_BROKER_PORT_SECURE_AUTH = 8883;
static char const DEFAULT_BROKER_ADDRESS_USER_PASS_AUTH[] = "mqtts-up.iot.arduino.cc";
static uint16_t const DEFAULT_BROKER_PORT_USER_PASS_AUTH = 8884;

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/

typedef bool (*onOTARequestCallbackFunc)(void);
typedef int (*onSendMessageUpstreamCallbackFunc)(ArduinoIoTCloudProcess::Event);

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class TimedAttempt
{
  public:
    TimedAttempt(unsigned long delay, unsigned long max_delay) { _delay = delay; _max_delay = max_delay; }

    void begin(unsigned long delay, unsigned long max_delay) { _retry_cnt = 0; _delay = delay; _max_delay = max_delay; }
    void begin(unsigned long delay) { _retry_cnt = 0; _delay = delay; _max_delay = delay; }
    bool isRetry() { return _retry_cnt > 0; }
    void reset() { _retry_cnt = 0; }
    bool isExpired() { return millis() > _next_retry_tick; }
    unsigned long reconfigure(unsigned long delay, unsigned long max_delay) { _delay = delay; _max_delay = max_delay; return reload(); }
    unsigned long retry() { _retry_cnt++; return reload(); }
    unsigned long reload() {
      unsigned long retry_delay = (1 << _retry_cnt) * _delay;
      retry_delay = min(retry_delay, _max_delay);
     _next_retry_tick = millis() + retry_delay;
     return retry_delay;
    }
    unsigned int getRetryCount() { return _retry_cnt; }

  private:
    unsigned long _delay;
    unsigned long _max_delay;
    unsigned long _next_retry_tick;
    unsigned int  _retry_cnt;

};

class ArduinoIoTCloudDevice: public ArduinoIoTCloudProcess, public ArduinoIoTCloudPropertiesClass
{
  public:

             ArduinoIoTCloudDevice();
    virtual ~ArduinoIoTCloudDevice() { }

    virtual void update        () override;
    virtual int  connected     () override;
    virtual void printDebugInfo() override;
    virtual void push          () override;

    void begin(onSendMessageUpstreamCallbackFunc cb);
    int sendMessageDownstream(ArduinoIoTCloudProcess::Event ev, const char* data = nullptr);
    inline bool attached() { return _attached; };

  private:

    enum class State
    {
      SendCapabilities,
      RequestThingId,
      ProcessThingId,
      AttachThing,
      Connected,
      Disconnect,
    };

    State _state;
    TimedAttempt _connection_attempt;
    String _thing_id;
    bool _attached;

    onSendMessageUpstreamCallbackFunc _send_message_upstream;
    int sendMessageUpstream(ArduinoIoTCloudProcess::Event id);

    State handle_SendCapabilities();
    State handle_RequestThingId();
    State handle_ProcessThingId();
    State handle_AttachThing();
    State handle_Connected();
    State handle_Disconnect();

#if OTA_ENABLED
    bool _ota_cap;
    int _ota_error;
    String _ota_img_sha256;
    String _ota_url;
    bool _ota_req;
#endif
};

class ArduinoIoTCloudThing: public ArduinoIoTCloudProcess , public ArduinoIoTCloudPropertiesClass
{
  public:

             ArduinoIoTCloudThing();
    virtual ~ArduinoIoTCloudThing() { }

    virtual void update        () override;
    virtual int  connected     () override;
    virtual void printDebugInfo() override;
    virtual void push          () override;

    void begin(onSendMessageUpstreamCallbackFunc cb);
    bool setTimestamp(String const & prop_name, unsigned long const timestamp);
    int sendMessageDownstream(ArduinoIoTCloudProcess::Event ev, const char* data = nullptr);

    int _tz_offset;
    Property * _tz_offset_property;
    unsigned int _tz_dst_until;
    Property * _tz_dst_until_property;

  private:

    enum class State
    {
      RequestLastValues, //Init
      Connected,         //Connected
      Disconnect,        //Error
    };

    State _state;
    TimedAttempt _connection_attempt;

    onSendMessageUpstreamCallbackFunc _send_message_upstream;
    int sendMessageUpstream(ArduinoIoTCloudProcess::Event id);

    State handle_RequestLastValues();
    State handle_Connected();
    State handle_Disconnect();
};


class ArduinoIoTCloudTCP: public ArduinoIoTCloudClass
{
  public:

             ArduinoIoTCloudTCP();
    virtual ~ArduinoIoTCloudTCP() { }

    virtual void update        () override;
    virtual int  connected     () override;
    virtual void printDebugInfo() override;
    virtual void push          () override;

    bool setTimestamp(String const & prop_name, unsigned long const timestamp);

    #if defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_OFFLOADED_ECCX08) || defined(BOARD_HAS_SE050)
    int begin(ConnectionHandler & connection, bool const enable_watchdog = true, String brokerAddress = DEFAULT_BROKER_ADDRESS_SECURE_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_SECURE_AUTH);
    #else
    int begin(ConnectionHandler & connection, bool const enable_watchdog = true, String brokerAddress = DEFAULT_BROKER_ADDRESS_USER_PASS_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_USER_PASS_AUTH);
    #endif
    int begin(bool const enable_watchdog = true, String brokerAddress = DEFAULT_BROKER_ADDRESS_SECURE_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_SECURE_AUTH);

    #ifdef BOARD_HAS_SECRET_KEY
    inline void setBoardId        (String const device_id) { setDeviceId(device_id); }
    inline void setSecretDeviceKey(String const password)  { _password = password;  }
    #endif

    inline void     setThingId (String const thing_id)  { _thing_id = thing_id; };
    inline String & getThingId ()                       { return _thing_id; };
    inline void     setDeviceId(String const device_id) { _device_id = device_id; };
    inline String & getDeviceId()                       { return _device_id; };

    inline ArduinoIoTCloudThing &getThing() { return _thing; }

    inline String   getBrokerAddress() const { return _brokerAddress; }
    inline uint16_t getBrokerPort   () const { return _brokerPort; }

#if OTA_ENABLED
    /* The callback is triggered when the OTA is initiated and it gets executed until _ota_req flag is cleared.
     * It should return true when the OTA can be applied or false otherwise.
     * See example ArduinoIoTCloud-DeferredOTA.ino
     */
    void onOTARequestCb(onOTARequestCallbackFunc cb) {
      _get_ota_confirmation = cb;
      _ask_user_before_executing_ota = true;
    }
#endif

  private:
    static const int MQTT_TRANSMIT_BUFFER_SIZE = 256;

    enum class State
    {
      ConnectPhy,
      SyncTime,
      ConnectMqttBroker,
      Connected,
      Disconnect,
    };

    State _state;

    String _device_id;
    String _thing_id;
    Property * _thing_id_property;

    TimedAttempt _connection_attempt;
    String _brokerAddress;
    uint16_t _brokerPort;
    uint8_t _mqtt_data_buf[MQTT_TRANSMIT_BUFFER_SIZE];
    int _mqtt_data_len;
    bool _mqtt_data_request_retransmit;

    #if defined(BOARD_HAS_ECCX08)
    ArduinoIoTCloudCertClass _cert;
    BearSSLClient _sslClient;
    CryptoUtil _crypto;
    #elif defined(BOARD_HAS_OFFLOADED_ECCX08)
    ArduinoIoTCloudCertClass _cert;
    WiFiBearSSLClient _sslClient;
    CryptoUtil _crypto;
    #elif defined(BOARD_ESP)
    WiFiClientSecure _sslClient;
    #elif defined(ARDUINO_UNOR4_WIFI)
    WiFiSSLClient _sslClient;
    #elif defined(ARDUINO_EDGE_CONTROL)
    GSMSSLClient _sslClient;
    #elif defined(ARDUINO_PORTENTA_C33)
    ArduinoIoTCloudCertClass _cert;
    SSLClient _sslClient;
    CryptoUtil _crypto;
    #elif defined(BOARD_HAS_SE050)
    ArduinoIoTCloudCertClass _cert;
    WiFiSSLSE050Client _sslClient;
    CryptoUtil _crypto;
    #endif

    #if defined (BOARD_HAS_SECRET_KEY)
    String _password;
    #endif

    MqttClient _mqttClient;

    String _deviceTopicOut;
    String _deviceTopicIn;
    String _shadowTopicOut;
    String _shadowTopicIn;
    String _dataTopicOut;
    String _dataTopicIn;

    ArduinoIoTCloudThing _thing;
    ArduinoIoTCloudDevice _device;

#if OTA_ENABLED
    bool _ask_user_before_executing_ota;
    onOTARequestCallbackFunc _get_ota_confirmation;
#endif /* OTA_ENABLED */

    inline String getTopic_deviceout() { return String("/a/d/" + getDeviceId() + "/e/o");}
    inline String getTopic_devicein () { return String("/a/d/" + getDeviceId() + "/e/i");}
    inline String getTopic_shadowout() { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/shadow/o"); }
    inline String getTopic_shadowin () { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/shadow/i"); }
    inline String getTopic_dataout  () { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/e/o"); }
    inline String getTopic_datain   () { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/e/i"); }

    State handle_ConnectPhy();
    State handle_SyncTime();
    State handle_ConnectMqttBroker();
    State handle_Connected();
    State handle_Disconnect();

    static void onDownstreamMessage(int length);
    void handleDownstreamMessage(int length);
    static int onUpstreamMessage(ArduinoIoTCloudProcess::Event id);
    int handleUpstreamMessage(ArduinoIoTCloudProcess::Event id);

    void sendPropertyContainerToCloud(String const topic, PropertyContainer & property_container, unsigned int & current_property_index);
    void sendThingPropertiesToCloud();
    void sendDevicePropertiesToCloud();
    void requestLastValue();
    void requestThingId();
    void attachThing();
    int write(String const topic, byte const data[], int const length);

#if OTA_ENABLED
    void sendDevicePropertyToCloud(String const name);
#endif

};

/******************************************************************************
 * EXTERN DECLARATION
 ******************************************************************************/

extern ArduinoIoTCloudTCP ArduinoCloud;

#endif
