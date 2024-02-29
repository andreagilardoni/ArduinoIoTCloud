#include <AIoTC_Config.h>

#if defined(ARDUINO_UNOR4_WIFI) && OTA_ENABLED
#include "OTAUnoR4.h"

#include <Arduino_DebugUtils.h>
#include "tls/utility/SHA256.h"
#include "fsp_common_api.h"
#include "r_flash_lp.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

/* Key code for writing PRCR register. */
#define BSP_PRV_PRCR_KEY                (0xA500U)
#define BSP_PRV_PRCR_PRC1_UNLOCK        ((BSP_PRV_PRCR_KEY) | 0x2U)
#define BSP_PRV_PRCR_LOCK               ((BSP_PRV_PRCR_KEY) | 0x0U)

#define OTA_MAGIC (*((volatile uint16_t *) &R_SYSTEM->VBTBKR[4]))
#define OTA_SIZE  (*((volatile uint32_t *) &R_SYSTEM->VBTBKR[6]))

static void unor4_setOTASize(uint32_t size){
  R_SYSTEM->PRCR = (uint16_t) BSP_PRV_PRCR_PRC1_UNLOCK;
  OTA_MAGIC = 0x07AA;
  OTA_SIZE = size;
  R_SYSTEM->PRCR = (uint16_t) BSP_PRV_PRCR_LOCK;
}

static uint32_t unor4_getOTASize() {
  if (OTA_MAGIC == 0x07AA) {
    return OTA_SIZE;
  }
  return 0;
}

static int unor4_codeFlashOpen(flash_lp_instance_ctrl_t * ctrl) {
  flash_cfg_t                  cfg;

  cfg.data_flash_bgo         = false;
  cfg.p_callback             = nullptr;
  cfg.p_context              = nullptr;
  cfg.p_extend               = nullptr;
  cfg.ipl                    = (BSP_IRQ_DISABLED);
  cfg.irq                    = FSP_INVALID_VECTOR;
  cfg.err_ipl                = (BSP_IRQ_DISABLED);
  cfg.err_irq                = FSP_INVALID_VECTOR;

  fsp_err_t rv = FSP_ERR_UNSUPPORTED;

  rv = R_FLASH_LP_Open(ctrl,&cfg);
  return rv;
}

static int unor4_codeFlashClose(flash_lp_instance_ctrl_t * ctrl) {
  fsp_err_t rv = FSP_ERR_UNSUPPORTED;

  rv = R_FLASH_LP_Close(ctrl);
  return rv;
}

const char UNOR4OTACloudProcess::UPDATE_FILE_NAME[] = "/update.bin";

UNOR4OTACloudProcess::UNOR4OTACloudProcess(MessageStream *ms, ConnectionHandler* connection_handler)
: OTACloudProcessInterface(ms, connection_handler){

}

OTACloudProcessInterface::State UNOR4OTACloudProcess::resume(Message* msg) {
  return OtaBegin;
}

OTACloudProcessInterface::State UNOR4OTACloudProcess::startOTA() {
  OTAUpdate::Error ota_err = OTAUpdate::Error::None;

  // Open fs for ota
  if((ota_err = ota.begin(UPDATE_FILE_NAME)) != OTAUpdate::Error::None) {
    DEBUG_ERROR("OTAUpdate::begin() failed with %d", static_cast<int>(ota_err));
    return OtaDownloadFail; // FIXME
  }

  return Fetch;
}

OTACloudProcessInterface::State UNOR4OTACloudProcess::fetch() {
  OTAUpdate::Error ota_err = OTAUpdate::Error::None;

  int const ota_download = ota.download(this->context->url,UPDATE_FILE_NAME);
  if (ota_download <= 0) {
    DEBUG_ERROR("OTAUpdate::download() failed with %d", ota_download);
    return OtaDownloadFail;
  }
  DEBUG_VERBOSE("OTAUpdate::download() %d bytes downloaded", static_cast<int>(ota_download));

  if ((ota_err = ota.verify()) != OTAUpdate::Error::None) {
    DEBUG_ERROR("OTAUpdate::verify() failed with %d", static_cast<int>(ota_err));
    return OtaHeaderCrcFail; // FIXME
  }

  /* Store update size and write OTA maginc number */
  unor4_setOTASize(ota_download);

  return FlashOTA;
}

OTACloudProcessInterface::State UNOR4OTACloudProcess::flashOTA() {
  OTAUpdate::Error ota_err = OTAUpdate::Error::None;

  /* Flash new firmware */
  if ((ota_err = ota.update(UPDATE_FILE_NAME)) != OTAUpdate::Error::None) { // This reboots the MCU
    DEBUG_ERROR("OTAUpdate::update() failed with %d", static_cast<int>(ota_err));
    return OtaDownloadFail; // FIXME
  }
}

OTACloudProcessInterface::State UNOR4OTACloudProcess::reboot() {
}

void UNOR4OTACloudProcess::reset() {
}

bool UNOR4OTACloudProcess::isOtaCapable() {
  String const fv = WiFi.firmwareVersion();
  if (fv < String("0.3.0")) {
    return false;
  }
  return true;
}


#endif // defined(ARDUINO_UNOR4_WIFI) && OTA_ENABLED
