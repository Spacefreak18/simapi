#ifndef _SCS2_H
#define _SCS2_H

#include <stdbool.h>
#include "../include/scs2data.h"

#define EUROTRUCKS2_EXE "eurotrucks2"
#define AMERICANTRUCKS_EXE "amtrucks"

#define SCS2_TELEMETRY_FILE "SCS/SCSTelemetry"

typedef struct
{
    bool has_telemetry;
    void* telemetry_map_addr;
    int fd_telemetry;
    struct scs2TelemetryMap_s scs2_telemetry;
}
SCS2Map;

#endif
