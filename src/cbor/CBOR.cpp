#include "CBOR.h"

CommandID toCommandId(CBORCommandTag tag) {
  switch(tag) {
  case CBORCommandTag::CBOROtaBeginUp:
    return CommandID::OtaBeginUpId;
  case CBORCommandTag::CBORThingGetIdCmdUp:
    return CommandID::ThingGetIdCmdUpId;
  case CBORCommandTag::CBORThingGetLastValueCmdUp:
    return CommandID::ThingGetLastValueCmdUpId;
  case CBORCommandTag::CBORDeviceBeginCmdUp:
    return CommandID::DeviceBeginCmdUpId;
  case CBORCommandTag::CBOROtaProgressCmdUp:
    return CommandID::OtaProgressCmdUpId;
  case CBORCommandTag::CBORTimezoneCommandUp:
    return CommandID::TimezoneCommandUpId;
  case CBORCommandTag::CBOROtaUpdateCmdDown:
    return CommandID::OtaUpdateCmdDownId;
  case CBORCommandTag::CBORThingGetIdCmdDown:
    return CommandID::ThingGetIdCmdDownId;
  case CBORCommandTag::CBORThingGetLastValueCmdDown:
    return CommandID::ThingGetLastValueCmdDownId;
  case CBORCommandTag::CBORTimezoneCommandDown:
    return CommandID::TimezoneCommandDownId;
  default:
    return CommandID::UnknownCmdId;
  }
}


CBORCommandTag toCBORCommandTag(CommandID id) {
  switch(id) {
  case CommandID::OtaBeginUpId:
    return CBORCommandTag::CBOROtaBeginUp;
  case CommandID::ThingGetIdCmdUpId:
    return CBORCommandTag::CBORThingGetIdCmdUp;
  case CommandID::ThingGetLastValueCmdUpId:
    return CBORCommandTag::CBORThingGetLastValueCmdUp;
  case CommandID::DeviceBeginCmdUpId:
    return CBORCommandTag::CBORDeviceBeginCmdUp;
  case CommandID::OtaProgressCmdUpId:
    return CBORCommandTag::CBOROtaProgressCmdUp;
  case CommandID::TimezoneCommandUpId:
    return CBORCommandTag::CBORTimezoneCommandUp;
  case CommandID::OtaUpdateCmdDownId:
    return CBORCommandTag::CBOROtaUpdateCmdDown;
  case CommandID::ThingGetIdCmdDownId:
    return CBORCommandTag::CBORThingGetIdCmdDown;
  case CommandID::ThingGetLastValueCmdDownId:
    return CBORCommandTag::CBORThingGetLastValueCmdDown;
  case CommandID::TimezoneCommandDownId:
    return CBORCommandTag::CBORTimezoneCommandDown;
  default:
    return CBORCommandTag::CBORUnknownCmdTag;
  }
}