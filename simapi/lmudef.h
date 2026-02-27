#ifndef _LMUDEF_H
#define _LMUDEF_H

#include <stdbool.h>
#include "../include/lmu.h"


#define LMU_FILE "LMU_Data"

typedef struct
{
    bool has_telemetry;
    void* telemetry_map_addr;
    int fd_telemetry;
    struct LMUObject lmu_telemetry;
}
LMUMap;

#endif
