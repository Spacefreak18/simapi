#ifndef _SIMMAP_H
#define _SIMMAP_H

#include "ac.h"
#include "rf2.h"
#include "pcars2.h"
#include "scs2.h"
#include "dirt2.h"
#include "wreckfest2.h"
#include "rbr.h"

struct _simmap
{
    void* addr;
    int fd;
    bool hasSimApiDat;

    ACMap ac;
    RF2Map rf2;
    PCars2Map pcars2;
    SCS2Map scs2;
    Dirt2Map dirt2;
    Wreckfest2Map wf2;
    RBRMap rbr;
};

#endif
