#ifndef _SIMDATA_H
#define _SIMDATA_H

#include <stdint.h>

#define SIMMAP_ALL 1
#ifndef SIMMAP_ALL

typedef struct
{
    uint32_t velocity;
    uint32_t rpms;
    uint32_t gear;
    uint32_t pulses;
    uint32_t maxrpm;
    uint32_t altitude;
    uint32_t lap;
    uint32_t position;
}
SimData;

#endif
#ifdef SIMMAP_ALL

typedef struct
{
    uint32_t velocity;
    uint32_t rpms;
    uint32_t gear;
    uint32_t pulses;
    uint32_t maxrpm;
    uint32_t altitude;
    uint32_t lap;
    uint32_t position;
    uint32_t numlaps;
    double gas;
    double brake;
    double fuel;

    double braketemp[4];
    double tyrewear[4];
    double tyretemp[4];
    double tyrepressure[4];

    double airdensity;
    double airtemp;
    double tracktemp;

    uint32_t lastlap;
    uint32_t bestlap;
    uint32_t time;
    double timeleft;

    char* car;
    char* track;
    char* driver;
}
SimData;

#endif

#endif
