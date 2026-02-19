#ifndef _R3EDEF_H
#define _R3EDEF_H

#include <stdbool.h>
#include "../include/r3e.h"

#define RACE_ROOM_EXE "RRRE64.exe"

#define R3E_FILE "$R3E"

typedef struct
{
    bool has_telemetry;
    void* telemetry_map_addr;
    int fd_telemetry;
    struct r3e_share r3e_telemetry;
}
R3EMap;

#endif
