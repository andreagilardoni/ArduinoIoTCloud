#pragma once
#include <stdint.h>

#if __cplusplus >= 201402L // Cpp14
#include <algorithm>
#endif // __cplusplus

#define THING_ID_SIZE 37
#define SHA256_SIZE 32
#define URL_SIZE 256
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

struct ThingGetIdCmdUp {
    Command c;
    struct {
        char thing_id[THING_ID_SIZE];
    } params;
};

struct ThingGetLastValueCmdUp {
    Command c;
};

struct OtaBeginUp {
    Command c;
    struct {
        uint8_t sha [SHA256_SIZE];
    } params;
};

struct DeviceBeginCmdUp {
    Command c;
    struct {
        char lib_version[MAX_LIB_VERSION_SIZE];
    } params;
};

struct OtaProgressCmdUp {
    Command c;
    struct {
        char id[ID_SIZE];
        char state[MAX_STATE_SIZE];
        uint32_t time;
        uint32_t count;
    } params;
};

struct TimezoneCommandUp {
    Command c;
};

struct __attribute__((__packed__)) OtaUpdateCmdDown {
    Command c;
    struct {
        char    id[ID_SIZE];
        char    url[URL_SIZE];
        uint8_t initialSha256[SHA256_SIZE];
        uint8_t finalSha256[SHA256_SIZE];
    } params;
};

struct __attribute__((__packed__)) ThingGetIdCmdDown {
    Command c;
    struct {
        char thing_id[THING_ID_SIZE];
    } params;
};

struct __attribute__((__packed__)) ThingGetLastValueCmdDown {
    Command c;
    struct {
        uint8_t * last_values;
        size_t length;
    } params;
};

struct __attribute__((__packed__)) TimezoneCommandDown{
    Command c;
    struct {
        uint32_t offset;
        uint32_t until;
    } params;
};

union CommandDown {
    struct Command                  c;
    struct OtaUpdateCmdDown         otaUpdateCmdDown;
    struct ThingGetIdCmdDown        thingGetIdCmdDown;
    struct ThingGetLastValueCmdDown thingGetLastValueCmdDown;
    struct TimezoneCommandDown      timezoneCommandDown;
    uint8_t buf[
#if __cplusplus >= 201402L // Cpp14
        std::max({sizeof(otaUpdateCmdDown),
            sizeof(thingGetIdCmdDown),
            sizeof(thingGetLastValueCmdDown),
            sizeof(timezoneCommandDown)})
#else
        /* for versions of cpp prior to cpp14 std::max is not defined as constexpr,
         * hence it cannot be used here thus we need to specify the size of the
         * biggest member manually.
         */
        sizeof(otaUpdateCmdDown)
#endif // __cplusplus
        ];
};