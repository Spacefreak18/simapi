#ifndef _AC_H
#define _AC_H

#include <stdbool.h>
#include "../include/acdata.h"

#define AC_EXE "acs.exe"
#define ACC_EXE "AC2-Win64-Shipping.exe"
#define ACE_EXE "AssettoCorsaEVO.exe"
#define ACR_EXE "acr.exe"

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
    int fd_physics;
    int fd_static;
    int fd_graphic;
    int fd_crewchief;
    struct SPageFilePhysics ac_physics;
    struct SPageFileStatic ac_static;
    struct SPageFileGraphic ac_graphic;
    struct SPageFileCrewChief ac_crewchief;
    char car[32];
    char driver[32];
    char track[32];
    char compound[32];
}
ACMap;

#endif
