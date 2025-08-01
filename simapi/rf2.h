#ifndef _RF2_H
#define _RF2_H

#include <stdbool.h>
#include "../include/rf2data.h"

#define RFACTOR2_EXE "rFactor2.exe"
#define LEMANS_ULTIMATE_EXE "Le Mans Ultimate.exe"

#define RF2_TELEMETRY_FILE "$rFactor2SMMP_Telemetry$"
#define RF2_SCORING_FILE "$rFactor2SMMP_Scoring$"

typedef struct
{
    bool has_telemetry;
    bool has_scoring;
    void* telemetry_map_addr;
    void* scoring_map_addr;
    int fd_telemetry;
    int fd_scoring;
    struct rF2Telemetry rf2_telemetry;
    struct rF2Scoring rf2_scoring;
    char car[64];
    char driver[33];
    char track[64];
    char compound[18];
}
RF2Map;

#endif
