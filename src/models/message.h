#pragma once
#include <stdint.h>

enum CommandID: uint8_t {
    GetLastValueCommandId,
    SetLastValueCommandId,
    GetTimezoneCommandId,
    SetTimezoneCommandId,
    OTABeginId,
    OTAavailableId,
    OTAStatusReportId
};

struct Command {
    CommandID id;
};

typedef Command Message;

struct GenericCommand {
    Command command;
    uint8_t content[];
};

struct LastValueCommand {
    Command command;
};

union GetLastValueCommand {
    struct {
        LastValueCommand lastValueCommand;
        struct params {
            // TODO fill in params
        };
    } fields;
    uint8_t buf[sizeof(fields)];
};

union SetLastValueCommand {
    struct {
        LastValueCommand lastValueCommand;
        struct params {
            // TODO fill in params
        };
    } fields;
    uint8_t buf[sizeof(fields)];
};

struct TimezoneCommand {
    Command command;
};

union GetTimezoneCommand {
    struct {
        TimezoneCommand timezoneCommand;
        struct params {
            // TODO fill in params
        };
    } fields;
    uint8_t buf[sizeof(fields)];
};

union SetTimezoneCommand {
    struct {
        TimezoneCommand timezoneCommand;
        struct params {
            // TODO fill in params
        };
    } fields;
    uint8_t buf[sizeof(fields)];
};

struct OTACommand {
    Command command;
};

union OTAavailable {
    struct {
        OTACommand otaCommand;
        struct {
            char    id[10];
            char    url[128];
            uint8_t initialSha256[32];
            uint8_t finalSha256[32];
        } params;
    } fields;
    uint8_t buf[sizeof(fields)];
};

union OTABegin {
    struct {
        OTACommand otaCommand;
        struct {
            uint8_t sha256[32];
        } params;
    } fields;
    uint8_t buf[sizeof(fields)];
};

union OTAStatusReport {
    struct {
        OTACommand otaCommand;
        struct {
            const char* id;
            const char* state; // TODO make it an enum
            uint32_t    time;
            uint32_t    count;
        } params;
    } fields;
    uint8_t buf[sizeof(fields)];
};