#ifndef _SIMDATA_H
#define _SIMDATA_H

#include <stdint.h>

#define MAXCARS 24

#ifndef SIMMAP_ALL

typedef struct
{
    uint32_t simstatus;
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

#else

typedef struct
{
    uint32_t xpos;
    uint32_t ypos;
    uint32_t zpos;
    uint32_t pos;
    uint32_t lap;
    uint32_t lastlap;
    uint32_t bestlap;
    uint32_t inpitlane;
    uint32_t inpit;
    char* driver;
    char* car;
}
CarData;

typedef struct
{
    uint32_t simstatus;
    uint32_t velocity;
    uint32_t rpms;
    uint32_t gear;
    uint32_t pulses;
    uint32_t maxrpm;
    uint32_t altitude;
    uint32_t lap;
    uint32_t position;
    uint32_t numlaps;
    uint32_t playerlaps;
    uint32_t numcars;
    double gas;
    double brake;
    double fuel;
    double clutch;
    double steer;
    double handbrake;

    double worldposx;
    double worldposy;
    double worldposz;

    double braketemp[4];
    double tyrewear[4];
    double tyretemp[4];
    double tyrepressure[4];

    double airdensity;
    double airtemp;
    double tracktemp;

    double trackdistancearound;
    double playerspline;
    double trackspline;

    uint32_t lastlap;
    uint32_t bestlap;
    uint32_t currentlapinseconds;
    uint32_t lastlapinseconds;
    uint32_t time;
    uint32_t timeleft;
    uint32_t session;
    uint32_t sectorindex;
    uint32_t lastsectorinms;

    bool inpit;
    bool lapisvalid;

    char* car;
    char* track;
    char* driver;
    CarData cars[MAXCARS];
}
SimData;

#endif

#endif
