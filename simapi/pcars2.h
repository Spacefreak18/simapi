#ifndef _RF2_H
#define _RF2_H

#include <stdbool.h>
#include "../include/pcars2data.h"

#define PCARS2_FILE "$pcars2$"

typedef struct
{
    bool has_telemetry;
    void* telemetry_map_addr;
    void* scoring_map_addr;
    struct pcars2APIStruct pcars2_telemetry;
}
PCars2Map;

#endif
