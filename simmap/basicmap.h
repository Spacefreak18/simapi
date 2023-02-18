#ifndef _BASICMAP_H
#define _BASICMAP_H

#include "../simapi/simapi.h"

struct Map {
    const char* name;
    void* value;
    SimDataType dtype;
};

#endif
