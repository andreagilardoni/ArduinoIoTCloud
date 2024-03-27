#pragma once
#include <message/Models.h>

enum CBORCommandTag: uint64_t {
  // Commands UP
  CBOROtaBeginUp               = 0x10000,
  CBORThingGetIdCmdUp          = 0x10300,
  CBORThingGetLastValueCmdUp   = 0x10500,
  CBORDeviceBeginCmdUp         = 0x10700,
  CBOROtaProgressCmdUp         = 0x10200,
  CBORTimezoneCommandUp        = 0x10800,

  // Commands DOWN
  CBOROtaUpdateCmdDown         = 0x10100,
  CBORThingGetIdCmdDown        = 0x10400,
  CBORThingGetLastValueCmdDown = 0x10600,
  CBORTimezoneCommandDown      = 0x10900,

  // Unknown Command Tag https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml
  CBORUnknownCmdTag16b             = 0xffff,              // invalid tag
  CBORUnknownCmdTag32b             = 0xffffffff,          // invalid tag
  CBORUnknownCmdTag64b             = 0xffffffffffffffff,  // invalid tag
  CBORUnknownCmdTag                = CBORUnknownCmdTag32b
};

CommandID toCommandId(CBORCommandTag tag);
CBORCommandTag toCBORCommandTag(CommandID id);