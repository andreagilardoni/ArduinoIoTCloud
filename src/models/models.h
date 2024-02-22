#pragma once
#include <stdint.h>

#define MAX_PAYLOAD_SIZE 256
#define THING_ID_SIZE 37
#define SHA256_SIZE 32
#define URL_SIZE 64
#define MAX_STATE_SIZE 32
#define ID_SIZE 32
#define MAX_LIB_VERSION_SIZE 10

enum CommandID: uint32_t {
    // Commands UP
    OtaBeginUpId = 65536,
    ThingGetIdCmdUpId = 66304,
    ThingGetLastValueCmdUpId = 66816,
    DeviceBeginCmdUpId = 67328,
    OtaProgressCmdUpId = 66048,
    TimezoneCommandUpId = 67584,

    // Commands DOWN
    OtaUpdateCmdDownId = 65792,
    ThingGetIdCmdDownId = 66560,
    ThingGetLastValueCmdDownId = 67072,
    TimezoneCommandDownId = 67840
};

struct Command {
    CommandID id;
};

typedef Command Message;

struct GenericCommand {
    Command command;
    uint8_t content[MAX_PAYLOAD_SIZE];
};

struct ThingGetIdCmdUpCommand {
    Command command;
};

union ThingGetIdCmdUp {
    struct {
        ThingGetIdCmdUpCommand thingGetIdCmdUpCommand;
        struct {
            char thing_id[THING_ID_SIZE];
        } params;
    } fields;
};

struct ThingGetLastValueCmdUpCommand {
    Command command;
};

union ThingGetLastValueCmdUp {
    struct {
        ThingGetLastValueCmdUpCommand thingGetLastValueCmdUpCommand;
    } fields;
};

struct OtaBeginUpCommand {
    Command command;
};

union OtaBeginUp {
    struct {
        OtaBeginUpCommand otaBeginUpCommand;
        struct {
          uint8_t sha [SHA256_SIZE];
        } params;
    } fields;
};

struct DeviceBeginCmdUpCommand {
    Command command;
};

union DeviceBeginCmdUp {
    struct {
        DeviceBeginCmdUpCommand deviceBeginCmdUpCommand;
        struct {
          char lib_version[MAX_LIB_VERSION_SIZE];
        } params;
    } fields;
};

struct OtaProgressCmdUpCommand {
    Command command;
};

union OtaProgressCmdUp {
    struct {
        OtaProgressCmdUpCommand otaProgressCmdUpCommand;
        struct {
            char id[ID_SIZE];
            char state[MAX_STATE_SIZE];
            uint32_t time;
            uint32_t count;
        } params;
    } fields;
};

struct OtaUpdateCmdDownCommand {
    Command command;
};

union OtaUpdateCmdDown {
    struct {
        OtaUpdateCmdDownCommand otaUpdateCmdDownCommand;
        struct {
            char    id[ID_SIZE];
            char    url[URL_SIZE];
            uint8_t initialSha256[SHA256_SIZE];
            uint8_t finalSha256[SHA256_SIZE];
        } params;
    } fields;
};

struct ThingGetIdCmdDownCommand {
    Command command; 
};

union ThingGetIdCmdDown {
    struct {
        ThingGetIdCmdDownCommand thingGetIdCmdDownCommand;
        struct {
            char thing_id[THING_ID_SIZE];
        } params;
    } fields;
};

struct ThingGetLastValueCmdDownCommand {
    Command command; 
};

union ThingGetLastValueCmdDown {
    struct {
        ThingGetLastValueCmdDownCommand thingGetLastValueCmdDownCommand;
        struct {
            uint8_t * last_values;
            size_t length;
        } params;
    } fields;
};

struct TimezoneCommandUpCommand {
    Command command;
};

union TimezoneCommandUp {
    struct {
        TimezoneCommandUpCommand timezoneCommandUp;
    } fields;
};

struct TimezoneCommandDownCommand {
    Command command;
};

union TimezoneCommandDown {
    struct {
        TimezoneCommandDownCommand timezoneCommandDown;
        struct {
            uint32_t offset;
            uint32_t until;
        } params;
    } fields;
};