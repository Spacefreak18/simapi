#ifndef _BASICMAP_H
#define _BASICMAP_H

#include "../simapi/simapi.h"
#include "../simapi/simdata.h"

struct Map
{
    const char* name;
    void* value;
    SimDataType dtype;
};

int CreateSimDataMap(struct Map *map, SimData *simdata, int mapdata);

#endif
