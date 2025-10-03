#ifndef _SIMMAP_H
#define _SIMMAP_H

#include "ac.h"
#include "rf2.h"
#include "pcars2.h"
#include "scs2.h"

struct _simmap
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
};

#endif
