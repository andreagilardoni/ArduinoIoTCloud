#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <GenericConnectionHandler.h>
#include <ConfiguratorAgents/AgentsConfiguratorManager.h>
#include <ConfiguratorAgents/agents/BLE/BLEConfiguratorAgent.h>
#include "arduino_secrets.h"

#if !(defined(HAS_TCP) || defined(HAS_LORA))
  #error  "Please check Arduino IoT Cloud supported boards list: https://github.com/arduino-libraries/ArduinoIoTCloud/#what"
#endif

#if defined(BOARD_HAS_SECRET_KEY)
  #define BOARD_ID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
#endif

#if defined(HAS_LORA)
  #define THING_ID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
#endif

void onLedChange();

bool led;
int potentiometer;
int seconds;

void initProperties() {
#if defined(BOARD_HAS_SECRET_KEY)
  ArduinoCloud.setBoardId(BOARD_ID);
  ArduinoCloud.setSecretDeviceKey(SECRET_DEVICE_KEY);
#endif
#if defined(HAS_TCP)
  ArduinoCloud.addProperty(led, Permission::Write).onUpdate(onLedChange);
  ArduinoCloud.addProperty(potentiometer, Permission::Read).publishOnChange(10);
  ArduinoCloud.addProperty(seconds, Permission::Read).publishOnChange(1);
  ConfiguratorManager.addAgent(BLEAgent);
#elif defined(HAS_LORA)
  ArduinoCloud.addProperty(led, 1, Permission::ReadWrite).onUpdate(onLedChange);
  ArduinoCloud.addProperty(potentiometer, 2, Permission::Read).publishOnChange(10);
  ArduinoCloud.addProperty(seconds, 3, Permission::Read).publishEvery(5 * MINUTES);

  ArduinoCloud.setThingId(THING_ID);
#endif
}

GenericConnectionHandler ArduinoIoTPreferredConnection;

NetworkConfigurator NetworkConf(ConfiguratorManager, ArduinoIoTPreferredConnection);
