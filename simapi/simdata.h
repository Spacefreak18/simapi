#ifndef _SIMDATA_H
#define _SIMDATA_H

#include <stdint.h>
#include <stdbool.h>

#define MAXCARS 24
#define PROXCARS 6

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
    double xpos;
    double ypos;
    double zpos;
    double carspline;
    double speed;
    uint32_t pos;
    uint32_t lap;
    uint32_t trackpos;
    LapTime lastlap;
    LapTime bestlap;
    // not all of these are set by each sim
    bool inpit; // no matter what, if you're in the pit set this
    bool inpitlane; // in pit lane, actively entering or exiting, but not stopped or in garage
    bool ingarage; // not always available
    bool inpitentrance; // not always available
    bool inpitexit; // not always available
    bool inpitstopped; // not always available
    char driver[128];
    char car[128];
}
CarData;

typedef struct
{
    double radius;
    double theta; // in degrees
    uint32_t lap;
}
ProximityData;

typedef struct
{
    uint64_t mtick;

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

    double Xvelocity;
    double Yvelocity;
    double Zvelocity;

    double worldXvelocity;
    double worldYvelocity;
    double worldZvelocity;

    double gas;
    double brake;
    double fuel;
    double clutch;
    double steer;
    double handbrake;

    double turboboost;
    double turboboostperct;
    double maxturbo;

    double abs;
    double brakebias;
    double tyreRPS[4];
    double tyrediameter[4];

    double heading;
    double worldposx;
    double worldposy;
    double worldposz;

    double braketemp[4];
    double tyrewear[4];
    double tyretemp[4];
    double tyrepressure[4];

    double tyrecontact0[4];
    double tyrecontact1[4];
    double tyrecontact2[4];

    double airdensity;
    double airtemp;
    double tracktemp;

    double suspension[4];

    double trackdistancearound;
    double playerspline;
    double trackspline;
    uint32_t playertrackpos;
    uint32_t tracksamples;

    LapTime lastlap;
    LapTime bestlap;
    LapTime currentlap;
    uint32_t currentlapinseconds;
    uint32_t lastlapinseconds;
    uint32_t time;
    LapTime sessiontime;
    uint8_t session;
    uint8_t sectorindex;
    uint32_t lastsectorinms;
    uint8_t courseflag;
    uint8_t playerflag;

    bool lapisvalid;

    char car[128];
    char track[128];
    char driver[128];
    char tyrecompound[128];
    CarData cars[MAXCARS];

    ProximityData pd[PROXCARS];

    uint8_t simapi;
    uint8_t simexe;
    bool simon;
    uint8_t simapiversion;
}
SimData;

#pragma pack(pop)
#endif
