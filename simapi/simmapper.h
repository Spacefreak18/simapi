#ifndef _SIMMAPPER_H
#define _SIMMAPPER_H

#include "ac.h"
#include "rf2.h"

#include "simdata.h"

typedef struct
{
    void* addr;
    int fd;
    union
    {
        ACMap ac;
        RF2Map rf2;
    } d;
}
SimMap;

void getSim(SimData* simdata, SimMap* simmap, bool* simstate, Simulator* sim);
int siminit(SimData* simdata, SimMap* simmap, Simulator simulator);
int simdatamap(SimData* simdata, SimMap* simmap, Simulator simulator);

#endif
