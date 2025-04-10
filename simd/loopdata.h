#ifndef _LOOPDATA_H
#define _LOOPDATA_H

#include <uv.h>
#include <simdata.h>
#include <simmapper.h>

typedef struct simd_settings
{
  bool force_udp;
  bool auto_memmap;
  bool daemon;
} simd_settings;

typedef struct loop_data
{
    uv_work_t req;
    SimulatorAPI sim;
    bool use_udp;
    bool simstate;
    bool uion;
    bool releasing;
    int numdevices;
    SimData* simdata;
    SimMap* simmap;
    SimMap* simmap2;
} loop_data;

#endif
