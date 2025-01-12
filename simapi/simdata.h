#ifndef _SIMDATA_H
#define _SIMDATA_H

#include <stdint.h>
#include <stdbool.h>

#define MAXCARS 24

typedef enum
{
    SIMAPI_FLAG_GREEN            = 0,
    SIMAPI_FLAG_YELLOW           = 1,
    SIMAPI_FLAG_RED              = 2,
    SIMAPI_FLAG_CHEQUERED        = 3,
    SIMAPI_FLAG_BLUE             = 4,
    SIMAPI_FLAG_WHITE            = 5,
    SIMAPI_FLAG_BLACK            = 6,
    SIMAPI_FLAG_BLACK_WHITE      = 7,
    SIMAPI_FLAG_BLACK_ORANGE     = 8,
    SIMAPI_FLAG_ORANGE           = 9
}
SimAPI_FLAG;


#pragma pack(push)
#pragma pack(4)

typedef struct
{
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;
    uint32_t fraction;
}
LapTime;

typedef struct
{
    uint32_t xpos;
    uint32_t ypos;
    uint32_t zpos;
    uint32_t pos;
    uint32_t lap;
    LapTime lastlap;
    LapTime bestlap;
    uint32_t inpitlane;
    uint32_t inpit;
    char driver[128];
    char car[128];
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
    char gearc[3];

    uint32_t Xvelocity;
    uint32_t Yvelocity;
    uint32_t Zvelocity;

    double gas;
    double brake;
    double fuel;
    double clutch;
    double steer;
    double handbrake;

    double abs;
    double brakebias;
    double tyreRPS[4];
    double tyrediameter[4];

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

    LapTime lastlap;
    LapTime bestlap;
    LapTime currentlap;
    uint32_t currentlapinseconds;
    uint32_t lastlapinseconds;
    uint32_t time;
    LapTime sessiontime;
    uint32_t session;
    uint32_t sectorindex;
    uint32_t lastsectorinms;
    uint8_t courseflag;
    uint8_t playerflag;

    bool inpit;
    bool lapisvalid;

    char car[128];
    char track[128];
    char driver[128];
    char tyrecompound[128];
    CarData cars[MAXCARS];

    int sim;
    int simapi;
    bool simon;
    int simversion;
}
SimData;

#pragma pack(pop)
#endif
