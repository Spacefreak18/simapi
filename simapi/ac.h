#ifndef _AC_H
#define _AC_H

#include <stdbool.h>
#include "../include/acdata.h"

#define AC_PHYSICS_FILE "acpmf_physics"
#define AC_STATIC_FILE "acpmf_static"
#define AC_GRAPHIC_FILE "acpmf_graphics"
#define AC_CREWCHIEF_FILE "acpmf_crewchief"

typedef struct
{
    bool has_physics;
    bool has_static;
    bool has_graphic;
    bool has_crewchief;
    void* physics_map_addr;
    void* static_map_addr;
    void* graphic_map_addr;
    void* crewchief_map_addr;
    struct SPageFilePhysics ac_physics;
    struct SPageFileStatic ac_static;
    struct SPageFileGraphic ac_graphic;
    struct SPageFileCrewChief ac_crewchief;
}
ACMap;

#endif
