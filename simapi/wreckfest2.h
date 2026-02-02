#ifndef _WRECKFEST2_H
#define _WRECKFEST2_H

#include <stdbool.h>
#include <stdint.h>
#include "../include/wreckfest2data.h"

#define WRECKFEST2_EXE "Wreckfest2.exe"

typedef struct
{
    bool has_main_packet;
    bool has_leaderboard_packet;
    bool has_timing_packet;

    // Store last received packets for multi-packet sync
    WF2_PacketMain main_packet;

    int32_t last_session_time; // For packet synchronization
} Wreckfest2Map;

#endif
