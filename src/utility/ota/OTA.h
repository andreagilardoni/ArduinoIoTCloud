#pragma once
#include "AIoTC_Config.h"
#if OTA_ENABLED

#ifdef ARDUINO_ARCH_SAMD
#include "OTASamd.h"
using OTACloudProcess = SAMDOTACloudProcess;

#elif defined(ARDUINO_NANO_RP2040_CONNECT)
#include "OTANanoRP2040.h"
using OTACloudProcess = NANO_RP2040OTACloudProcess;

#elif defined(BOARD_STM32H7)
#include "OTAPortentaH7.h"
using OTACloudProcess = STM32H7OTACloudProcess;

#elif defined(ARDUINO_ARCH_ESP32)
#include "OTAEsp32.h"
using OTACloudProcess = ESP32OTACloudProcess;

#elif defined(ARDUINO_UNOR4_WIFI)
#include "OTAUnoR4.h"
using OTACloudProcess = UNOR4OTACloudProcess;

#else
#error "This Board doesn't support OTA"
#endif

#define RP2040_OTA_ERROR_BASE (-100)

enum class OTAError : int
{
  None           = 0,
  DownloadFailed = 1,
  RP2040_UrlParseError        = RP2040_OTA_ERROR_BASE - 0,
  RP2040_ServerConnectError   = RP2040_OTA_ERROR_BASE - 1,
  RP2040_HttpHeaderError      = RP2040_OTA_ERROR_BASE - 2,
  RP2040_HttpDataError        = RP2040_OTA_ERROR_BASE - 3,
  RP2040_ErrorOpenUpdateFile  = RP2040_OTA_ERROR_BASE - 4,
  RP2040_ErrorWriteUpdateFile = RP2040_OTA_ERROR_BASE - 5,
  RP2040_ErrorParseHttpHeader = RP2040_OTA_ERROR_BASE - 6,
  RP2040_ErrorFlashInit       = RP2040_OTA_ERROR_BASE - 7,
  RP2040_ErrorReformat        = RP2040_OTA_ERROR_BASE - 8,
  RP2040_ErrorUnmount         = RP2040_OTA_ERROR_BASE - 9,
};

#endif // OTA_ENABLED
