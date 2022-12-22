#ifndef _AC_H
#define _AC_H

#include <stdbool.h>
#include "../include/acdata.h"

#define AC_PHYSICS_FILE "acpmf_physics"
#define AC_STATIC_FILE "acpmf_static"
#define AC_GRAPHIC_FILE "acpmf_graphic"

typedef struct
{
    bool has_physics;
    bool has_static;
    bool has_graphic;
    void* physics_map_addr;
    void* static_map_addr;
    void* graphic_map_addr;
    struct SPageFilePhysics ac_physics;
    struct SPageFileStatic ac_static;
    struct SPageFileGraphic ac_graphic;
}
ACMap;

#endif
