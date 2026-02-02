#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>
#include <sys/time.h>

#include "simdata.h"
#include "simapi.h"
#include "simmapper.h"
#include "test.h"
#include "ac.h"
#include "rf2.h"
#include "pcars2.h"
#include "scs2.h"
#include "outgauge.h"
#include "dirt2.h"
#include "f1.h"
#include "wreckfest2.h"
#include "simmap.h"

#include <sys/stat.h>
#include <sys/types.h>
#if defined(OS_WIN)
#include <windows.h>
#else
#include <dirent.h> // for *Nix directory access
#include <unistd.h>
#endif

#include "getpid.h"
#include "../include/f12018.h"
#include "../include/acdata.h"
#include "../include/rf2data.h"
#include "../include/pcars2data.h"
#include "../include/outgauge.h"
#include "../include/scs2data.h"


SimMap* createSimMap()
{
    SimMap* ptr = malloc(sizeof(SimMap));
    ptr->fd = -1;
    ptr->addr = 0;
    return ptr;
}

void* getSimMapPtr(SimMap* simmap)
{
    return simmap->addr;
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
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

bool does_sim_need_bridge(SimulatorEXE s)
{
    if(s == SIMULATOREXE_ASSETTO_CORSA || s == SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE || s == SIMULATOREXE_ASSETTO_CORSA_EVO || s == SIMULATOREXE_ASSETTO_CORSA_RALLY)
    {
        return true;
    }

    if(s == SIMULATOREXE_AUTOMOBILISTA2)
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
    else if (sstrcicmp(game, "r3e") == 0)
    {
        sim = SIMULATOREXE_RACE_ROOM;
    }
    else if (sstrcicmp(game, "wf2") == 0)
    {
        sim = SIMULATOREXE_WRECKFEST2;
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
        case SIMULATOREXE_RACE_ROOM:
            return "r3e";
        case SIMULATOREXE_WRECKFEST2:
            return "wf2";
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
        case SIMULATOREXE_RACE_ROOM:
            return "Race Room";
        case SIMULATOREXE_WRECKFEST2:
            return "Wreckfest 2";
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

void set_simapi_log_info(func_ptr_t func)
{
    loginfo = func;
}
void set_simapi_log_debug(func_ptr_t func)
{
    logdebug = func;
}
void set_simapi_log_trace(func_ptr_t func)
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

void SetProximityData(SimData* simdata, int cars, int8_t lr_flip)
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


bool does_sim_file_exist(const char* file)
{
    if (file == NULL)
    {
        return false;
    }
#if defined(OS_WIN)
#if defined(WIN_API)
    // if you want the WinAPI, versus CRT
    if (strnlen(file, MAX_PATH+1) > MAX_PATH)
    {
        // ... throw error here or ...
        return false;
    }
    DWORD res = GetFileAttributesA(file);
    return (res != INVALID_FILE_ATTRIBUTES &&
            !(res & FILE_ATTRIBUTE_DIRECTORY));
#else
    // Use Win CRT
    struct stat fi;
    if (_stat(file, &fi) == 0)
    {
#if defined(S_ISSOCK)
        // sockets come back as a 'file' on some systems
        // so make sure it's not a socket or directory
        // (in other words, make sure it's an actual file)
        return !(S_ISDIR(fi.st_mode)) &&
               !(S_ISSOCK(fi.st_mode));
#else
        return !(S_ISDIR(fi.st_mode));
#endif
    }
    return false;
#endif
#else
    struct stat fi;
    if (stat(file, &fi) == 0)
    {
#if defined(S_ISSOCK)
        return !(S_ISDIR(fi.st_mode)) &&
               !(S_ISSOCK(fi.st_mode));
#else
        return !(S_ISDIR(fi.st_mode));
#endif
    }
    return false;
#endif
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

int setSimInfo(SimInfo* si)
{
    switch ( si->simulatorapi )
    {

        case SIMULATORAPI_ASSETTO_CORSA :
            si->SimUsesUDP = false;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = true;
            si->SimSupportsAdvancedUI = true;
            break;
        case SIMULATORAPI_RFACTOR2 :
            si->SimUsesUDP = false;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = true;
            si->SimSupportsAdvancedUI = true;
            break;
        case SIMULATORAPI_PROJECTCARS2 :
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = true;
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
            break;
        case SIMULATORAPI_DIRT_RALLY_2 :
            si->SimUsesUDP = true;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = true;
            si->SimSupportsAdvancedUI = false;
            break;
        case SIMULATORAPI_F1_2018:
            si->SimUsesUDP = true;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = false;
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

SimulatorEXE getSimExe(SimInfo* si)
{
    int pid = 0;

    pid = IsProcessRunning(AC_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_ASSETTO_CORSA;
    }
    pid = IsProcessRunning(ACC_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE;
    }
    pid = IsProcessRunning(ACE_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_ASSETTO_CORSA_EVO;
    }
    pid = IsProcessRunning(ACR_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_ASSETTO_CORSA_RALLY;
    }
    pid = IsProcessRunning(RFACTOR2_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_RFACTOR2;
    }
    pid = IsProcessRunning(AMS2_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_AUTOMOBILISTA2;
    }
    pid = IsProcessRunning(EUROTRUCKS2_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_EUROTRUCKS2;
    }
    pid = IsProcessRunning(AMERICANTRUCKS_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_AMERICANTRUCKS;
    }
    pid = IsProcessRunning(LEMANS_ULTIMATE_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_LEMANS_ULTIMATE;
    }
    pid = IsProcessRunning(LIVE_FOR_SPEED_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_LIVE_FOR_SPEED;
    }
    pid = IsProcessRunning(BEAMNG_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_BEAMNG;
    }
    pid = IsProcessRunning(DIRT_RALLY_2_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_DIRT_RALLY_2;
    }
    pid = IsProcessRunning(F1_2022_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_F1_2022;
    }
    pid = IsProcessRunning(WRECKFEST2_EXE);
    if(pid>0)
    {
        si->pid = pid;
        return SIMULATOREXE_WRECKFEST2;
    }
    //pid = IsProcessRunning(FORZA_HORIZON_5_EXE);
    //if(pid>0)
    //{
    //    si->pid = pid;
    //    return SIMULATOREXE_FORZA_HORIZON_5;
    //}
    //pid = IsProcessRunning(RACE_ROOM_EXE);
    //if(pid>0)
    //{
    //    si->pid = pid;
    //    return SIMULATOREXE_RACE_ROOM;
    //}
    return SIMULATOREXE_SIMAPI_TEST_NONE;
}


SimInfo getSim(SimData* simdata, SimMap* simmap, bool force_udp, int (*setup_udp)(int), bool simd)
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


    if(simd == false)
    {
        if (does_sim_file_exist("/dev/shm/SIMAPI.DAT"))
        {
            int e = siminit(simdata, simmap, SIMULATORAPI_SIMAPI_TEST);
            simdatamap(simdata, simmap, NULL, SIMULATORAPI_SIMAPI_TEST, false, NULL);
            char* temp;
            asprintf(&temp, "found running simapi daemon simint error %i", e);
            simapi_log(SIMAPI_LOGLEVEL_TRACE, temp);
            free(temp);
            //simdatamap(simdata, simmap, NULL, SIMULATORAPI_SIMAPI_TEST, false, NULL);
            if(simdata->simapiversion == SIMAPI_VERSION)
            {
                if (simdata->simon == 1)
                {
                    simapi_log(SIMAPI_LOGLEVEL_TRACE, "status okay");
                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.simulatorexe = simdata->simexe;
                    si.mapapi = SIMULATORAPI_SIMAPI_TEST;
                    si.simulatorexe = simdata->simexe;
                    setSimInfo(&si);
                    si.SimUsesUDP = false;
                }
                return si;
            }
            else
            {
                char* temp;
                asprintf(&temp, "skipping sim api daemon due to version mismatch. Daemon Version: %i. App Version: %i", simdata->simapiversion, SIMAPI_VERSION);
                simapi_log(SIMAPI_LOGLEVEL_INFO, temp);
                free(temp);
            }
        }
        else
        {
            simapi_log(SIMAPI_LOGLEVEL_TRACE, "looking for running simulators");
        }
    }

    SimulatorEXE simexe = getSimExe(&si);

    switch ( simexe )
    {
        case SIMULATOREXE_ASSETTO_CORSA:
        case SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE:
        case SIMULATOREXE_ASSETTO_CORSA_EVO:
        case SIMULATOREXE_ASSETTO_CORSA_RALLY:
            simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Found running process for Assetto Corsa");
            if (does_sim_file_exist("/dev/shm/acpmf_physics"))
            {
                if (does_sim_file_exist("/dev/shm/acpmf_static"))
                {
                    simapi_log(SIMAPI_LOGLEVEL_DEBUG, "static and physics files found");
                    si.simulatorapi = SIMULATORAPI_ASSETTO_CORSA;
                    int error = siminit(simdata, simmap, SIMULATORAPI_ASSETTO_CORSA);
                    char* temp;
                    asprintf(&temp, "siminit error %i", error);
                    simapi_log(SIMAPI_LOGLEVEL_DEBUG, temp);
                    free(temp);
                    simdatamap(simdata, simmap, NULL, SIMULATORAPI_ASSETTO_CORSA, false, NULL);

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
                        setSimInfo(&si);
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
            error = siminitudp(simdata, simmap, SIMULATORAPI_OUTSIMOUTGAUGE);

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
                setSimInfo(&si);
                return si;
            }
            break;

        case SIMULATOREXE_F1_2022:
            int error1 = 0;
            if(*setup_udp != NULL)
            {
                error1 = (*setup_udp)(30000);
            }
            error1 = siminitudp(simdata, simmap, SIMULATORAPI_F1_2018);

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
                setSimInfo(&si);
                return si;
            }
            break;

        case SIMULATOREXE_RFACTOR2:
        case SIMULATOREXE_LEMANS_ULTIMATE:
            if (does_sim_file_exist("/dev/shm/$rFactor2SMMP_Telemetry$"))
            {
                simapi_log(SIMAPI_LOGLEVEL_DEBUG, "RFactor2 telemetry file found");
                si.simulatorapi = SIMULATORAPI_RFACTOR2;
                int error = siminit(simdata, simmap, SIMULATORAPI_RFACTOR2);
                simdatamap(simdata, simmap, NULL, SIMULATORAPI_RFACTOR2, false, NULL);
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
                    setSimInfo(&si);

                    return si;
                }
            }
            break;

        case SIMULATOREXE_AUTOMOBILISTA2:
            if (force_udp == false)
            {
                if (does_sim_file_exist("/dev/shm/$pcars2$"))
                {
                    si.simulatorapi = SIMULATORAPI_PROJECTCARS2;
                    int error = siminit(simdata, simmap, SIMULATORAPI_PROJECTCARS2);
                    simdatamap(simdata, simmap, NULL, SIMULATORAPI_PROJECTCARS2, false, NULL);
                    if (error == 0)
                    {
                        simdata->simon = true;
                        simdata->simapi = SIMULATORAPI_PROJECTCARS2;
                        simdata->simexe = simexe;

                        si.isSimOn = true;
                        si.simulatorapi = simdata->simapi;
                        si.mapapi = si.simulatorapi;
                        si.simulatorexe = simdata->simexe;
                        setSimInfo(&si);

                        return si;
                    }
                }
            }
            else
            {
                int error = (*setup_udp)(5606);
                error = siminitudp(simdata, simmap, SIMULATORAPI_PROJECTCARS2);
                if (error == 0)
                {
                    si.simulatorapi = SIMULATORAPI_PROJECTCARS2;
                    si.SimUsesUDP = true;
                    simdatamap(simdata, simmap, NULL, SIMULATORAPI_PROJECTCARS2, true, NULL);
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
                    setSimInfo(&si);

                    return si;
                }
            }
            break;
        case SIMULATOREXE_EUROTRUCKS2:
        case SIMULATOREXE_AMERICANTRUCKS:
            if (does_sim_file_exist("/dev/shm/SCS/SCSTelemetry"))
            {
                si.simulatorapi = SIMULATORAPI_SCSTRUCKSIM2;
                int error = siminit(simdata, simmap, SIMULATORAPI_SCSTRUCKSIM2);
                simdatamap(simdata, simmap, NULL, SIMULATORAPI_SCSTRUCKSIM2, false, NULL);
                if (error == 0)
                {
                    simdata->simon = true;
                    simdata->simapi = SIMULATORAPI_SCSTRUCKSIM2;
                    simdata->simexe = simexe;

                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.mapapi = si.simulatorapi;
                    si.simulatorexe = simdata->simexe;
                    setSimInfo(&si);

                    return si;
                }
            }
            break;
        case SIMULATOREXE_DIRT_RALLY_2:
            simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Found running process for DiRT Rally 2.0");
            int dr2_error = (*setup_udp)(20777);
            if (dr2_error == 0)
            {
                dr2_error = siminitudp(simdata, simmap, SIMULATORAPI_DIRT_RALLY_2);
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
                setSimInfo(&si);

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
                wf2_error = siminitudp(simdata, simmap, SIMULATORAPI_WRECKFEST2);
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
                setSimInfo(&si);

                return si;
            }
            break;
        case SIMULATOREXE_RACE_ROOM:
        case SIMULATOREXE_FORZA_HORIZON_5:
            break;
    }
    return si;
}

int simdatamap(SimData* simdata, SimMap* simmap, SimMap* simmap2, SimulatorAPI simulatorapi, bool udp, char* base)
{
    char* a;
    char* b;
    char* c;
    char* d;

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

        case SIMULATORAPI_FORZA:
            break;
        case SIMULATORAPI_RACE_ROOM:
            break;
        case SIMULATORAPI_LMU:
            break;
    }

    if (simmap2 != NULL && simmap2->addr != NULL)
    {
        simdmap(simmap2, simdata);
    }

}

int simdmap(SimMap* simmap, SimData* simdata)
{
    memcpy(simmap->addr, simdata, sizeof(SimData));
}

int siminitudp(SimData* simdata, SimMap* simmap, SimulatorAPI simulator)
{
    int error = SIMAPI_ERROR_NONE;

    simdata->simapiversion = SIMAPI_VERSION;
    return error;
}


int siminit(SimData* simdata, SimMap* simmap, SimulatorAPI simulator)
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
            simmap->fd = shm_open(SIMAPI_MEM_FILE, O_RDONLY, S_IRUSR|S_IWUSR);
            if (simmap->fd == -1)
            {
                return 10;
            }

            simmap->addr = mmap(NULL, sizeof(SimData), PROT_READ, MAP_SHARED, simmap->fd, 0);
            if (simmap->addr == MAP_FAILED)
            {
                return 30;
            }
            simmap->hasSimApiDat = true;
            //slogi("found data for monocoque test...");
            break;

        case SIMULATORAPI_ASSETTO_CORSA :

            if(simmap->ac.has_physics == true)
            {
                return 0;
            }
            simmap->ac.has_physics=false;
            simmap->ac.has_static=false;
            simmap->ac.fd_physics = shm_open(AC_PHYSICS_FILE, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
            if (simmap->ac.fd_physics == -1)
            {
                //slogd("could not open Assetto Corsa physics engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->ac.physics_map_addr = mmap(NULL, sizeof(simmap->ac.ac_physics), PROT_READ, MAP_SHARED, simmap->ac.fd_physics, 0);
            if (simmap->ac.physics_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa physics data");
                return 30;
            }
            simmap->ac.has_physics=true;

            simmap->ac.fd_static = shm_open(AC_STATIC_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->ac.fd_static == -1)
            {
                //slogd("could not open Assetto Corsa static data");
                return 10;
            }
            simmap->ac.static_map_addr = mmap(NULL, sizeof(simmap->ac.ac_static), PROT_READ, MAP_SHARED, simmap->ac.fd_static, 0);
            if (simmap->ac.static_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->ac.has_static=true;

            simmap->ac.fd_graphic = shm_open(AC_GRAPHIC_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->ac.fd_graphic == -1)
            {
                //slogd("could not open Assetto Corsa graphic data");
                return 10;
            }
            simmap->ac.graphic_map_addr = mmap(NULL, sizeof(simmap->ac.ac_graphic), PROT_READ, MAP_SHARED, simmap->ac.fd_graphic, 0);
            if (simmap->ac.graphic_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->ac.has_graphic=true;
            //slogi("found data for Assetto Corsa...");
            simmap->ac.fd_crewchief = shm_open(AC_CREWCHIEF_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->ac.fd_crewchief == -1)
            {
                //slogd("could not open Assetto Corsa graphic data");
                return 10;
            }
            simmap->ac.crewchief_map_addr = mmap(NULL, sizeof(simmap->ac.ac_crewchief), PROT_READ, MAP_SHARED, simmap->ac.fd_crewchief, 0);
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
            simmap->pcars2.fd_telemetry = shm_open(PCARS2_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->pcars2.fd_telemetry == -1)
            {
                //slogd("could not open Assetto Corsa physics engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->pcars2.telemetry_map_addr = mmap(NULL, sizeof(simmap->pcars2.pcars2_telemetry), PROT_READ, MAP_SHARED, simmap->pcars2.fd_telemetry, 0);
            if (simmap->pcars2.telemetry_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa physics data");
                return 30;
            }
            simmap->pcars2.has_telemetry=true;

            break;

        case SIMULATORAPI_RFACTOR2 :

            if(simmap->rf2.has_telemetry == true)
            {
                return 0;
            }
            simmap->rf2.has_telemetry=false;
            simmap->rf2.has_scoring=false;
            simmap->rf2.fd_telemetry = shm_open(RF2_TELEMETRY_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->rf2.fd_telemetry == -1)
            {
                //slogd("could not open RFactor2 Telemetry engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->rf2.telemetry_map_addr = mmap(NULL, sizeof(simmap->rf2.rf2_telemetry), PROT_READ, MAP_SHARED, simmap->rf2.fd_telemetry, 0);
            if (simmap->rf2.telemetry_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve RFactor2 telemetry data");
                return 30;
            }
            simmap->rf2.has_telemetry=true;
            simmap->rf2.fd_scoring = shm_open(RF2_SCORING_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->rf2.fd_scoring == -1)
            {
                //slogd("could not open RFactor2 Telemetry engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->rf2.scoring_map_addr = mmap(NULL, sizeof(simmap->rf2.rf2_scoring), PROT_READ, MAP_SHARED, simmap->rf2.fd_scoring, 0);
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
            simmap->scs2.fd_telemetry = open("/dev/shm/SCS/SCSTelemetry", O_CREAT|O_RDWR, 0777);
            //simmap->scs2.fd_telemetry = shm_open(SCS2_TELEMETRY_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->scs2.fd_telemetry == -1)
            {
                return SIMAPI_ERROR_NODATA;
            }
            simmap->scs2.telemetry_map_addr = mmap(NULL, sizeof(simmap->scs2.scs2_telemetry), PROT_READ, MAP_SHARED, simmap->scs2.fd_telemetry, 0);
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


int simfree(SimData* simdata, SimMap* simmap, SimulatorAPI simulator)
{
    int error = SIMAPI_ERROR_NONE;

    if(simmap->ac.has_physics==true)
    {
        if (munmap(simmap->ac.physics_map_addr, sizeof(simmap->ac.ac_physics)) == -1)
        {
            return 0101100;
        }

        if (close(simmap->ac.fd_physics) == -1)
        {
            return 0101200;
        }

        simmap->ac.has_physics = false;
    }
    if(simmap->ac.has_static==true)
    {
        if (munmap(simmap->ac.static_map_addr, sizeof(simmap->ac.ac_static)) == -1)
        {
            return 0102100;
        }

        if (close(simmap->ac.fd_static) == -1)
        {
            return 0102200;
        }

        simmap->ac.has_static = false;
    }

    if(simmap->ac.has_graphic==true)
    {
        if (munmap(simmap->ac.graphic_map_addr, sizeof(simmap->ac.ac_graphic)) == -1)
        {
            return 0103100;
        }

        if (close(simmap->ac.fd_graphic) == -1)
        {
            return 0103200;
        }

        simmap->ac.has_graphic = false;
    }

    if(simmap->ac.has_crewchief==true)
    {
        if (munmap(simmap->ac.crewchief_map_addr, sizeof(simmap->ac.ac_crewchief)) == -1)
        {
            return 0104100;
        }

        if (close(simmap->ac.fd_crewchief) == -1)
        {
            return 0104200;
        }

        simmap->ac.has_crewchief = false;
    }

    if(simmap->pcars2.has_telemetry==true)
    {
        if (munmap(simmap->pcars2.telemetry_map_addr, sizeof(simmap->pcars2.pcars2_telemetry)) == -1)
        {
            return 0201100;
        }

        if (close(simmap->pcars2.fd_telemetry) == -1)
        {
            return 0201200;
        }

        simmap->pcars2.has_telemetry = false;
    }

    if(simmap->rf2.has_telemetry==true)
    {
        if (munmap(simmap->rf2.telemetry_map_addr, sizeof(simmap->rf2.rf2_telemetry)) == -1)
        {
            return 100;
        }

        if (close(simmap->rf2.fd_telemetry) == -1)
        {
            return 200;
        }

        simmap->rf2.has_telemetry = false;
    }
    if(simmap->rf2.has_scoring==true)
    {
        if (munmap(simmap->rf2.scoring_map_addr, sizeof(simmap->rf2.rf2_scoring)) == -1)
        {
            return 100;
        }

        if (close(simmap->rf2.fd_scoring) == -1)
        {
            return 200;
        }

        simmap->rf2.has_scoring = false;
    }

    if(simmap->scs2.has_telemetry==true)
    {
        if (munmap(simmap->scs2.telemetry_map_addr, sizeof(simmap->scs2.scs2_telemetry)) == -1)
        {
            return 100;
        }

        if (close(simmap->scs2.fd_telemetry) == -1)
        {
            return 200;
        }

        simmap->scs2.has_telemetry = false;
    }

    if (simmap->dirt2.has_telemetry == true)
    {
        simmap->dirt2.has_telemetry = false;
    }

    bzero(simdata, sizeof(SimData));
    simdata->simapiversion = SIMAPI_VERSION;
    return error;
}

int freesimmap(SimMap* simmap, bool issimd)
{
    simapi_log(SIMAPI_LOGLEVEL_INFO, "Freeing universal shared memory");

    if(simmap->fd == -1)
    {
        free(simmap);
        return 0;
    }

    if (munmap(simmap->addr, sizeof(SimData)) == -1)
    {
        return 100;
    }
    if(issimd == true)
    {
        shm_unlink(SIMAPI_MEM_FILE);
    }

    if (close(simmap->fd) == -1)
    {
        return 200;
    }
    simmap->hasSimApiDat = false;
    free(simmap);
    return 0;
}

int opensimmap(SimMap* simmap)
{
    simmap->fd = shm_open(SIMAPI_MEM_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    simapi_log(SIMAPI_LOGLEVEL_INFO, "Opening universal shared memory");
    if (simmap->fd == -1)
    {
        printf("open");
        return 10;
    }
    int res = ftruncate(simmap->fd, sizeof(SimData));
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    void* addr = mmap(NULL, sizeof(SimData), PROT_WRITE, MAP_SHARED, simmap->fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    simmap->addr = addr;
    return 0;
}


int opensimcompatmap(SimCompatMap* compatmap)
{
    compatmap->pcars2_fd = shm_open(PCARS2_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->pcars2_fd == -1)
    {
        printf("open");
        return 10;
    }
    int res = ftruncate(compatmap->pcars2_fd, PCARS2_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    void* addr = mmap(NULL, PCARS2_SIZE, PROT_WRITE, MAP_SHARED, compatmap->pcars2_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->pcars2_addr = addr;

    compatmap->acphysics_fd = shm_open(AC_PHYSICS_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acphysics_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = ftruncate(compatmap->acphysics_fd, AC_PHYSICS_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = mmap(NULL, AC_PHYSICS_SIZE, PROT_WRITE, MAP_SHARED, compatmap->acphysics_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->acphysics_addr = addr;

    compatmap->acgraphics_fd = shm_open(AC_GRAPHIC_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acgraphics_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = ftruncate(compatmap->acgraphics_fd, AC_GRAPHIC_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = mmap(NULL, AC_GRAPHIC_SIZE, PROT_WRITE, MAP_SHARED, compatmap->acgraphics_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->acgraphics_addr = addr;

    compatmap->acstatic_fd = shm_open(AC_STATIC_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acstatic_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = ftruncate(compatmap->acstatic_fd, AC_STATIC_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = mmap(NULL, AC_STATIC_SIZE, PROT_WRITE, MAP_SHARED, compatmap->acstatic_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->acstatic_addr = addr;

    compatmap->accrew_fd = shm_open(AC_CREWCHIEF_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->accrew_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = ftruncate(compatmap->accrew_fd, AC_CREWCHIEF_SIZE);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = mmap(NULL, AC_CREWCHIEF_SIZE, PROT_WRITE, MAP_SHARED, compatmap->accrew_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->accrew_addr = addr;
    return 0;
}

int simcompatmapclear(SimCompatMap* compatmap)
{
    memset(compatmap->acphysics_addr, 0, AC_PHYSICS_SIZE);
    memset(compatmap->acphysics_addr, 0, AC_STATIC_SIZE);
    memset(compatmap->acphysics_addr, 0, AC_GRAPHIC_SIZE);
    memset(compatmap->acphysics_addr, 0, AC_CREWCHIEF_SIZE);
    memset(compatmap->pcars2_addr, 0, PCARS2_SIZE);

    return 0;
}


int freesimcompatmap(SimCompatMap* compatmap)
{
    if (munmap(compatmap->acphysics_addr, AC_PHYSICS_SIZE) == -1)
    {
        return 100;
    }
    shm_unlink(AC_PHYSICS_FILE);

    if (close(compatmap->acphysics_fd) == -1)
    {
        return 200;
    }

    if (munmap(compatmap->acstatic_addr, AC_STATIC_SIZE) == -1)
    {
        return 100;
    }
    shm_unlink(AC_STATIC_FILE);

    if (close(compatmap->acstatic_fd) == -1)
    {
        return 200;
    }

    if (munmap(compatmap->acgraphics_addr, AC_GRAPHIC_SIZE) == -1)
    {
        return 100;
    }
    shm_unlink(AC_GRAPHIC_FILE);

    if (close(compatmap->acgraphics_fd) == -1)
    {
        return 200;
    }

    if (munmap(compatmap->accrew_addr, AC_CREWCHIEF_SIZE) == -1)
    {
        return 100;
    }
    shm_unlink(AC_CREWCHIEF_FILE);

    if (close(compatmap->accrew_fd) == -1)
    {
        return 200;
    }

    if (munmap(compatmap->pcars2_addr, PCARS2_SIZE) == -1)
    {
        return 100;
    }
    shm_unlink(PCARS2_FILE);

    if (close(compatmap->pcars2_fd) == -1)
    {
        return 200;
    }

    return 0;
}
