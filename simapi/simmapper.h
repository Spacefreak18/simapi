#ifndef _SIMMAPPER_H
#define _SIMMAPPER_H

//#include "ac.h"
//#include "rf2.h"
//#include "pcars2.h"
//#include "scs2.h"

#include "simdata.h"
#include "simapi.h"


typedef struct
{
    bool isSimOn;
    bool SimUsesUDP;
    bool SimSupportsBasicTelemetry;
    bool SimSupportsTyreEffects;
    bool SimSupportsRealtimeTelemetry;
    bool SimSupportsAdvancedUI;
    SimulatorAPI mapapi;
    SimulatorAPI simulatorapi;
    SimulatorEXE simulatorexe;
}
SimInfo;


//typedef struct
//{
//    void* addr;
//    int fd;
//    union
//    {
//        ACMap* ac;
//        RF2Map* rf2;
//        PCars2Map* pcars2;
//        SCS2Map* scs2;
//    } d;
//}
//SimMap;

//struct _simmap;
typedef struct _simmap SimMap;


typedef struct
{
    void* pcars2_addr;
    int pcars2_fd;
    void* acphysics_addr;
    int acphysics_fd;
    void* acgraphics_addr;
    int acgraphics_fd;
    void* acstatic_addr;
    int acstatic_fd;
    void* accrew_addr;
    int accrew_fd;
}
SimCompatMap;

SimInfo getSim(SimData* simdata, SimMap* simmap, bool force_udp, int (*setup_udp)(int), bool simd);
int siminit(SimData* simdata, SimMap* simmap, SimulatorAPI simulator);
int siminitudp(SimData* simdata, SimMap* simmap, SimulatorAPI simulator);
int simdatamap(SimData* simdata, SimMap* simmap, SimMap* simmap2, SimulatorAPI simulator, bool udp, char* base);
int simfree(SimData* simdata, SimMap* simmap, SimulatorAPI simulator);

int simapi_strtogame(const char* game);
char* simapi_gametostr(SimulatorEXE sim);

SimMap* createSimMap(void);
void* getSimMapPtr(SimMap* simmap);
int simdmap(SimMap* simmap, SimData* simdata);
int opensimmap(SimMap* simmap);
int freesimmap(SimMap* simmap, bool issimd);
int opensimcompatmap(SimCompatMap* compatmap);
int freesimcompatmap(SimCompatMap* compatmap);

#endif
