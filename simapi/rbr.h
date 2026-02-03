#ifndef _RBR_H
#define _RBR_H

#include <stdbool.h>
#include "../include/rbrdata.h"

#define RICHARD_BURNS_RALLY_EXE "RichardBurnsRally_SSE.exe"

typedef struct
{
    bool has_telemetry;
    RBR_TelemetryData rbr_telemetry;
}
RBRMap;

#endif
