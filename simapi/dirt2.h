#ifndef _DIRT2_H
#define _DIRT2_H

#include <stdbool.h>
#include "../include/dirt2data.h"

#define DIRT_RALLY_2_EXE "dirtrally2.exe"

typedef struct
{
    bool has_telemetry;
    struct dirt2_udp_packet dirt2_telemetry;
    float last_velX;
    float last_velY;
    float last_velZ;
    float last_runTime;
    bool has_last_vel;
}
Dirt2Map;

#endif

