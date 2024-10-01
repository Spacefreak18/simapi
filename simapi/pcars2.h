#ifndef _PCARS2_H
#define _PCARS2_H

#include <stdbool.h>
#include "../include/pcars2data.h"
#include "../include/ams2udpdata.h"

#define AMS2_EXE "AMS2AVX.exe"

#define PCARS2_FILE "$pcars2$"
#define PCARS2_FILE_LINUX "$pcars2"

#define AMS2_MAX_UDP_PACKET_SIZE 1452

typedef struct
{
    bool has_telemetry;
    void* telemetry_map_addr;
    int fd_telemetry;
    struct pcars2APIStruct pcars2_telemetry;
    struct ams2UDPData pcars2_udp_telemetry;
}
PCars2Map;

#endif
