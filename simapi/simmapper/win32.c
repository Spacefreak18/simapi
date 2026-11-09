/* Windows simmapper implementation; selected by ../simmapper.c. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>

#include "../simdata.h"
#include "../simapi.h"
#include "../simmapper.h"
#include "../test.h"
#include "../ac.h"
#include "../rf2.h"
#include "../pcars2.h"
#include "../scs2.h"
#include "../outgauge.h"
#include "../dirt2.h"
#include "../f1.h"
#include "../wreckfest2.h"
#include "../rbr.h"
#include "../forzadef.h"
#include "../r3edef.h"
#include "../lmudef.h"
#include "../simmap.h"

/* Windows-only compatibility declarations; private to this implementation. */
#ifdef _WIN32
#ifndef S_IRUSR
#define S_IRUSR 0400
#endif
#ifndef S_IWUSR
#define S_IWUSR 0200
#endif
#endif

#ifndef PROT_READ
#define PROT_READ 1
#endif
#ifndef PROT_WRITE
#define PROT_WRITE 2
#endif
#ifndef MAP_SHARED
#define MAP_SHARED 1
#endif
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

int simapi_platform_shm_open(const char *name, int flags, int mode);
int simapi_platform_file_open(const char *path, int flags, int mode);
void *simapi_platform_shm_map(void *hint, size_t length, int prot, int flags, int fd, size_t offset);
int simapi_platform_shm_unmap(void *address, size_t length);
int simapi_platform_shm_resize(int fd, size_t length);
int simapi_platform_shm_close(int fd);
int simapi_platform_shm_unlink(const char *name);
int simapi_platform_shm_exists(const char *name);
long long simapi_platform_time_ms(void);
int simapi_platform_asprintf(char **out, const char *format, ...);

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../../include/f12018.h"
#include "../../include/acdata.h"
#include "../../include/rf2data.h"
#include "../../include/pcars2data.h"
#include "../../include/outgauge.h"
#include "../../include/scs2data.h"

static bool does_sim_file_exist(const char* file);
static int simdmap(SimMap* simmap, SimData* simdata);




SimMap* simapi_simmap_create()
{
    SimMap* ptr = malloc(sizeof(SimMap));
    ptr->fd = -1;
    ptr->addr = 0;
    return ptr;
}


int sstrcicmp(char const *a, char const *b)
{
    for (;; a++, b++)
    {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
        {
            return d;
        }
    }
}

long long timeInMilliseconds(void)
{
    return simapi_platform_time_ms();
}

void map_suspension_velocity(SimData* simdata, double new_suspension[4])
{
    // Calculate suspension velocity as rate of change between frames
    // Units: meters per second (m/s)
    // Note: simdata->suspension contains previous frame's values at this point
    if (simdata->prev_mtick > 0 && simdata->mtick > simdata->prev_mtick)
    {
        double dt = (double)(simdata->mtick - simdata->prev_mtick) / 1000.0; // Convert ms to seconds
        if (dt > 0)
        {
            for (int i = 0; i < 4; i++)
            {
                simdata->suspvelocity[i] = fabs((new_suspension[i] - simdata->suspension[i]) / dt);
            }
        }
    }
}

bool simapi_does_sim_need_bridge(SimulatorEXE s)
{
#if defined(_WIN32)
    (void)s;
    return false; /* Native Windows simulators publish named mappings. */
#endif
    if(s == SIMULATOREXE_ASSETTO_CORSA || s == SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE || s == SIMULATOREXE_ASSETTO_CORSA_EVO || s == SIMULATOREXE_ASSETTO_CORSA_RALLY)
    {
        return true;
    }

    if(s == SIMULATOREXE_AUTOMOBILISTA2 || s == SIMULATOREXE_AUTOMOBILISTA2_DEMO)
    {
        return true;
    }

    if (s == SIMULATOREXE_LEMANS_ULTIMATE)
    {
        return true;
    }

    if(s == SIMULATOREXE_RACE_ROOM)
    {
        return true;
    }
    return false;
}

int simapi_strtogame(const char* game)
{
    int sim = 0;
    if (sstrcicmp(game, "ac") == 0)
    {
        sim = SIMULATOREXE_ASSETTO_CORSA;
    }
    else if (sstrcicmp(game, "acc") == 0)
    {
        sim = SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE;
    }
    else if (sstrcicmp(game, "ace") == 0)
    {
        sim = SIMULATOREXE_ASSETTO_CORSA_EVO;
    }
    else if (sstrcicmp(game, "acr") == 0)
    {
        sim = SIMULATOREXE_ASSETTO_CORSA_RALLY;
    }
    else if (sstrcicmp(game, "rf2") == 0)
    {
        sim = SIMULATOREXE_RFACTOR2;
    }
    else if (sstrcicmp(game, "ams2") == 0)
    {
        sim = SIMULATOREXE_AUTOMOBILISTA2;
    }
    else if (sstrcicmp(game, "ams2demo") == 0)
    {
        sim = SIMULATOREXE_AUTOMOBILISTA2_DEMO;
    }
    else if (sstrcicmp(game, "et2") == 0)
    {
        sim = SIMULATOREXE_EUROTRUCKS2;
    }
    else if (sstrcicmp(game, "at") == 0)
    {
        sim = SIMULATOREXE_EUROTRUCKS2;
    }
    else if (sstrcicmp(game, "lmu") == 0)
    {
        sim = SIMULATOREXE_LEMANS_ULTIMATE;
    }
    else if (sstrcicmp(game, "lfs") == 0)
    {
        sim = SIMULATOREXE_LIVE_FOR_SPEED;
    }
    else if (sstrcicmp(game, "beamng") == 0)
    {
        sim = SIMULATOREXE_BEAMNG;
    }
    else if (sstrcicmp(game, "dr2") == 0)
    {
        sim = SIMULATOREXE_DIRT_RALLY_2;
    }
    else if (sstrcicmp(game, "f122") == 0)
    {
        sim = SIMULATOREXE_F1_2022;
    }
    else if (sstrcicmp(game, "fh5") == 0)
    {
        sim = SIMULATOREXE_FORZA_HORIZON_5;
    }
    else if (sstrcicmp(game, "fh6") == 0)
    {
        sim = SIMULATOREXE_FORZA_HORIZON_6;
    }
    else if (sstrcicmp(game, "r3e") == 0)
    {
        sim = SIMULATOREXE_RACE_ROOM;
    }
    else if (sstrcicmp(game, "wf2") == 0)
    {
        sim = SIMULATOREXE_WRECKFEST2;
    }
    else if (sstrcicmp(game, "rbr") == 0)
    {
        sim = SIMULATOREXE_RICHARD_BURNS_RALLY;
    }
    else
    {
        sim = 0;
    }
    return sim;
}

char* simapi_gametostr(SimulatorEXE sim)
{
    switch ( sim )
    {
        case SIMULATOREXE_ASSETTO_CORSA:
            return "ac";
        case SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE:
            return "acc";
        case SIMULATOREXE_ASSETTO_CORSA_EVO:
            return "ace";
        case SIMULATOREXE_ASSETTO_CORSA_RALLY:
            return "acr";
        case SIMULATOREXE_RFACTOR2:
            return "rf2";
        case SIMULATOREXE_LEMANS_ULTIMATE:
            return "lmu";
        case SIMULATOREXE_AUTOMOBILISTA2:
            return "ams2";
        case SIMULATOREXE_AUTOMOBILISTA2_DEMO:
            return "ams2demo";
        case SIMULATOREXE_AMERICANTRUCKS:
            return "at";
        case SIMULATOREXE_EUROTRUCKS2:
            return "et2";
        case SIMULATOREXE_LIVE_FOR_SPEED:
            return "lfs";
        case SIMULATOREXE_BEAMNG:
            return "beamng";
        case SIMULATOREXE_DIRT_RALLY_2:
            return "dr2";
        case SIMULATOREXE_F1_2022:
            return "f122";
        case SIMULATOREXE_FORZA_HORIZON_5:
            return "fh5";
        case SIMULATOREXE_FORZA_HORIZON_6:
            return "fh6";
        case SIMULATOREXE_RACE_ROOM:
            return "r3e";
        case SIMULATOREXE_WRECKFEST2:
            return "wf2";
        case SIMULATOREXE_RICHARD_BURNS_RALLY:
            return "rbr";
        default:
            return "default";
    }
}

char* simapi_gametofullstr(SimulatorEXE sim)
{
    switch ( sim )
    {
        case SIMULATOREXE_ASSETTO_CORSA:
            return "Assetto Corsa";
        case SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE:
            return "Assetto Corsa Competizione";
        case SIMULATOREXE_ASSETTO_CORSA_EVO:
            return "Assetto Corsa EVO";
        case SIMULATOREXE_ASSETTO_CORSA_RALLY:
            return "Assetto Corsa Rally";
        case SIMULATOREXE_RFACTOR2:
            return "RFactor2";
        case SIMULATOREXE_LEMANS_ULTIMATE:
            return "LeMans Ultimate";
        case SIMULATOREXE_AUTOMOBILISTA2:
            return "Automobilista 2";
        case SIMULATOREXE_AUTOMOBILISTA2_DEMO:
            return "Automobilista 2 Demo";
        case SIMULATOREXE_AMERICANTRUCKS:
            return "American Truck Simulator";
        case SIMULATOREXE_EUROTRUCKS2:
            return "Euro Truck Simulator 2";
        case SIMULATOREXE_LIVE_FOR_SPEED:
            return "Live For Speed";
        case SIMULATOREXE_BEAMNG:
            return "beamng";
        case SIMULATOREXE_DIRT_RALLY_2:
            return "DiRT Rally 2.0";
        case SIMULATOREXE_F1_2022:
            return "F1 2022";
        case SIMULATOREXE_FORZA_HORIZON_5:
            return "Forza Horizon 5";
        case SIMULATOREXE_FORZA_HORIZON_6:
            return "Forza Horizon 6";
        case SIMULATOREXE_RACE_ROOM:
            return "Race Room";
        case SIMULATOREXE_WRECKFEST2:
            return "Wreckfest 2";
        case SIMULATOREXE_RICHARD_BURNS_RALLY:
            return "Richard Burns Rally";
        default:
            return "default";
    }
}


int droundint(double d)
{
    return trunc(nearbyint(d));
}

int froundint(float f)
{
    return trunc(nearbyint(f));
}

func_ptr_t loginfo = NULL;
func_ptr_t logdebug = NULL;
func_ptr_t logtrace = NULL;

void simapi_set_log_info(func_ptr_t func)
{
    loginfo = func;
}
void simapi_set_log_debug(func_ptr_t func)
{
    logdebug = func;
}
void simapi_set_log_trace(func_ptr_t func)
{
    logtrace = func;
}

void simapi_log(SIMAPI_LOGLEVEL sll, char* message)
{
    if(message != NULL)
    {
        if(sll == SIMAPI_LOGLEVEL_INFO)
        {
            if(loginfo != NULL)
            {
                loginfo(message);
            }
        }
        if(sll == SIMAPI_LOGLEVEL_DEBUG)
        {
            if(logdebug != NULL)
            {
                logdebug(message);
            }
        }
        if(sll == SIMAPI_LOGLEVEL_TRACE)
        {
            if(logtrace != NULL)
            {
                logtrace(message);
            }
        }
    }
}

void simapi_set_proximity_data(SimData* simdata, int cars, int8_t lr_flip)
{
    double carwidth = 1.8;
    double maxradius = 10.0;

    int proxcars = PROXCARS;
    if(cars < PROXCARS)
    {
        proxcars = cars;
    }


    for(int x = 0; x < proxcars; x++)
    {
        simdata->pd[x].radius = 0.0;
        simdata->pd[x].theta = 0.0;
    }
    if(abs(simdata->Yvelocity) < 0.0001 && abs(simdata->Xvelocity) < 0.0001)
    {
        return;
    }

    double angle = atan2(-1, 0) - atan2(simdata->worldYvelocity, simdata->worldXvelocity);
    double angleD = angle * 360 / (2 * M_PI);
    double angleR = angleD * M_PI / 180;

    double cosTheta = cos(angle);
    double sinTheta = sin(angle);

    for(int car = 1; car < cars; car++)
    {
        double rawXCoordinate = simdata->cars[car].xpos - simdata->worldposx;
        double rawYCoordinate = simdata->cars[car].ypos - simdata->worldposy;

        double xscore  = cosTheta * rawXCoordinate - sinTheta * rawYCoordinate;
        double yscore  = sinTheta * rawXCoordinate + cosTheta * rawYCoordinate;

        double rads = atan2(yscore, xscore * lr_flip);
        double degrees = (rads * (180 / M_PI)) + 90.0;

        double radius = sqrt((rawXCoordinate * rawXCoordinate) + (rawYCoordinate * rawYCoordinate));
        double theta = degrees;
        if( theta < 0 )
        {
            theta = 360 + degrees;
        }
        radius = radius - carwidth;

        //fprintf(stderr, "rawx: %f, rawy: %f degs: %f mag: %f\n", rawXCoordinate, rawYCoordinate, theta, radius);

        if(radius < maxradius)
        {
            int j = proxcars - 1;
            if((simdata->pd[j].radius == 0) || (radius < simdata->pd[j].radius))
            {
                simdata->pd[j].radius = radius;
                simdata->pd[j].theta = theta;


                j--;
                while(j >= 0)
                {
                    double tempradius = simdata->pd[j+1].radius;
                    double temptheta = simdata->pd[j+1].theta;


                    if((simdata->pd[j].radius == 0) || (tempradius < simdata->pd[j].radius))
                    {
                        simdata->pd[j+1].radius = simdata->pd[j].radius;
                        simdata->pd[j+1].theta = simdata->pd[j].theta;

                        simdata->pd[j].radius = tempradius;
                        simdata->pd[j].theta = temptheta;
                    }
                    j--;
                }
            }
        }
    }
}


static bool does_sim_file_exist(const char* file)
{
    return simapi_platform_shm_exists(file) != 0;
}


// if this becomes more necessary i will move it into it's own file
float spLineLengthToDistanceRoundTrack(float trackLength, float spLine)
{
    if (spLine < 0.0)
    {
        spLine -= 1;
    }
    return spLine * trackLength;
}

static int set_sim_info(SimInfo* si)
{
    switch ( si->simulatorapi )
    {

        case SIMULATORAPI_ASSETTO_CORSA :
            si->SimUsesUDP = false;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = true;
            si->SimCalculatesTyreDiameter = true;
            si->SimSupportsHapticEffects = true;
            si->SimSupportsAdvancedUI = true;
            break;
        case SIMULATORAPI_RFACTOR2 :
            si->SimUsesUDP = false;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = true;
            si->SimSupportsHapticEffects = true;
            si->SimSupportsAdvancedUI = true;
            break;
        case SIMULATORAPI_PROJECTCARS2 :
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = true;
            si->SimSupportsHapticEffects = true;
            si->SimSupportsAdvancedUI = true;
            break;
        case SIMULATORAPI_SCSTRUCKSIM2 :
            si->SimUsesUDP = false;
            si->SimSupportsBasicTelemetry = true;
            break;
        case SIMULATORAPI_OUTSIMOUTGAUGE :
            si->SimUsesUDP = true;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = false;
            si->SimSupportsRealtimeTelemetry = false;
            si->SimSupportsAdvancedUI = false;
            si->SimSupportsHapticEffects = true;
            si->SimCalculatesSlipRatio = true;
            break;
        case SIMULATORAPI_DIRT_RALLY_2 :
            si->SimUsesUDP = true;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = true;
            si->SimSupportsHapticEffects = true;
            si->SimSupportsAdvancedUI = false;
            break;
        case SIMULATORAPI_F1_2018:
            si->SimUsesUDP = true;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsHapticEffects = true;
            si->SimSupportsRealtimeTelemetry = false;
            si->SimSupportsAdvancedUI = false;
            break;
        case SIMULATORAPI_RICHARD_BURNS_RALLY:
            si->SimUsesUDP = true;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = false;
            si->SimSupportsHapticEffects = true;
            si->SimSupportsAdvancedUI = false;
            break;
        default:
            si->SimSupportsBasicTelemetry = true;
    }

    return 0;
}

void hexDump(char* desc, void* addr, int len)
{
    int i;
    unsigned char buff[17];
    unsigned char* pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
    {
        printf ("%s:\n", desc);
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++)
    {
        // Multiple of 16 means new line (with line offset).
        if ((i % 16) == 0)
        {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
            {
                printf ("  %s\n", buff);
            }

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
        {
            buff[i % 16] = '.';
        }
        else
        {
            buff[i % 16] = pc[i];
        }
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0)
    {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

SimulatorEXE simapi_get_sim_exe(SimInfo* si)
{
    int pid = 0;

    char* processes[19];
    processes[0] = AC_EXE;
    processes[1] = ACC_EXE;
    processes[2] = ACE_EXE;
    processes[3] = ACR_EXE;
    processes[4] = RFACTOR2_EXE;
    processes[5] = AMS2_EXE;
    processes[6] = AMS2_DEMO_EXE;
    processes[7] = EUROTRUCKS2_EXE;
    processes[8] = AMERICANTRUCKS_EXE;
    processes[9] = LEMANS_ULTIMATE_EXE;
    processes[10] = LIVE_FOR_SPEED_EXE;
    processes[11] = BEAMNG_EXE;
    processes[12] = DIRT_RALLY_2_EXE;
    processes[13] = F1_2022_EXE;
    processes[14] = WRECKFEST2_EXE;
    processes[15] = RICHARD_BURNS_RALLY_EXE;
    processes[16] = FORZA_HORIZON_5_EXE;
    processes[17] = FORZA_HORIZON_6_EXE;
    processes[18] = RACE_ROOM_EXE;

    SimulatorEXE sim_exe[19];
    sim_exe[0] = SIMULATOREXE_ASSETTO_CORSA;
    sim_exe[1] = SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE;
    sim_exe[2] = SIMULATOREXE_ASSETTO_CORSA_EVO;
    sim_exe[3] = SIMULATOREXE_ASSETTO_CORSA_RALLY;
    sim_exe[4] = SIMULATOREXE_RFACTOR2;
    sim_exe[5] = SIMULATOREXE_AUTOMOBILISTA2;
    sim_exe[6] = SIMULATOREXE_AUTOMOBILISTA2_DEMO;
    sim_exe[7] = SIMULATOREXE_EUROTRUCKS2;
    sim_exe[8] = SIMULATOREXE_AMERICANTRUCKS;
    sim_exe[9] = SIMULATOREXE_LEMANS_ULTIMATE;
    sim_exe[10] = SIMULATOREXE_LIVE_FOR_SPEED;
    sim_exe[11] = SIMULATOREXE_BEAMNG;
    sim_exe[12] = SIMULATOREXE_DIRT_RALLY_2;
    sim_exe[13] = SIMULATOREXE_F1_2022;
    sim_exe[14] = SIMULATOREXE_WRECKFEST2;
    sim_exe[15] = SIMULATOREXE_RICHARD_BURNS_RALLY;
    sim_exe[16] = SIMULATOREXE_FORZA_HORIZON_5;
    sim_exe[17] = SIMULATOREXE_FORZA_HORIZON_6;
    sim_exe[18] = SIMULATOREXE_RACE_ROOM;

    struct SimProcessInfo r = get_process_match(processes, 19);

    if(r.pid >= 0)
    {
        si->pid = r.pid;
        return(sim_exe[r.pos]);
    }

    si->pid = -1;
    return SIMULATOREXE_SIMAPI_TEST_NONE;
}

void simapi_set_faux_siminfo(SimInfo* si)
{
    si->simulatorapi = 0;
    si->mapapi = 0;
    si->simulatorexe = 0;
    si->isSimOn = true;
    si->SimUsesUDP = false;
    si->SimSupportsBasicTelemetry = true;
    si->SimSupportsTyreEffects = true;
    si->SimSupportsRealtimeTelemetry = true;
    si->SimSupportsAdvancedUI = true;
    si->SimCalculatesTyreDiameter = true;
    si->SimCalculatesSlipRatio = true;
    si->SimSupportsHapticEffects = true;
}

SimInfo simapi_get_sim(SimData* simdata, SimMap* simmap, bool force_udp, int (*setup_udp)(int), bool simd)
{

    SimInfo si;
    si.simulatorapi = -1;
    si.mapapi = -1;
    si.simulatorexe = 0;
    si.isSimOn = false;
    si.SimUsesUDP = false;
    si.SimSupportsBasicTelemetry = false;
    si.SimSupportsTyreEffects = false;
    si.SimSupportsRealtimeTelemetry = false;
    si.SimSupportsAdvancedUI = false;
    si.SimCalculatesTyreDiameter = false;
    si.SimCalculatesSlipRatio = false;
    si.SimSupportsHapticEffects = false;


    if(simd == false)
    {
        if (does_sim_file_exist("/dev/shm/SIMAPI.DAT"))
        {
            int e = simapi_init(simdata, simmap, SIMULATORAPI_SIMAPI_TEST, SIMULATOREXE_SIMAPI_TEST_NONE);
            simapi_datamap(simdata, simmap, SIMULATORAPI_SIMAPI_TEST, false, NULL);
            char* temp;
            simapi_platform_asprintf(&temp, "found running simapi daemon simint error %i", e);
            simapi_log(SIMAPI_LOGLEVEL_TRACE, temp);
            free(temp);
            //simapi_datamap(simdata, simmap, NULL, SIMULATORAPI_SIMAPI_TEST, false, NULL);
            if(simdata->simapiversion == SIMAPI_VERSION)
            {
                si.mapapi = SIMULATORAPI_SIMAPI_TEST;
                if (simdata->simon == 1)
                {
                    simapi_log(SIMAPI_LOGLEVEL_TRACE, "status okay");
                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.simulatorexe = simdata->simexe;
                    si.mapapi = SIMULATORAPI_SIMAPI_TEST;
                    si.simulatorexe = simdata->simexe;
                    set_sim_info(&si);
                    si.SimUsesUDP = false;
                }
                return si;
            }
            else
            {
                char* temp;
                simapi_platform_asprintf(&temp, "skipping sim api daemon due to version mismatch. Daemon Version: %i. App Version: %i", simdata->simapiversion, SIMAPI_VERSION);
                simapi_log(SIMAPI_LOGLEVEL_INFO, temp);
                free(temp);
            }
        }
        else
        {
            simapi_log(SIMAPI_LOGLEVEL_TRACE, "looking for running simulators");
        }
    }

    SimulatorEXE simexe = simapi_get_sim_exe(&si);

    switch ( simexe )
    {
        case SIMULATOREXE_ASSETTO_CORSA:
        case SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE:

        case SIMULATOREXE_ASSETTO_CORSA_RALLY:
            simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Found running process for Assetto Corsa");
            if (does_sim_file_exist("/dev/shm/acpmf_physics"))
            {
                if (does_sim_file_exist("/dev/shm/acpmf_static"))
                {
                    simapi_log(SIMAPI_LOGLEVEL_DEBUG, "static and physics files found");
                    si.simulatorapi = SIMULATORAPI_ASSETTO_CORSA;
                    int error = simapi_init(simdata, simmap, SIMULATORAPI_ASSETTO_CORSA, simexe);
                    char* temp;
                    simapi_platform_asprintf(&temp, "simapi_init error %i", error);
                    simapi_log(SIMAPI_LOGLEVEL_DEBUG, temp);
                    free(temp);
                    simapi_datamap(simdata, simmap, SIMULATORAPI_ASSETTO_CORSA, false, NULL);

                    // temporary workaround for beta data from ACEvo and ACRally
                    if(simexe == SIMULATOREXE_ASSETTO_CORSA_EVO || simexe == SIMULATOREXE_ASSETTO_CORSA_RALLY)
                    {
                        simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;
                    }

                    if (error == 0 && simdata->simstatus > 1)
                    {
                        simapi_log(SIMAPI_LOGLEVEL_DEBUG, "AC Shared memory looks good");
                        simdata->simon = true;
                        simdata->simapi = SIMULATORAPI_ASSETTO_CORSA;
                        simdata->simexe = simexe;

                        si.isSimOn = true;
                        si.simulatorapi = simdata->simapi;
                        si.mapapi = si.simulatorapi;
                        si.simulatorexe = simdata->simexe;
                        set_sim_info(&si);
                        if(simexe == SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE)
                        {
                            // support will have to be revisited for this sim
                            si.SimSupportsRealtimeTelemetry = false;
                        }

                        return si;
                    }
                }
                else
                {
                    simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Could not find static shared memory file");
                }
            }
            else
            {
                simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Could not find physics shared memory file");
            }
            break;

        case SIMULATOREXE_ASSETTO_CORSA_EVO:
            simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Found running process for Assetto Corsa EVO");
            if (does_sim_file_exist("/dev/shm/acevo_pmf_physics"))
            {
                if (does_sim_file_exist("/dev/shm/acevo_pmf_static"))
                {
                    simapi_log(SIMAPI_LOGLEVEL_DEBUG, "static and physics files found");
                    si.simulatorapi = SIMULATORAPI_ASSETTO_CORSA;
                    int error = simapi_init(simdata, simmap, SIMULATORAPI_ASSETTO_CORSA, simexe);
                    char* temp;
                    simapi_platform_asprintf(&temp, "simapi_init error %i", error);
                    simapi_log(SIMAPI_LOGLEVEL_DEBUG, temp);
                    free(temp);
                    simapi_datamap(simdata, simmap, SIMULATORAPI_ASSETTO_CORSA, false, NULL);

                    // temporary workaround for beta data from ACEvo and ACRally
                    if(simexe == SIMULATOREXE_ASSETTO_CORSA_EVO || simexe == SIMULATOREXE_ASSETTO_CORSA_RALLY)
                    {
                        simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;
                    }

                    if (error == 0 && simdata->simstatus > 1)
                    {
                        simapi_log(SIMAPI_LOGLEVEL_DEBUG, "AC Shared memory looks good");
                        simdata->simon = true;
                        simdata->simapi = SIMULATORAPI_ASSETTO_CORSA;
                        simdata->simexe = simexe;

                        si.isSimOn = true;
                        si.simulatorapi = simdata->simapi;
                        si.mapapi = si.simulatorapi;
                        si.simulatorexe = simdata->simexe;
                        set_sim_info(&si);
                        if(simexe == SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE)
                        {
                            // support will have to be revisited for this sim
                            si.SimSupportsRealtimeTelemetry = false;
                        }

                        return si;
                    }
                }
                else
                {
                    simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Could not find static shared memory file");
                }
            }
            else
            {
                simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Could not find physics shared memory file");
            }
            break;

        case SIMULATOREXE_LIVE_FOR_SPEED:
        case SIMULATOREXE_BEAMNG:
            int error = 0;
            if(*setup_udp != NULL)
            {
                error = (*setup_udp)(30000);
            }
            error = simapi_initudp(simdata, simmap, SIMULATORAPI_OUTSIMOUTGAUGE);

            if(error == 0)
            {
                simdata->simon = true;
                simdata->simapi = SIMULATORAPI_OUTSIMOUTGAUGE;
                simdata->simexe = simexe;

                simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;
                simdata->gear = 0;
                simdata->velocity = 0;
                simdata->rpms = 0;
                simdata->altitude = 0;

                si.isSimOn = true;
                si.simulatorapi = simdata->simapi;
                si.mapapi = si.simulatorapi;
                si.simulatorexe = simdata->simexe;
                set_sim_info(&si);
                return si;
            }
            break;

        case SIMULATOREXE_F1_2022:
            int error1 = 0;
            if(*setup_udp != NULL)
            {
                error1 = (*setup_udp)(20777);
            }
            error1 = simapi_initudp(simdata, simmap, SIMULATORAPI_F1_2018);

            if(error1 == 0)
            {
                simdata->simon = true;
                simdata->simapi = SIMULATORAPI_F1_2018;
                simdata->simexe = simexe;

                simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;
                simdata->gear = 0;
                simdata->velocity = 0;
                simdata->rpms = 0;
                simdata->altitude = 0;

                si.isSimOn = true;
                si.simulatorapi = simdata->simapi;
                si.mapapi = si.simulatorapi;
                si.simulatorexe = simdata->simexe;
                set_sim_info(&si);
                return si;
            }
            break;

        case SIMULATOREXE_RFACTOR2:
        case SIMULATOREXE_LEMANS_ULTIMATE:
            if (does_sim_file_exist("/dev/shm/$rFactor2SMMP_Telemetry$"))
            {
                simapi_log(SIMAPI_LOGLEVEL_DEBUG, "RFactor2 telemetry file found");
                si.simulatorapi = SIMULATORAPI_RFACTOR2;
                int error = simapi_init(simdata, simmap, SIMULATORAPI_RFACTOR2, simexe);
                simapi_datamap(simdata, simmap, SIMULATORAPI_RFACTOR2, false, NULL);
                if (error == 0)
                {
                    //slogi("found Assetto Corsa, starting application...");

                    simdata->simon = true;
                    simdata->simapi = SIMULATORAPI_RFACTOR2;
                    simdata->simexe = simexe;

                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.mapapi = si.simulatorapi;
                    si.simulatorexe = simdata->simexe;
                    set_sim_info(&si);

                    return si;
                }
            }
            break;

        case SIMULATOREXE_AUTOMOBILISTA2:
        case SIMULATOREXE_AUTOMOBILISTA2_DEMO:
            if (force_udp == false)
            {
                if (does_sim_file_exist("/dev/shm/$pcars2$"))
                {
                    si.simulatorapi = SIMULATORAPI_PROJECTCARS2;
                    int error = simapi_init(simdata, simmap, SIMULATORAPI_PROJECTCARS2, simexe);
                    simapi_datamap(simdata, simmap, SIMULATORAPI_PROJECTCARS2, false, NULL);
                    if (error == 0)
                    {
                        simdata->simon = true;
                        simdata->simapi = SIMULATORAPI_PROJECTCARS2;
                        simdata->simexe = simexe;

                        si.isSimOn = true;
                        si.simulatorapi = simdata->simapi;
                        si.mapapi = si.simulatorapi;
                        si.simulatorexe = simdata->simexe;
                        set_sim_info(&si);

                        return si;
                    }
                }
            }
            else
            {
                int error = (*setup_udp)(5606);
                error = simapi_initudp(simdata, simmap, SIMULATORAPI_PROJECTCARS2);
                if (error == 0)
                {
                    si.simulatorapi = SIMULATORAPI_PROJECTCARS2;
                    si.SimUsesUDP = true;
                    simapi_datamap(simdata, simmap, SIMULATORAPI_PROJECTCARS2, true, NULL);
                }
                if (error == 0 && simdata->simstatus > 1)
                {
                    simdata->simon = true;
                    simdata->simapi = SIMULATORAPI_PROJECTCARS2;
                    simdata->simexe = simexe;

                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.mapapi = si.simulatorapi;
                    si.simulatorexe = simdata->simexe;
                    set_sim_info(&si);

                    return si;
                }
            }
            break;
        case SIMULATOREXE_EUROTRUCKS2:
        case SIMULATOREXE_AMERICANTRUCKS:
            if (does_sim_file_exist("/dev/shm/SCS/SCSTelemetry"))
            {
                si.simulatorapi = SIMULATORAPI_SCSTRUCKSIM2;
                int error = simapi_init(simdata, simmap, SIMULATORAPI_SCSTRUCKSIM2, simexe);
                simapi_datamap(simdata, simmap, SIMULATORAPI_SCSTRUCKSIM2, false, NULL);
                if (error == 0)
                {
                    simdata->simon = true;
                    simdata->simapi = SIMULATORAPI_SCSTRUCKSIM2;
                    simdata->simexe = simexe;

                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.mapapi = si.simulatorapi;
                    si.simulatorexe = simdata->simexe;
                    set_sim_info(&si);

                    return si;
                }
            }
            break;
        case SIMULATOREXE_DIRT_RALLY_2:
            simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Found running process for DiRT Rally 2.0");
            int dr2_error = (*setup_udp)(20777);
            if (dr2_error == 0)
            {
                dr2_error = simapi_initudp(simdata, simmap, SIMULATORAPI_DIRT_RALLY_2);
            }

            if (dr2_error == 0)
            {
                simdata->simon = true;
                simdata->simapi = SIMULATORAPI_DIRT_RALLY_2;
                simdata->simexe = simexe;

                simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;

                si.isSimOn = true;
                si.SimUsesUDP = true;
                si.simulatorapi = simdata->simapi;
                si.mapapi = si.simulatorapi;
                si.simulatorexe = simdata->simexe;
                set_sim_info(&si);

                return si;
            }
            break;
        case SIMULATOREXE_WRECKFEST2:
            simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Found running process for Wreckfest 2");
            int wf2_error = 0;
            if (*setup_udp != NULL)
            {
                wf2_error = (*setup_udp)(23123);
            }

            if (wf2_error == 0)
            {
                wf2_error = simapi_initudp(simdata, simmap, SIMULATORAPI_WRECKFEST2);
            }

            if (wf2_error == 0)
            {
                simdata->simon = true;
                simdata->simapi = SIMULATORAPI_WRECKFEST2;
                simdata->simexe = simexe;
                simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;

                si.isSimOn = true;
                si.SimUsesUDP = true;
                si.simulatorapi = simdata->simapi;
                si.mapapi = si.simulatorapi;
                si.simulatorexe = simdata->simexe;
                set_sim_info(&si);

                return si;
            }
            break;
        case SIMULATOREXE_RICHARD_BURNS_RALLY:
            simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Found running process for Richard Burns Rally");
            int rbr_error = 0;
            if (*setup_udp != NULL)
            {
                rbr_error = (*setup_udp)(6776);
            }

            if (rbr_error == 0)
            {
                rbr_error = simapi_initudp(simdata, simmap, SIMULATORAPI_RICHARD_BURNS_RALLY);
            }

            if (rbr_error == 0)
            {
                simdata->simon = true;
                simdata->simapi = SIMULATORAPI_RICHARD_BURNS_RALLY;
                simdata->simexe = simexe;
                simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;

                si.isSimOn = true;
                si.SimUsesUDP = true;
                si.simulatorapi = simdata->simapi;
                si.mapapi = si.simulatorapi;
                si.simulatorexe = simdata->simexe;
                set_sim_info(&si);

                return si;
            }
            break;
        case SIMULATOREXE_RACE_ROOM:
            simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Found running process for RaceRoom Racing Experience");
            if (does_sim_file_exist("/dev/shm/$R3E"))
            {
                simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Found shared memory file for RaceRoom Racing Experience");
                si.simulatorapi = SIMULATORAPI_RACE_ROOM;
                int race_room_error = simapi_init(simdata, simmap, SIMULATORAPI_RACE_ROOM, simexe);
                simapi_datamap(simdata, simmap, SIMULATORAPI_RACE_ROOM, false, NULL);
                if (race_room_error == 0)
                {
                    simdata->simon = true;
                    simdata->simapi = SIMULATORAPI_RACE_ROOM;
                    simdata->simexe = simexe;

                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.mapapi = si.simulatorapi;
                    si.simulatorexe = simdata->simexe;
                    set_sim_info(&si);

                    return si;
                }
            }
            break;
        case SIMULATOREXE_FORZA_HORIZON_5:
        case SIMULATOREXE_FORZA_HORIZON_6:
            simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Found running process for Forza");
            int forza_error = 0;
            if (*setup_udp != NULL)
            {
                forza_error = (*setup_udp)(5685);
            }

            if (forza_error == 0)
            {
                forza_error = simapi_initudp(simdata, simmap, SIMULATORAPI_FORZA);
            }

            if (forza_error == 0)
            {
                simdata->simon = true;
                simdata->simapi = SIMULATORAPI_FORZA;
                simdata->simexe = simexe;
                simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;

                si.isSimOn = true;
                si.SimUsesUDP = true;
                si.simulatorapi = simdata->simapi;
                si.mapapi = si.simulatorapi;
                si.simulatorexe = simdata->simexe;
                set_sim_info(&si);

                return si;
            }
            break;
    }
    return si;
}

int simapi_datamap(SimData* simdata, SimMap* simmap, SimulatorAPI simulatorapi, bool udp, char* base)
{
    char* a;
    char* b;
    char* c;
    char* d;

    // this will get overwritten by sim mappers
    simdata->car[0] = 'D';
    simdata->car[1] = 'E';
    simdata->car[2] = 'F';
    simdata->car[3] = 'A';
    simdata->car[4] = 'U';
    simdata->car[5] = 'L';
    simdata->car[6] = 'T';
    simdata->car[7] = '\0';
    simdata->prev_mtick = simdata->mtick;
    simdata->mtick = timeInMilliseconds();

    switch ( simulatorapi )
    {
        case SIMULATORAPI_SIMAPI_TEST :
            double tyre0 = simdata->tyrediameter[0];
            double tyre1 = simdata->tyrediameter[1];
            double tyre2 = simdata->tyrediameter[2];
            double tyre3 = simdata->tyrediameter[3];
            memcpy(simdata, simmap->addr, sizeof(SimData));
            if(simdata->simapi != SIMULATORAPI_ASSETTO_CORSA)
            {
                simdata->tyrediameter[0] = tyre0;
                simdata->tyrediameter[1] = tyre1;
                simdata->tyrediameter[2] = tyre2;
                simdata->tyrediameter[3] = tyre3;
            }
            return 0;
        case SIMULATORAPI_ASSETTO_CORSA :

            map_assetto_corsa_data(simdata, simmap, simdata->simexe);

            break;

        case SIMULATORAPI_RFACTOR2 :

            map_rfactor2_data(simdata, simmap);

            break;

        case SIMULATORAPI_PROJECTCARS2 :

            map_project_cars2_data(simdata, simmap, udp, base);

            break;

        case SIMULATORAPI_SCSTRUCKSIM2 :

            map_trucks_data(simdata, simmap);

            break;

        case SIMULATORAPI_OUTSIMOUTGAUGE :

            map_outgauge_outsim_data(simdata, simmap, simdata->simexe, base);
            break;

        case SIMULATORAPI_DIRT_RALLY_2 :

            map_dirt_rally_2_data(simdata, simmap, base);
            break;

        case SIMULATORAPI_F1_2018 :

            map_f1_2018_data(simdata, simmap, base);
            break;

        case SIMULATORAPI_WRECKFEST2 :

            map_wreckfest2_data(simdata, simmap, base);
            break;

        case SIMULATORAPI_RICHARD_BURNS_RALLY :

            map_richard_burns_rally_data(simdata, simmap, base);
            break;

        case SIMULATORAPI_FORZA:
            map_forza_data(simdata, simmap, base);
            break;
        case SIMULATORAPI_RACE_ROOM:

            map_r3e_data(simdata, simmap);
            break;

        case SIMULATORAPI_LMU:

            break;
    }

    if (simmap->addr != NULL)
    {
        simdmap(simmap, simdata);
    }

}

static int simdmap(SimMap* simmap, SimData* simdata)
{
    memcpy(simmap->addr, simdata, sizeof(SimData));
}

int simapi_initudp(SimData* simdata, SimMap* simmap, SimulatorAPI simulator)
{
    int error = SIMAPI_ERROR_NONE;

    simdata->simapiversion = SIMAPI_VERSION;
    return error;
}


int simapi_init(SimData* simdata, SimMap* simmap, SimulatorAPI simulator, SimulatorEXE simexe)
{
    //slogi("searching for simulator data...");
    int error = SIMAPI_ERROR_NONE;

    void* a;
    switch ( simulator )
    {
        case SIMULATORAPI_SIMAPI_TEST :
            if(simmap->hasSimApiDat == true)
            {
                return 0;
            }
            simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Opening universal shared memory api");
            simmap->fd = simapi_platform_shm_open(SIMAPI_MEM_FILE, O_RDONLY, S_IRUSR|S_IWUSR);
            if (simmap->fd == -1)
            {
                return 10;
            }

            simmap->addr = simapi_platform_shm_map(NULL, sizeof(SimData), PROT_READ, MAP_SHARED, simmap->fd, 0);
            if (simmap->addr == MAP_FAILED)
            {
                return 30;
            }
            simmap->hasSimApiDat = true;
            //slogi("found data for monocoque test...");
            break;

        case SIMULATORAPI_ASSETTO_CORSA :

            if(simexe == SIMULATOREXE_ASSETTO_CORSA_EVO)
            {
                if(simmap->ac.has_physics == true)
                {
                    return 0;
                }
                simmap->ac.has_physics=false;
                simmap->ac.has_static=false;
                simmap->ac.fd_physics = simapi_platform_shm_open(ACEVO_PHYSICS_FILE, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
                if (simmap->ac.fd_physics == -1)
                {
                    //slogd("could not open Assetto Corsa physics engine");
                    return SIMAPI_ERROR_NODATA;
                }
                simmap->ac.physics_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->ac.ac_physics), PROT_READ, MAP_SHARED, simmap->ac.fd_physics, 0);
                if (simmap->ac.physics_map_addr == MAP_FAILED)
                {
                    //slogd("could not retrieve Assetto Corsa physics data");
                    return 30;
                }
                simmap->ac.has_physics=true;

                simmap->ac.fd_static = simapi_platform_shm_open(ACEVO_STATIC_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
                if (simmap->ac.fd_static == -1)
                {
                    //slogd("could not open Assetto Corsa static data");
                    return 10;
                }
                simmap->ac.static_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->ac.ac_static), PROT_READ, MAP_SHARED, simmap->ac.fd_static, 0);
                if (simmap->ac.static_map_addr == MAP_FAILED)
                {
                    //slogd("could not retrieve Assetto Corsa static data");
                    return 30;
                }
                simmap->ac.has_static=true;

                simmap->ac.fd_graphic = simapi_platform_shm_open(ACEVO_GRAPHIC_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
                if (simmap->ac.fd_graphic == -1)
                {
                    //slogd("could not open Assetto Corsa graphic data");
                    return 10;
                }
                simmap->ac.graphic_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->ac.ac_graphic), PROT_READ, MAP_SHARED, simmap->ac.fd_graphic, 0);
                if (simmap->ac.graphic_map_addr == MAP_FAILED)
                {
                    //slogd("could not retrieve Assetto Corsa static data");
                    return 30;
                }
                simmap->ac.has_graphic=true;
                //slogi("found data for Assetto Corsa...");
                simmap->ac.fd_crewchief = simapi_platform_shm_open(AC_CREWCHIEF_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
                if (simmap->ac.fd_crewchief == -1)
                {
                    //slogd("could not open Assetto Corsa graphic data");
                    return 10;
                }
                simmap->ac.crewchief_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->ac.ac_crewchief), PROT_READ, MAP_SHARED, simmap->ac.fd_crewchief, 0);
                if (simmap->ac.crewchief_map_addr == MAP_FAILED)
                {
                    //slogd("could not retrieve Assetto Corsa static data");
                    return 30;
                }
                simmap->ac.has_crewchief=true;

                break;

            }

            if(simmap->ac.has_physics == true)
            {
                return 0;
            }
            simmap->ac.has_physics=false;
            simmap->ac.has_static=false;
            simmap->ac.fd_physics = simapi_platform_shm_open(AC_PHYSICS_FILE, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
            if (simmap->ac.fd_physics == -1)
            {
                //slogd("could not open Assetto Corsa physics engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->ac.physics_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->ac.ac_physics), PROT_READ, MAP_SHARED, simmap->ac.fd_physics, 0);
            if (simmap->ac.physics_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa physics data");
                return 30;
            }
            simmap->ac.has_physics=true;

            simmap->ac.fd_static = simapi_platform_shm_open(AC_STATIC_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->ac.fd_static == -1)
            {
                //slogd("could not open Assetto Corsa static data");
                return 10;
            }
            simmap->ac.static_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->ac.ac_static), PROT_READ, MAP_SHARED, simmap->ac.fd_static, 0);
            if (simmap->ac.static_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->ac.has_static=true;

            simmap->ac.fd_graphic = simapi_platform_shm_open(AC_GRAPHIC_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->ac.fd_graphic == -1)
            {
                //slogd("could not open Assetto Corsa graphic data");
                return 10;
            }
            simmap->ac.graphic_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->ac.ac_graphic), PROT_READ, MAP_SHARED, simmap->ac.fd_graphic, 0);
            if (simmap->ac.graphic_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->ac.has_graphic=true;
            //slogi("found data for Assetto Corsa...");
            simmap->ac.fd_crewchief = simapi_platform_shm_open(AC_CREWCHIEF_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->ac.fd_crewchief == -1)
            {
                //slogd("could not open Assetto Corsa graphic data");
                return 10;
            }
            simmap->ac.crewchief_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->ac.ac_crewchief), PROT_READ, MAP_SHARED, simmap->ac.fd_crewchief, 0);
            if (simmap->ac.crewchief_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->ac.has_crewchief=true;

            break;


        case SIMULATORAPI_PROJECTCARS2 :

            if(simmap->pcars2.has_telemetry == true)
            {
                return 0;
            }
            simmap->pcars2.has_telemetry=false;
            simmap->pcars2.fd_telemetry = simapi_platform_shm_open(PCARS2_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->pcars2.fd_telemetry == -1)
            {
                //slogd("could not open Assetto Corsa physics engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->pcars2.telemetry_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->pcars2.pcars2_telemetry), PROT_READ, MAP_SHARED, simmap->pcars2.fd_telemetry, 0);
            if (simmap->pcars2.telemetry_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa physics data");
                return 30;
            }
            simmap->pcars2.has_telemetry=true;

            break;

        case SIMULATORAPI_RACE_ROOM :

            if(simmap->r3e.has_telemetry == true)
            {
                return 0;
            }
            simmap->r3e.has_telemetry=false;
            simmap->r3e.fd_telemetry = simapi_platform_shm_open(R3E_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->r3e.fd_telemetry == -1)
            {
                //slogd("could not open Assetto Corsa physics engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->r3e.telemetry_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->r3e.r3e_telemetry), PROT_READ, MAP_SHARED, simmap->r3e.fd_telemetry, 0);
            if (simmap->r3e.telemetry_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa physics data");
                return 30;
            }
            simmap->r3e.has_telemetry=true;

            break;

        case SIMULATORAPI_RFACTOR2 :

            if(simmap->rf2.has_telemetry == true)
            {
                return 0;
            }
            simmap->rf2.has_telemetry=false;
            simmap->rf2.has_scoring=false;
            simmap->rf2.fd_telemetry = simapi_platform_shm_open(RF2_TELEMETRY_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->rf2.fd_telemetry == -1)
            {
                //slogd("could not open RFactor2 Telemetry engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->rf2.telemetry_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->rf2.rf2_telemetry), PROT_READ, MAP_SHARED, simmap->rf2.fd_telemetry, 0);
            if (simmap->rf2.telemetry_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve RFactor2 telemetry data");
                return 30;
            }
            simmap->rf2.has_telemetry=true;
            simmap->rf2.fd_scoring = simapi_platform_shm_open(RF2_SCORING_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->rf2.fd_scoring == -1)
            {
                //slogd("could not open RFactor2 Telemetry engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->rf2.scoring_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->rf2.rf2_scoring), PROT_READ, MAP_SHARED, simmap->rf2.fd_scoring, 0);
            if (simmap->rf2.scoring_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve RFactor2 telemetry data");
                return 30;
            }
            simmap->rf2.has_scoring=true;


            //slogi("found data for RFactor2...");
            break;

        case SIMULATORAPI_SCSTRUCKSIM2 :

            if(simmap->scs2.has_telemetry == true)
            {
                return 0;
            }
            simmap->scs2.has_telemetry=false;
#ifdef _WIN32
            simmap->scs2.fd_telemetry = simapi_platform_shm_open(SCS2_TELEMETRY_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
#else
            simmap->scs2.fd_telemetry = simapi_platform_file_open("/dev/shm/SCS/SCSTelemetry", O_CREAT|O_RDWR, 0777);
#endif
            //simmap->scs2.fd_telemetry = simapi_platform_shm_open(SCS2_TELEMETRY_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->scs2.fd_telemetry == -1)
            {
                return SIMAPI_ERROR_NODATA;
            }
            simmap->scs2.telemetry_map_addr = simapi_platform_shm_map(NULL, sizeof(simmap->scs2.scs2_telemetry), PROT_READ, MAP_SHARED, simmap->scs2.fd_telemetry, 0);
            if (simmap->scs2.telemetry_map_addr == MAP_FAILED)
            {
                return 30;
            }
            simmap->scs2.has_telemetry=true;

            break;
    }
    simdata->simapiversion = SIMAPI_VERSION;

    return error;
}


int simapi_sim_clear(SimData* simdata, SimMap* simmap, bool issimd)
{
    int error = SIMAPI_ERROR_NONE;

    if(simmap->ac.has_physics==true)
    {
        if (simapi_platform_shm_unmap(simmap->ac.physics_map_addr, sizeof(simmap->ac.ac_physics)) == -1)
        {
            return 0101100;
        }

        if (simapi_platform_shm_close(simmap->ac.fd_physics) == -1)
        {
            return 0101200;
        }

        simmap->ac.has_physics = false;
    }
    if(simmap->ac.has_static==true)
    {
        if (simapi_platform_shm_unmap(simmap->ac.static_map_addr, sizeof(simmap->ac.ac_static)) == -1)
        {
            return 0102100;
        }

        if (simapi_platform_shm_close(simmap->ac.fd_static) == -1)
        {
            return 0102200;
        }

        simmap->ac.has_static = false;
    }

    if(simmap->ac.has_graphic==true)
    {
        if (simapi_platform_shm_unmap(simmap->ac.graphic_map_addr, sizeof(simmap->ac.ac_graphic)) == -1)
        {
            return 0103100;
        }

        if (simapi_platform_shm_close(simmap->ac.fd_graphic) == -1)
        {
            return 0103200;
        }

        simmap->ac.has_graphic = false;
    }

    if(simmap->ac.has_crewchief==true)
    {
        if (simapi_platform_shm_unmap(simmap->ac.crewchief_map_addr, sizeof(simmap->ac.ac_crewchief)) == -1)
        {
            return 0104100;
        }

        if (simapi_platform_shm_close(simmap->ac.fd_crewchief) == -1)
        {
            return 0104200;
        }

        simmap->ac.has_crewchief = false;
    }

    if(simmap->pcars2.has_telemetry==true)
    {
        if (simapi_platform_shm_unmap(simmap->pcars2.telemetry_map_addr, sizeof(simmap->pcars2.pcars2_telemetry)) == -1)
        {
            return 0201100;
        }

        if (simapi_platform_shm_close(simmap->pcars2.fd_telemetry) == -1)
        {
            return 0201200;
        }

        simmap->pcars2.has_telemetry = false;
    }

    if(simmap->r3e.has_telemetry==true)
    {
        if (simapi_platform_shm_unmap(simmap->r3e.telemetry_map_addr, sizeof(simmap->r3e.r3e_telemetry)) == -1)
        {
            return 0301100;
        }

        if (simapi_platform_shm_close(simmap->r3e.fd_telemetry) == -1)
        {
            return 0301200;
        }

        simmap->r3e.has_telemetry = false;
    }

    if(simmap->rf2.has_telemetry==true)
    {
        if (simapi_platform_shm_unmap(simmap->rf2.telemetry_map_addr, sizeof(simmap->rf2.rf2_telemetry)) == -1)
        {
            return 100;
        }

        if (simapi_platform_shm_close(simmap->rf2.fd_telemetry) == -1)
        {
            return 200;
        }

        simmap->rf2.has_telemetry = false;
    }
    if(simmap->rf2.has_scoring==true)
    {
        if (simapi_platform_shm_unmap(simmap->rf2.scoring_map_addr, sizeof(simmap->rf2.rf2_scoring)) == -1)
        {
            return 100;
        }

        if (simapi_platform_shm_close(simmap->rf2.fd_scoring) == -1)
        {
            return 200;
        }

        simmap->rf2.has_scoring = false;
    }

    if(simmap->scs2.has_telemetry==true)
    {
        if (simapi_platform_shm_unmap(simmap->scs2.telemetry_map_addr, sizeof(simmap->scs2.scs2_telemetry)) == -1)
        {
            return 100;
        }

        if (simapi_platform_shm_close(simmap->scs2.fd_telemetry) == -1)
        {
            return 200;
        }

        simmap->scs2.has_telemetry = false;
    }

    if (simmap->dirt2.has_telemetry == true)
    {
        simmap->dirt2.has_telemetry = false;
    }

    if (issimd == true && simmap != NULL && simmap->addr != NULL)
    {
        memset(simdata, 0, sizeof(SimData));
        simdata->simapiversion = SIMAPI_VERSION;
        simdmap(simmap, simdata);
    }
    return error;
}

int simapi_universalmap_free(SimMap* simmap)
{
    simapi_log(SIMAPI_LOGLEVEL_INFO, "Freeing universal shared memory");

    if(simmap->fd == -1)
    {
        free(simmap);
        return 0;
    }

    if (simapi_platform_shm_unmap(simmap->addr, sizeof(SimData)) == -1)
    {
        return 100;
    }
    simapi_platform_shm_unlink(SIMAPI_MEM_FILE);

    if (simapi_platform_shm_close(simmap->fd) == -1)
    {
        return 200;
    }
    simmap->hasSimApiDat = false;
    return 0;
}

int simapi_universalmap_open(SimMap* simmap, SimData* simdata)
{
    simmap->fd = simapi_platform_shm_open(SIMAPI_MEM_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    simapi_log(SIMAPI_LOGLEVEL_INFO, "Opening universal shared memory");
    if (simmap->fd == -1)
    {
        printf("open");
        return 10;
    }
    int res = simapi_platform_shm_resize(simmap->fd, sizeof(SimData));
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    void* addr = simapi_platform_shm_map(NULL, sizeof(SimData), PROT_WRITE, MAP_SHARED, simmap->fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    simmap->addr = addr;

    simdata->simapiversion = SIMAPI_VERSION;
    simdmap(simmap, simdata);

    return 0;
}


int simapi_compatmap_open(SimCompatMap* compatmap)
{
    compatmap->pcars2_fd = simapi_platform_shm_open(PCARS2_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->pcars2_fd == -1)
    {
        printf("open");
        return 10;
    }
    int res = simapi_platform_shm_resize(compatmap->pcars2_fd, PCARS2_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    void* addr = simapi_platform_shm_map(NULL, PCARS2_SIZE, PROT_WRITE, MAP_SHARED, compatmap->pcars2_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->pcars2_addr = addr;

    compatmap->r3e_fd = simapi_platform_shm_open(R3E_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->r3e_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = simapi_platform_shm_resize(compatmap->r3e_fd, R3E_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = simapi_platform_shm_map(NULL, R3E_SIZE, PROT_WRITE, MAP_SHARED, compatmap->r3e_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->r3e_addr = addr;

    compatmap->lmu_fd = simapi_platform_shm_open(LMU_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->lmu_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = simapi_platform_shm_resize(compatmap->lmu_fd, LMU_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = simapi_platform_shm_map(NULL, LMU_SIZE, PROT_WRITE, MAP_SHARED, compatmap->lmu_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->lmu_addr = addr;

    compatmap->acphysics_fd = simapi_platform_shm_open(AC_PHYSICS_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acphysics_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = simapi_platform_shm_resize(compatmap->acphysics_fd, AC_PHYSICS_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = simapi_platform_shm_map(NULL, AC_PHYSICS_SIZE, PROT_WRITE, MAP_SHARED, compatmap->acphysics_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->acphysics_addr = addr;

    compatmap->acgraphics_fd = simapi_platform_shm_open(AC_GRAPHIC_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acgraphics_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = simapi_platform_shm_resize(compatmap->acgraphics_fd, AC_GRAPHIC_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = simapi_platform_shm_map(NULL, AC_GRAPHIC_SIZE, PROT_WRITE, MAP_SHARED, compatmap->acgraphics_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->acgraphics_addr = addr;

    compatmap->acstatic_fd = simapi_platform_shm_open(AC_STATIC_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acstatic_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = simapi_platform_shm_resize(compatmap->acstatic_fd, AC_STATIC_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = simapi_platform_shm_map(NULL, AC_STATIC_SIZE, PROT_WRITE, MAP_SHARED, compatmap->acstatic_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->acstatic_addr = addr;

    compatmap->acevophysics_fd = simapi_platform_shm_open(ACEVO_PHYSICS_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acevophysics_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = simapi_platform_shm_resize(compatmap->acevophysics_fd, AC_PHYSICS_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = simapi_platform_shm_map(NULL, AC_PHYSICS_SIZE, PROT_WRITE, MAP_SHARED, compatmap->acevophysics_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->acevophysics_addr = addr;

    compatmap->acevographics_fd = simapi_platform_shm_open(ACEVO_GRAPHIC_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acevographics_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = simapi_platform_shm_resize(compatmap->acevographics_fd, AC_GRAPHIC_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = simapi_platform_shm_map(NULL, AC_GRAPHIC_SIZE, PROT_WRITE, MAP_SHARED, compatmap->acevographics_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->acevographics_addr = addr;

    compatmap->acevostatic_fd = simapi_platform_shm_open(ACEVO_STATIC_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acevostatic_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = simapi_platform_shm_resize(compatmap->acevostatic_fd, AC_STATIC_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = simapi_platform_shm_map(NULL, AC_STATIC_SIZE, PROT_WRITE, MAP_SHARED, compatmap->acevostatic_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->acevostatic_addr = addr;

    compatmap->accrew_fd = simapi_platform_shm_open(AC_CREWCHIEF_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->accrew_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = simapi_platform_shm_resize(compatmap->accrew_fd, AC_CREWCHIEF_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = simapi_platform_shm_map(NULL, AC_CREWCHIEF_SIZE, PROT_WRITE, MAP_SHARED, compatmap->accrew_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->accrew_addr = addr;
    return 0;
}

int simapi_compatmap_clear(SimCompatMap* compatmap)
{
    memset(compatmap->acphysics_addr, 0, AC_PHYSICS_SIZE);
    memset(compatmap->acstatic_addr, 0, AC_STATIC_SIZE);
    memset(compatmap->acgraphics_addr, 0, AC_GRAPHIC_SIZE);
    memset(compatmap->acevophysics_addr, 0, AC_PHYSICS_SIZE);
    memset(compatmap->acevostatic_addr, 0, AC_STATIC_SIZE);
    memset(compatmap->acevographics_addr, 0, AC_GRAPHIC_SIZE);
    memset(compatmap->accrew_addr, 0, AC_CREWCHIEF_SIZE);
    memset(compatmap->r3e_addr, 0, R3E_SIZE);
    memset(compatmap->pcars2_addr, 0, PCARS2_SIZE);
    memset(compatmap->lmu_addr, 0, LMU_SIZE);

    return 0;
}


int simapi_compatmap_free(SimCompatMap* compatmap)
{
    if (simapi_platform_shm_unmap(compatmap->acphysics_addr, AC_PHYSICS_SIZE) == -1)
    {
        return 100;
    }
    simapi_platform_shm_unlink(AC_PHYSICS_FILE);
    if (simapi_platform_shm_close(compatmap->acphysics_fd) == -1)
    {
        return 200;
    }

    if (simapi_platform_shm_unmap(compatmap->acstatic_addr, AC_STATIC_SIZE) == -1)
    {
        return 100;
    }
    simapi_platform_shm_unlink(AC_STATIC_FILE);
    if (simapi_platform_shm_close(compatmap->acstatic_fd) == -1)
    {
        return 200;
    }

    if (simapi_platform_shm_unmap(compatmap->acgraphics_addr, AC_GRAPHIC_SIZE) == -1)
    {
        return 100;
    }
    simapi_platform_shm_unlink(AC_GRAPHIC_FILE);
    if (simapi_platform_shm_close(compatmap->acevographics_fd) == -1)
    {
        return 200;
    }

    if (simapi_platform_shm_unmap(compatmap->acevophysics_addr, AC_PHYSICS_SIZE) == -1)
    {
        return 100;
    }
    simapi_platform_shm_unlink(ACEVO_PHYSICS_FILE);
    if (simapi_platform_shm_close(compatmap->acevophysics_fd) == -1)
    {
        return 200;
    }

    if (simapi_platform_shm_unmap(compatmap->acevostatic_addr, AC_STATIC_SIZE) == -1)
    {
        return 100;
    }
    simapi_platform_shm_unlink(ACEVO_STATIC_FILE);
    if (simapi_platform_shm_close(compatmap->acevostatic_fd) == -1)
    {
        return 200;
    }

    if (simapi_platform_shm_unmap(compatmap->acevographics_addr, AC_GRAPHIC_SIZE) == -1)
    {
        return 100;
    }
    simapi_platform_shm_unlink(ACEVO_GRAPHIC_FILE);
    if (simapi_platform_shm_close(compatmap->acevographics_fd) == -1)
    {
        return 200;
    }

    if (simapi_platform_shm_unmap(compatmap->accrew_addr, AC_CREWCHIEF_SIZE) == -1)
    {
        return 100;
    }
    simapi_platform_shm_unlink(AC_CREWCHIEF_FILE);
    if (simapi_platform_shm_close(compatmap->accrew_fd) == -1)
    {
        return 200;
    }

    if (simapi_platform_shm_unmap(compatmap->pcars2_addr, PCARS2_SIZE) == -1)
    {
        return 100;
    }
    simapi_platform_shm_unlink(PCARS2_FILE);
    if (simapi_platform_shm_close(compatmap->pcars2_fd) == -1)
    {
        return 200;
    }

    if (simapi_platform_shm_unmap(compatmap->r3e_addr, R3E_SIZE) == -1)
    {
        return 100;
    }
    simapi_platform_shm_unlink(R3E_FILE);
    if (simapi_platform_shm_close(compatmap->r3e_fd) == -1)
    {
        return 200;
    }

    if (simapi_platform_shm_unmap(compatmap->lmu_addr, LMU_SIZE) == -1)
    {
        return 100;
    }
    simapi_platform_shm_unlink(LMU_FILE);
    if (simapi_platform_shm_close(compatmap->lmu_fd) == -1)
    {
        return 200;
    }

    return 0;
}

/* Windows shared-memory and time support. */
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Win32 compatibility prototypes are declared at the top of this file. */

/* Avoid truncating 64-bit Win32 HANDLEs into the existing int fd fields.
 * Descriptor values 3..130 are private to simapi; not Windows CRT fds.
 */
#define SIMAPI_PLATFORM_MAX_MAPS 128
#define SIMAPI_PLATFORM_NAME_MAX 256

typedef struct {
    int in_use;
    int create_if_missing;
    int writable;
    HANDLE mapping;
    char name[SIMAPI_PLATFORM_NAME_MAX];
} simapi_platform_slot;

static SRWLOCK simapi_maps_lock = SRWLOCK_INIT;
static simapi_platform_slot simapi_maps[SIMAPI_PLATFORM_MAX_MAPS];

/* /dev/shm/name is the Linux name. Windows shared memory is a named
 * section object. Local\\name denotes the current login session.
 * Do not strip or synthesize an already-qualified Global\\ name.
 */
static int simapi_normalize_name(const char *src, char *dst, size_t cap)
{
    const char *prefix = "Local\\";
    if (!src || !*src || !dst || cap == 0) return -1;
    if (strncmp(src, "/dev/shm/", 9) == 0) src += 9;
    while (*src == '/') src++;
    if (!*src) return -1;
    if (strncmp(src, "Local\\", 6) == 0 ||
        strncmp(src, "Global\\", 7) == 0)
        prefix = "";
    if (snprintf(dst, cap, "%s%s", prefix, src) >= (int)cap) return -1;
    return 0;
}

static int simapi_find_free_slot(void)
{
    for (int i = 0; i < SIMAPI_PLATFORM_MAX_MAPS; i++)
        if (!simapi_maps[i].in_use) return i;
    return -1;
}

int simapi_platform_shm_exists(const char *name)
{
    char full[SIMAPI_PLATFORM_NAME_MAX];
    if (simapi_normalize_name(name, full, sizeof(full)) != 0) return 0;
    HANDLE h = OpenFileMappingA(FILE_MAP_READ, FALSE, full);
    if (h == NULL) return 0;
    CloseHandle(h);
    return 1;
}

int simapi_platform_shm_open(const char *name, int flags, int mode)
{
    (void)mode;
    char full[SIMAPI_PLATFORM_NAME_MAX];
    if (simapi_normalize_name(name, full, sizeof(full)) != 0) {
        errno = EINVAL;
        return -1;
    }
    int writable = ((flags & O_RDWR) != 0);
    DWORD desired_access = writable ? FILE_MAP_WRITE : FILE_MAP_READ;
    HANDLE mapping = OpenFileMappingA(desired_access, FALSE, full);
    /* For simulator readers O_CREAT must NOT accidentally create a blank
     * section: the name is deferred until an explicit ftruncate, which is
     * only used by simapi's own mapping producers.
     */
    if (!mapping && !(flags & O_CREAT)) {
        errno = ENOENT;
        return -1;
    }

    AcquireSRWLockExclusive(&simapi_maps_lock);
    int i = simapi_find_free_slot();
    if (i < 0) {
        ReleaseSRWLockExclusive(&simapi_maps_lock);
        if (mapping) CloseHandle(mapping);
        errno = EMFILE;
        return -1;
    }
    simapi_platform_slot *s = &simapi_maps[i];
    memset(s, 0, sizeof(*s));
    s->in_use = 1;
    s->create_if_missing = !!(flags & O_CREAT);
    s->writable = writable;
    s->mapping = mapping;
    memcpy(s->name, full, strlen(full) + 1);
    ReleaseSRWLockExclusive(&simapi_maps_lock);
    return i + 3;
}

int simapi_platform_shm_resize(int fd, size_t length)
{
    const int i = fd - 3;
    if (i < 0 || i >= SIMAPI_PLATFORM_MAX_MAPS || length == 0) {
        errno = EINVAL;
        return -1;
    }
    AcquireSRWLockExclusive(&simapi_maps_lock);
    simapi_platform_slot *s = &simapi_maps[i];
    if (!s->in_use || !s->create_if_missing) {
        ReleaseSRWLockExclusive(&simapi_maps_lock);
        errno = EBADF;
        return -1;
    }
    /* Windows cannot resize an existing named section. The creating party
     * already owns its size; leave it alone. Mapping will fail if too small.
     */
    if (s->mapping == NULL) {
        unsigned long long len64 = (unsigned long long)length;
        s->mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL,
            PAGE_READWRITE, (DWORD)(len64 >> 32), (DWORD)len64, s->name);
    }
    int ok = (s->mapping != NULL);
    ReleaseSRWLockExclusive(&simapi_maps_lock);
    if (!ok) errno = EIO;
    return ok ? 0 : -1;
}

void *simapi_platform_shm_map(void *hint, size_t length, int prot,
                       int flags, int fd, size_t offset)
{
    (void)hint;
    if (flags != MAP_SHARED || length == 0) {
        errno = EINVAL;
        return MAP_FAILED;
    }
    const int i = fd - 3;
    if (i < 0 || i >= SIMAPI_PLATFORM_MAX_MAPS) {
        errno = EBADF;
        return MAP_FAILED;
    }
    AcquireSRWLockShared(&simapi_maps_lock);
    simapi_platform_slot *s = &simapi_maps[i];
    HANDLE h = s->in_use ? s->mapping : NULL;
    if (h == NULL) {
        ReleaseSRWLockShared(&simapi_maps_lock);
        errno = ENOENT;
        return MAP_FAILED;
    }
    DWORD access = (prot & PROT_WRITE) ? FILE_MAP_WRITE : FILE_MAP_READ;
    unsigned long long off64 = (unsigned long long)offset;
    void *view = MapViewOfFile(h, access,
                              (DWORD)(off64 >> 32), (DWORD)off64, length);
    ReleaseSRWLockShared(&simapi_maps_lock);
    if (!view) errno = EIO;
    return view ? view : MAP_FAILED;
}

int simapi_platform_shm_unmap(void *address, size_t length)
{
    (void)length;
    if (!address || address == MAP_FAILED) {
        errno = EINVAL;
        return -1;
    }
    if (!UnmapViewOfFile(address)) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int simapi_platform_shm_close(int fd)
{
    const int i = fd - 3;
    if (i < 0 || i >= SIMAPI_PLATFORM_MAX_MAPS) {
        errno = EBADF;
        return -1;
    }
    AcquireSRWLockExclusive(&simapi_maps_lock);
    simapi_platform_slot *s = &simapi_maps[i];
    if (!s->in_use) {
        ReleaseSRWLockExclusive(&simapi_maps_lock);
        errno = EBADF;
        return -1;
    }
    HANDLE mapping = s->mapping;
    memset(s, 0, sizeof(*s));
    ReleaseSRWLockExclusive(&simapi_maps_lock);
    if (mapping && !CloseHandle(mapping)) {
        errno = EIO;
        return -1;
    }
    return 0;
}

int simapi_platform_shm_unlink(const char *name)
{
    (void)name;
    /* Windows section objects have no unlink operation. They disappear when
     * every mapping HANDLE and view is closed. This differs from POSIX.
     */
    return 0;
}

int simapi_platform_asprintf(char **out, const char *format, ...)
{
    if (!out || !format) return -1;
    *out = NULL;
    va_list ap;
    va_start(ap, format);
    int n = _vscprintf(format, ap);
    va_end(ap);
    if (n < 0 || n == INT_MAX) return -1;
    char *buf = (char *)malloc((size_t)n + 1);
    if (!buf) return -1;
    va_start(ap, format);
    int written = vsnprintf(buf, (size_t)n + 1, format, ap);
    va_end(ap);
    if (written < 0) {
        free(buf);
        return -1;
    }
    *out = buf;
    return written;
}

long long simapi_platform_time_ms(void)
{
    return (long long)GetTickCount64();
}
int simapi_platform_file_open(const char *path, int flags, int mode)
{
    (void)path; (void)flags; (void)mode;
    errno = ENOSYS;
    return -1;
}

#endif /* _WIN32 */
