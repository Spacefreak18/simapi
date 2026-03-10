#ifndef _SIMMAPPER_H
#define _SIMMAPPER_H

//#include "ac.h"
//#include "rf2.h"
//#include "pcars2.h"
//#include "scs2.h"
#include <signal.h>

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
    pid_t pid;
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
    void* r3e_addr;
    int r3e_fd;
}
SimCompatMap;

bool simapi_does_sim_need_bridge(SimulatorEXE s);
SimulatorEXE simapi_get_sim_exe(SimInfo* si);
SimInfo simapi_get_sim(SimData* simdata, SimMap* simmap, bool force_udp, int (*setup_udp)(int), bool simd);

int simapi_strtogame(const char* game);
char* simapi_gametostr(SimulatorEXE sim);
char* simapi_gametofullstr(SimulatorEXE sim);

SimMap* simapi_simmap_create(void);

int simapi_init(SimData* simdata, SimMap* simmap, SimulatorAPI simulator);
int simapi_initudp(SimData* simdata, SimMap* simmap, SimulatorAPI simulator);
int simapi_datamap(SimData* simdata, SimMap* simmap, SimulatorAPI simulator, bool udp, char* base);
int simapi_sim_clear(SimData* simdata, SimMap* simmap);
int simapi_universalmap_open(SimMap* simmap, SimData* simdata);
int simapi_universalmap_free(SimMap* simmap, bool issimd);
int simapi_compatmap_open(SimCompatMap* compatmap);
int simapi_compatmap_free(SimCompatMap* compatmap);
int simapi_compatmap_clear(SimCompatMap* compatmap);

void simapi_set_proximity_data(SimData* simdata, int cars, int8_t lr_flip);

void map_assetto_corsa_data(SimData* simdata, SimMap* simmap, SimulatorEXE simexe);
void map_rfactor2_data(SimData* simdata, SimMap* simmap);
void map_project_cars2_data(SimData* simdata, SimMap* simmap, bool udp, char* base);
void map_trucks_data(SimData* simdata, SimMap* simmap);
void map_outgauge_outsim_data(SimData* simdata, SimMap* simmap, SimulatorEXE simexe, char* base);
void map_dirt_rally_2_data(SimData* simdata, SimMap* simmap, char* base);
void map_f1_2018_data(SimData* simdata, SimMap* simmap, char* base);
void map_wreckfest2_data(SimData* simdata, SimMap* simmap, char* base);
void map_richard_burns_rally_data(SimData* simdata, SimMap* simmap, char* base);
void map_forza_data(SimData* simdata, SimMap* simmap, char* base);
void map_r3e_data(SimData* simdata, SimMap* simmap);


#endif
