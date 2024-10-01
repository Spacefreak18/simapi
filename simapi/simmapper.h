#ifndef _SIMMAPPER_H
#define _SIMMAPPER_H

#include "ac.h"
#include "rf2.h"
#include "pcars2.h"
#include "scs2.h"

#include "simdata.h"
#include "simapi.h"


typedef struct
{
    bool isSimOn;
    bool SimUsesUDP;
    bool SimSupportsMonocoque;
    bool SimSupportsMonocoqueTyreEffects;
    bool SimSupportsTelemetry;
    bool SimSupportsAdvancedUI;
    SimulatorAPI simulatorapi;
}
SimInfo;

typedef struct
{
    void* addr;
    int fd;
    union
    {
        ACMap ac;
        RF2Map rf2;
        PCars2Map pcars2;
        SCS2Map scs2;
    } d;
}
SimMap;

SimInfo getSim(SimData* simdata, SimMap* simmap, bool force_udp, int (*setup_udp)(int));
int siminit(SimData* simdata, SimMap* simmap, SimulatorAPI simulator);
int siminitudp(SimData* simdata, SimMap* simmap, SimulatorAPI simulator);
int simdatamap(SimData* simdata, SimMap* simmap, SimulatorAPI simulator, bool udp, char* base);
int simfree(SimData* simdata, SimMap* simmap, SimulatorAPI simulator);

#endif
