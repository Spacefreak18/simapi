#ifndef _LOOPDATA_H
#define _LOOPDATA_H

#include <uv.h>
#include <simdata.h>
#include <simmapper.h>

typedef struct
{
    char* Name;
    int GameId;
    bool UseUDP;
    char* LaunchExe;
    char* LiveExe;
    int BridgeDelay;
    int simapi;
}
GameCompatInfo;

typedef struct
{
    bool force_udp;
    bool auto_memmap;
    bool auto_bridge;
    bool daemon;
    bool bridge_path_set;
    char* home_dir;
    char* configfile;
}
SimdSettings;

typedef struct
{
    uv_work_t req;
    SimulatorAPI sim;
    bool use_udp;
    bool simstate;
    bool uion;
    bool releasing;
    SimdSettings simds;
    int compat_info_size;
    GameCompatInfo* game_compat_info;
    SimData* simdata;
    SimMap* simmap;
    SimMap* simmap2;
}
LoopData;

#endif
