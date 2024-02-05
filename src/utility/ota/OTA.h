/*
   This file is part of ArduinoIoTCloud.

   Copyright 2020 ARDUINO SA (http://www.arduino.cc/)

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

#ifndef ARDUINO_OTA_LOGIC_H_
#define ARDUINO_OTA_LOGIC_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#if OTA_ENABLED
#include <Arduino.h>
#include <Arduino_ConnectionHandler.h>
#include <interfaces/CloudProcess.h>

/******************************************************************************
 * DEFINES
 ******************************************************************************/

#define RP2040_OTA_ERROR_BASE (-100)

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/

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

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class OTACloudProcess: public CloudProcess {
public:
    enum State {

    };
    virtual void handleMessage(Message*);
    // virtual CloudProcess::State getState();
    // virtual void hook(State s, void* action);
    virtual void update();

protected:
    // The following methods represent the FSM actions performed in each state

    // the first state is 'resume', where we try to understand if we are booting after a ota
    // the objective is to understand the result and report it to the cloud
    virtual State resume() = 0;

    // this state is the normal state where no action has to be performed. We may poll the cloud
    // for updates in this state
    virtual State idle();

    // we go in this state if there is an ota available, depending on the policies implemented we may
    // start the ota or wait for an user interaction
    virtual State otaAvailable();

    // we start the process of ota update and wait for the server to respond with the ota url and other info
    virtual State startOTA();

    // we start the download and decompress process
    virtual State fetch() = 0;

    // when the download is completed we verify for integrity and correctness of the downloaded binary
    virtual State verifyOTA(); // TODO this may be performed inside download

    // whene the download is correctly finished we set the mcu to use the newly downloaded binary
    virtual State flashOTA() = 0;

    // we reboot the device
    virtual State reboot() = 0;

    // if any of the steps described fail we get into this state and report to the cloud what happened
    // then we go back to idle state
    virtual State fail();
};


#endif /* OTA_ENABLED */

#endif /* ARDUINO_OTA_LOGIC_H_ */
