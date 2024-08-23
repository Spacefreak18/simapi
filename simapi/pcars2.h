#ifndef _PCARS2_H
#define _PCARS2_H

#include <stdbool.h>
#include "../include/pcars2data.h"

#define AMS2_EXE "ams2.exe"

#define PCARS2_FILE "$pcars2$"
#define PCARS2_FILE_LINUX "$pcars2"

typedef struct
{
    bool has_telemetry;
    void* telemetry_map_addr;
    int fd_telemetry;
    struct pcars2APIStruct pcars2_telemetry;
}
PCars2Map;

#endif
