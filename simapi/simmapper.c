#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "simdata.h"
#include "simapi.h"
#include "simmapper.h"
#include "test.h"
#include "ac.h"
#include "rf2.h"
#include "pcars2.h"
#include "scs2.h"

#include <sys/stat.h>
#include <sys/types.h>
#if defined(OS_WIN)
    #include <windows.h>
#else
    #include <dirent.h> // for *Nix directory access
    #include <unistd.h>
#endif

#include "getpid.h"
#include "../include/acdata.h"
#include "../include/rf2data.h"
#include "../include/pcars2data.h"
#include "../include/scs2data.h"

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

SimMap* createSimMap() {
    SimMap* ptr = malloc(sizeof(SimMap));
    ptr->fd = -1;
    ptr->addr = 0;
    return ptr;
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

void set_simapi_log_info(func_ptr_t func) {
    loginfo = func;
}
void set_simapi_log_debug(func_ptr_t func) {
    logdebug = func;
}
void set_simapi_log_trace(func_ptr_t func) {
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

// probably going to move functions like this to ac.h
LapTime ac_convert_to_simdata_laptime(int ac_laptime)
{
    LapTime l;
    l.minutes = ac_laptime/60000;
    l.seconds = ac_laptime/1000-(l.minutes*60);
    l.fraction = ac_laptime-(l.minutes*60000)-(l.seconds*1000);
    return l;
}

bool does_sim_file_exist(const char* file)
{
    if (file == NULL) { return false; }
    #if defined(OS_WIN)
        #if defined(WIN_API)
            // if you want the WinAPI, versus CRT
            if (strnlen(file, MAX_PATH+1) > MAX_PATH) {
                // ... throw error here or ...
                return false;
            }
            DWORD res = GetFileAttributesA(file);
            return (res != INVALID_FILE_ATTRIBUTES &&
                !(res & FILE_ATTRIBUTE_DIRECTORY));
        #else
            // Use Win CRT
            struct stat fi;
            if (_stat(file, &fi) == 0) {
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
        if (stat(file, &fi) == 0) {
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

        case SIMULATOR_ASSETTO_CORSA :
            si->SimUsesUDP = false;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = true;
            si->SimSupportsAdvancedUI = true;
            break;
        case SIMULATOR_RFACTOR2 :
            si->SimUsesUDP = false;
            si->SimSupportsBasicTelemetry = true;
        case SIMULATOR_PROJECTCARS2 :
            si->SimSupportsBasicTelemetry = true;
        case SIMULATOR_SCSTRUCKSIM2 :
            si->SimUsesUDP = false;
            si->SimSupportsBasicTelemetry = true;
        default:
            si->SimSupportsBasicTelemetry = true;
    }

    return 0;
}

SimInfo getSim(SimData* simdata, SimMap* simmap, bool force_udp, int (*setup_udp)(int), bool simd)
{
    SimInfo si;
    si.simulatorapi = -1;
    si.mapapi = -1;
    si.isSimOn = false;
    si.SimUsesUDP = false;
    si.SimSupportsBasicTelemetry = false;
    si.SimSupportsTyreEffects = false;
    si.SimSupportsRealtimeTelemetry = false;
    si.SimSupportsAdvancedUI = false;

    simapi_log(SIMAPI_LOGLEVEL_TRACE, "looking for running simulators");

    if(simd == false)
    {
        if (does_sim_file_exist("/dev/shm/SIMAPI.DAT"))
        {

            siminit(simdata, simmap, SIMULATOR_SIMAPI_TEST);
            simapi_log(SIMAPI_LOGLEVEL_INFO, "found running simapi daemon");
            simdatamap(simdata, simmap, NULL, SIMULATOR_SIMAPI_TEST, false, NULL);
            if(simdata->simversion == SIMAPI_VERSION)
            {
                if (simdata->simon == true)
                {
                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.mapapi = SIMULATOR_SIMAPI_TEST;
                    setSimInfo(&si);
                }
                return si;
            }
            else
            {
                simapi_log(SIMAPI_LOGLEVEL_INFO, "skipping sim api daemon due to version mismatch");
            }
        }
    }

    if (IsProcessRunning(AC_EXE)==true || IsProcessRunning(ACC_EXE)==true)
    {
        simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Found running process for Assetto Corsa");
        if (does_sim_file_exist("/dev/shm/acpmf_physics"))
        {
            if (does_sim_file_exist("/dev/shm/acpmf_static"))
            {
                simapi_log(SIMAPI_LOGLEVEL_DEBUG, "static and physics files found");
                si.simulatorapi = SIMULATOR_ASSETTO_CORSA;
                int error = siminit(simdata, simmap, SIMULATOR_ASSETTO_CORSA);
                simdatamap(simdata, simmap, NULL, SIMULATOR_ASSETTO_CORSA, false, NULL);
                if (error == 0 && simdata->simstatus > 1)
                {
                    simapi_log(SIMAPI_LOGLEVEL_DEBUG, "AC Shared memory looks good");
                    simdata->simon = true;
                    simdata->sim = SIMULATOR_ASSETTO_CORSA;
                    simdata->simapi = SIMULATOR_ASSETTO_CORSA;

                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.mapapi = si.simulatorapi;
                    setSimInfo(&si);

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
    }


    if (IsProcessRunning(RFACTOR2_EXE)==true)
    {

        if (does_sim_file_exist("/dev/shm/$rFactor2SMMP_Telemetry$"))
        {
            si.simulatorapi = SIMULATOR_RFACTOR2;
            int error = siminit(simdata, simmap, SIMULATOR_RFACTOR2);
            simdatamap(simdata, simmap, NULL, SIMULATOR_RFACTOR2, false, NULL);
            if (error == 0)
            {
                //slogi("found Assetto Corsa, starting application...");

                simdata->simon = true;
                simdata->sim = SIMULATOR_RFACTOR2;
                simdata->simapi = SIMULATOR_RFACTOR2;

                si.isSimOn = true;
                si.simulatorapi = simdata->simapi;
                si.mapapi = si.simulatorapi;
                setSimInfo(&si);

                return si;
            }
        }
    }

    if (IsProcessRunning(AMS2_EXE)==true)
    {
        if (force_udp == false)
        {
            if (does_sim_file_exist("/dev/shm/$pcars2"))
            {
                si.simulatorapi = SIMULATOR_PROJECTCARS2;
                int error = siminit(simdata, simmap, SIMULATOR_PROJECTCARS2);
                simdatamap(simdata, simmap, NULL, SIMULATOR_PROJECTCARS2, false, NULL);
                if (error == 0 && simdata->simstatus > 1)
                {
                    simdata->simon = true;
                    simdata->sim = SIMULATOR_PROJECTCARS2;
                    simdata->simapi = SIMULATOR_PROJECTCARS2;

                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.mapapi = si.simulatorapi;
                    setSimInfo(&si);

                    return si;
                }
            }
        }
        else
        {
            int error = (*setup_udp)(5606);
            error = siminitudp(simdata, simmap, SIMULATOR_PROJECTCARS2);
            if (error == 0)
            {
                si.simulatorapi = SIMULATOR_PROJECTCARS2;
                si.SimUsesUDP = true;
                simdatamap(simdata, simmap, NULL, SIMULATOR_PROJECTCARS2, true, NULL);
            }
            if (error == 0 && simdata->simstatus > 1)
            {
                simdata->simon = true;
                simdata->sim = SIMULATOR_PROJECTCARS2;
                simdata->simapi = SIMULATOR_PROJECTCARS2;

                si.isSimOn = true;
                si.simulatorapi = simdata->simapi;
                si.mapapi = si.simulatorapi;
                setSimInfo(&si);

                return si;
            }
        }
    }
    if (IsProcessRunning(EUROTRUCKS2_EXE)==true || IsProcessRunning(AMERICANTRUCKS_EXE))
    {
        if (does_sim_file_exist("/dev/shm/SCS/SCSTelemetry"))
        {
            si.simulatorapi = SIMULATOR_SCSTRUCKSIM2;
            int error = siminit(simdata, simmap, SIMULATOR_SCSTRUCKSIM2);
            simdatamap(simdata, simmap, NULL, SIMULATOR_SCSTRUCKSIM2, false, NULL);
            if (error == 0)
            {
                simdata->simon = true;
                simdata->sim = SIMULATOR_SCSTRUCKSIM2;
                simdata->simapi = SIMULATOR_SCSTRUCKSIM2;

                si.isSimOn = true;
                si.simulatorapi = simdata->simapi;
                si.mapapi = si.simulatorapi;
                setSimInfo(&si);

                return si;
            }
        }
    }
    return si;
}

int simdatamap(SimData* simdata, SimMap* simmap, SimMap* simmap2, SimulatorAPI simulator, bool udp, char* base)
{
    char* a;
    char* b;
    char* c;
    char* d;


    switch ( simulator )
    {
        case SIMULATOR_SIMAPI_TEST :
            memcpy(simdata, simmap->addr, sizeof(SimData));
            break;

        case SIMULATOR_ASSETTO_CORSA :

            a = simmap->d.ac.physics_map_addr;
            if (simmap->d.ac.has_static == true )
            {
                b = simmap->d.ac.static_map_addr;
                simdata->maxrpm = *(uint32_t*) (char*) (b + offsetof(struct SPageFileStatic, maxRpm));

                int strsize = 32;
                for(int i=0; i<strsize; i++)
                {
                    simmap->d.ac.car[i] = *(char*) (char*) ((b + offsetof(struct SPageFileStatic, carModel)) + (sizeof(char16_t) * i));
                    simmap->d.ac.track[i] = *(char*) (char*) ((b + offsetof(struct SPageFileStatic, track)) + (sizeof(char16_t) * i));
                    simmap->d.ac.driver[i] = *(char*) (char*) ((b + offsetof(struct SPageFileStatic, playerName)) + (sizeof(char16_t) * i));
                }

                simdata->car = simmap->d.ac.car;
                simdata->track = simmap->d.ac.track;
                simdata->driver = simmap->d.ac.driver;
            }

            if ( simmap->d.ac.has_graphic == true )
            {
                c = simmap->d.ac.graphic_map_addr;

                simdata->simstatus = *(int*) (char*) (c + offsetof(struct SPageFileGraphic, status));
                simdata->lap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, completedLaps));
                simdata->position = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, position));
                uint32_t lastlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iLastTime));
                simdata->lastlap = ac_convert_to_simdata_laptime(lastlap);
                uint32_t bestlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iBestTime));
                simdata->bestlap = ac_convert_to_simdata_laptime(bestlap);
                uint32_t currentlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iCurrentTime));
                simdata->currentlap = ac_convert_to_simdata_laptime(currentlap);
                simdata->time = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iCurrentTime));
                simdata->numlaps = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, numberOfLaps));
                simdata->session = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, session));
                simdata->inpit = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, isInPit));
                simdata->sectorindex = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, currentSectorIndex));
                simdata->lastsectorinms = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, lastSectorTime));
                simdata->flag = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, Flag));
                simdata->inpit = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, isInPit));

                int strsize = 32;
                for(int i=0; i<strsize; i++)
                {
                    simmap->d.ac.compound[i] = *(char*) (char*) ((c + offsetof(struct SPageFileGraphic, tyreCompound)) + (sizeof(char16_t) * i));
                }
                simdata->tyrecompound = simmap->d.ac.compound;

                float timeleft = *(float*) (char*) (c + offsetof(struct SPageFileGraphic, sessionTimeLeft));
                if (timeleft < 0)
                    simdata->timeleft = 0;
                else
                    simdata->timeleft = droundint(timeleft);
            }
            if ( simmap->d.ac.has_crewchief == true )
            {
                d = simmap->d.ac.crewchief_map_addr;
                int strsize = 32;

                simdata->numcars = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, numVehicles));
                int numcars = simdata->numcars;
                if (numcars > MAXCARS)
                {
                    numcars = MAXCARS;
                }
                for(int i=0; i<numcars; i++)
                {
                    simdata->cars[i].lap = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, lapCount)));
                    simdata->cars[i].pos = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, carLeaderboardPosition)));
                    simdata->cars[i].lastlap = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, lastLapTimeMS)));
                    simdata->cars[i].bestlap = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, bestLapMS)));
                    simdata->cars[i].inpitlane = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, isCarInPitline)));
                    simdata->cars[i].inpit = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, isCarInPit)));


                    simdata->cars[i].driver = (char*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, driverName)));
                    simdata->cars[i].car = (char*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, carModel)));
                }

                float player_spline = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, spLineLength)));
                float track_spline = *(float*) (char*) (b + offsetof(struct SPageFileStatic, TrackSPlineLength));
                simdata->playerspline = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, spLineLength)));
                simdata->trackspline = *(float*) (char*) (b + offsetof(struct SPageFileStatic, TrackSPlineLength));
                simdata->trackdistancearound = spLineLengthToDistanceRoundTrack(track_spline, player_spline);
                simdata->playerlaps = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, lapCount)));

                simdata->lapisvalid = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, currentLapInvalid)));
                simdata->lapisvalid = !simdata->lapisvalid;

                int currentlapinticks = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, currentLapTimeMS)));
                int lastlapinticks = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, lastLapTimeMS)));
                simdata->currentlapinseconds = currentlapinticks * 100000;
                simdata->lastlapinseconds = lastlapinticks * 100000;

                simdata->worldposx = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, x)));
                simdata->worldposy = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, y)));
                simdata->worldposx = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, z)));
            }

            simdata->rpms = *(uint32_t*) (char*) (a + offsetof(struct SPageFilePhysics, rpms));
            simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct SPageFilePhysics, gear));
            simdata->velocity = droundint( *(float*) (char*) (a + offsetof(struct SPageFilePhysics, speedKmh)));

            simdata->Xvelocity = droundint( *(float*) (char*) (a + offsetof(struct SPageFilePhysics, localVelocity) + (sizeof(float) * 0 )) );
            simdata->Zvelocity = droundint( *(float*) (char*) (a + offsetof(struct SPageFilePhysics, localVelocity) + (sizeof(float) * 1 )) );
            simdata->Yvelocity = droundint( *(float*) (char*) (a + offsetof(struct SPageFilePhysics, localVelocity) + (sizeof(float) * 2 )) );

            simdata->gas = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, gas));
            simdata->clutch = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, clutch));
            simdata->steer = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, steerAngle));
            simdata->brake = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brake));
            simdata->brakebias = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeBias));
            simdata->handbrake = 0;
            simdata->fuel = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, fuel));

            simdata->abs = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, abs));
            simdata->tyreRPS[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelAngularSpeed) + (sizeof(float) * 0));
            simdata->tyreRPS[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelAngularSpeed) + (sizeof(float) * 1));
            simdata->tyreRPS[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelAngularSpeed) + (sizeof(float) * 2));
            simdata->tyreRPS[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelAngularSpeed) + (sizeof(float) * 3));

            simdata->gearc[0] = simdata->gear + 47;
            if (simdata->gear == 0)
            {
                simdata->gearc[0] = 82;
            }
            if (simdata->gear == 1)
            {
                simdata->gearc[0] = 78;
            }
            simdata->gearc[1] = 0;

            simdata->tyrewear[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreWear) + (sizeof(float) * 0));
            simdata->tyrewear[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreWear) + (sizeof(float) * 1));
            simdata->tyrewear[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreWear) + (sizeof(float) * 2));
            simdata->tyrewear[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreWear) + (sizeof(float) * 3));

            simdata->tyretemp[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreCoreTemperature) + (sizeof(float) * 0));
            simdata->tyretemp[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreCoreTemperature) + (sizeof(float) * 1));
            simdata->tyretemp[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreCoreTemperature) + (sizeof(float) * 2));
            simdata->tyretemp[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreCoreTemperature) + (sizeof(float) * 3));

            simdata->braketemp[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeTemp) + (sizeof(float) * 0));
            simdata->braketemp[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeTemp) + (sizeof(float) * 1));
            simdata->braketemp[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeTemp) + (sizeof(float) * 2));
            simdata->braketemp[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeTemp) + (sizeof(float) * 3));

            simdata->tyrepressure[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelsPressure) + (sizeof(float) * 0));
            simdata->tyrepressure[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelsPressure) + (sizeof(float) * 1));
            simdata->tyrepressure[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelsPressure) + (sizeof(float) * 2));
            simdata->tyrepressure[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelsPressure) + (sizeof(float) * 3));

            simdata->airdensity = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, airDensity));
            simdata->airtemp = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, airTemp));
            simdata->tracktemp = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, roadTemp));

            simdata->altitude = 1;
            break;

        case SIMULATOR_RFACTOR2 :

            a = simmap->d.rf2.telemetry_map_addr;

            simdata->simstatus = 2;
            simdata->car = "default";
            simdata->velocity = abs(droundint(3.6 * (*(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + ((sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mLocalVel)) + (sizeof(double) * 2)))));
            simdata->rpms = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + ((sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mEngineRPM)));
            simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + ((sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mGear)));
            simdata->maxrpm = droundint( *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + ((sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mEngineMaxRPM))));
            simdata->altitude = 1;

            simdata->gearc[0] = simdata->gear + 48;
            if (simdata->gear < 0)
            {
                simdata->gearc[0] = 82;
            }
            if (simdata->gear == 0)
            {
                simdata->gearc[0] = 78;
            }
            simdata->gearc[1] = 0;

            break;

        case SIMULATOR_PROJECTCARS2 :



            if(udp == false)
            {

                a = simmap->d.pcars2.telemetry_map_addr;
                simdata->simstatus = 2;
                simdata->car = "default";
                simdata->velocity = droundint(3.6 * (*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mSpeed))));
                simdata->rpms = droundint(*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mRpm)));
                simdata->maxrpm = droundint(*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mMaxRPM)));
                simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mGear));
                simdata->altitude = 1;

                simdata->gearc[0] = simdata->gear + 48;
                if (simdata->gear < 0)
                {
                    simdata->gearc[0] = 82;
                }
                if (simdata->gear == 0)
                {
                    simdata->gearc[0] = 78;
                }
                simdata->gearc[1] = 0;
                break;
            }
            else
            {

                //eCarPhysics = 0,
                //eRaceDefinition = 1,
                //eParticipants = 2,
                //eTimings = 3,
                //eGameState = 4,
                //eWeatherState = 5, // not sent at the moment, information can be found in the game state packet
                //eVehicleNames = 6, //not sent at the moment
                //eTimeStats = 7,
                //eParticipantVehicleNames = 8
                a = base;
                simdata->simstatus = 2;
                if(base != NULL)
                {

                    int packet_type = *(uint8_t*) (char*) (a + offsetof(struct ams2UDPData, mPacketType));
                    char* msg;
                    asprintf(&msg, "project cars 2 packet type %i", packet_type);
                    simapi_log(SIMAPI_LOGLEVEL_TRACE, msg);
                    free(msg);
                    switch ( packet_type )
                    {
                        case 0:
                            simdata->car = "default";
                            simdata->fuel = *(float*) (char*) (a + offsetof(struct ams2UDPData, sFuelLevel));
                            simdata->velocity = droundint(3.6 * (*(float*) (char*) (a + offsetof(struct ams2UDPData, sSpeed))));
                            simdata->rpms = *(uint16_t*) (char*) (a + offsetof(struct ams2UDPData, sRpm));
                            simdata->maxrpm = *(uint16_t*) (char*) (a + offsetof(struct ams2UDPData, sMaxRpm));
                            simdata->gear = *(uint8_t*) (char*) (a + offsetof(struct ams2UDPData, sGearNumGears));
                            // couldn't find this documented anywhere outside of the crewchief source code
                            simdata->gear = simdata->gear & 15;
                            simdata->brake = *(uint8_t*) (char*) (a + offsetof(struct ams2UDPData, sBrake));
                            simdata->gas = *(uint8_t*) (char*) (a + offsetof(struct ams2UDPData, sThrottle));
                            simdata->altitude = 1;

                            simdata->gearc[0] = simdata->gear + 48;
                            if (simdata->gear < 0)
                            {
                                simdata->gearc[0] = 82;
                            }
                            if (simdata->gear == 0)
                            {
                                simdata->gearc[0] = 78;
                            }
                            simdata->gearc[1] = 0;
                    }

                }
                break;
            }

        case SIMULATOR_SCSTRUCKSIM2 :

            a = simmap->d.scs2.telemetry_map_addr;

            simdata->simstatus = 2;
            simdata->car = "default";
            simdata->velocity = droundint(3.6 * (*(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.speed))));
            simdata->rpms = droundint(*(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.engineRpm)));
            simdata->brake = droundint(*(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.userBrake)));
            simdata->gas = droundint(*(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.userThrottle)));
            simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_i.gear));
            simdata->tyreRPS[0] = *(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.truck_wheelVelocity));
            simdata->tyreRPS[1] = *(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.truck_wheelVelocity) + (sizeof(float) * 1));
            simdata->tyreRPS[2] = *(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.truck_wheelVelocity) + (sizeof(float) * 2));
            simdata->tyreRPS[3] = *(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.truck_wheelVelocity) + (sizeof(float) * 3));
            simdata->maxrpm = droundint( *(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, config_f.engineRpmMax)));

            if(simdata->gear>0)
            {
                simdata->gearc[0] = (simdata->gear/2) + 48;
                if(simdata->gear % 2 == 0)
                {
                    simdata->gearc[1] = 72;
                }
                else
                {
                    simdata->gearc[1] = 76;
                    simdata->gearc[0]++;
                }
                simdata->gearc[2] = 0;
            }
            else
            {
                if (simdata->gear < 0)
                {
                    simdata->gearc[0] = 82;
                }
                if (simdata->gear == 0)
                {
                    simdata->gearc[0] = 78;
                }
                simdata->gearc[1] = 0;
            }

            simdata->altitude = 1;
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

    void* a;
    switch ( simulator )
    {
        case SIMULATOR_PROJECTCARS2 :

            simmap->d.pcars2.has_telemetry=false;

            simmap->d.pcars2.telemetry_map_addr = malloc( AMS2_MAX_UDP_PACKET_SIZE );
            simmap->d.pcars2.has_telemetry=true;
            break;
    }

    return error;
}


int siminit(SimData* simdata, SimMap* simmap, SimulatorAPI simulator)
{
    //slogi("searching for simulator data...");
    int error = SIMAPI_ERROR_NONE;

    void* a;
    switch ( simulator )
    {
        case SIMULATOR_SIMAPI_TEST :
            simmap->fd = shm_open(SIMAPI_MEM_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->fd == -1)
            {
                return 10;
            }

            simmap->addr = mmap(NULL, sizeof(SimData), PROT_READ, MAP_SHARED, simmap->fd, 0);
            if (simmap->addr == MAP_FAILED)
            {
                return 30;
            }
            //slogi("found data for monocoque test...");
            break;

        case SIMULATOR_ASSETTO_CORSA :

            simmap->d.ac.has_physics=false;
            simmap->d.ac.has_static=false;
            simmap->d.ac.fd_physics = shm_open(AC_PHYSICS_FILE, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
            if (simmap->d.ac.fd_physics == -1)
            {
                //slogd("could not open Assetto Corsa physics engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->d.ac.physics_map_addr = mmap(NULL, sizeof(simmap->d.ac.ac_physics), PROT_READ, MAP_SHARED, simmap->d.ac.fd_physics, 0);
            if (simmap->d.ac.physics_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa physics data");
                return 30;
            }
            simmap->d.ac.has_physics=true;

            simmap->d.ac.fd_static = shm_open(AC_STATIC_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->d.ac.fd_static == -1)
            {
                //slogd("could not open Assetto Corsa static data");
                return 10;
            }
            simmap->d.ac.static_map_addr = mmap(NULL, sizeof(simmap->d.ac.ac_static), PROT_READ, MAP_SHARED, simmap->d.ac.fd_static, 0);
            if (simmap->d.ac.static_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->d.ac.has_static=true;

            simmap->d.ac.fd_graphic = shm_open(AC_GRAPHIC_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->d.ac.fd_graphic == -1)
            {
                //slogd("could not open Assetto Corsa graphic data");
                return 10;
            }
            simmap->d.ac.graphic_map_addr = mmap(NULL, sizeof(simmap->d.ac.ac_graphic), PROT_READ, MAP_SHARED, simmap->d.ac.fd_graphic, 0);
            if (simmap->d.ac.graphic_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->d.ac.has_graphic=true;
            //slogi("found data for Assetto Corsa...");
            simmap->d.ac.fd_crewchief = shm_open(AC_CREWCHIEF_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->d.ac.fd_crewchief == -1)
            {
                //slogd("could not open Assetto Corsa graphic data");
                return 10;
            }
            simmap->d.ac.crewchief_map_addr = mmap(NULL, sizeof(simmap->d.ac.ac_crewchief), PROT_READ, MAP_SHARED, simmap->d.ac.fd_crewchief, 0);
            if (simmap->d.ac.crewchief_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->d.ac.has_crewchief=true;

            break;

        case SIMULATOR_PROJECTCARS2 :

            simmap->d.pcars2.has_telemetry=false;
            simmap->d.pcars2.fd_telemetry = shm_open(PCARS2_FILE_LINUX, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->d.pcars2.fd_telemetry == -1)
            {
                //slogd("could not open Assetto Corsa physics engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->d.pcars2.telemetry_map_addr = mmap(NULL, sizeof(simmap->d.pcars2.pcars2_telemetry), PROT_READ, MAP_SHARED, simmap->d.pcars2.fd_telemetry, 0);
            if (simmap->d.pcars2.telemetry_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa physics data");
                return 30;
            }
            simmap->d.pcars2.has_telemetry=true;

            break;

        case SIMULATOR_RFACTOR2 :

            simmap->d.rf2.has_telemetry=false;
            simmap->d.rf2.has_scoring=false;
            simmap->d.rf2.fd_telemetry = shm_open(RF2_TELEMETRY_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->d.rf2.fd_telemetry == -1)
            {
                //slogd("could not open RFactor2 Telemetry engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->d.rf2.telemetry_map_addr = mmap(NULL, sizeof(simmap->d.rf2.rf2_telemetry), PROT_READ, MAP_SHARED, simmap->d.rf2.fd_telemetry, 0);
            if (simmap->d.rf2.telemetry_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve RFactor2 telemetry data");
                return 30;
            }
            simmap->d.rf2.has_telemetry=true;


            //slogi("found data for RFactor2...");
            break;

        case SIMULATOR_SCSTRUCKSIM2 :

            simmap->d.scs2.has_telemetry=false;
            simmap->d.scs2.fd_telemetry = open("/dev/shm/SCS/SCSTelemetry", O_CREAT|O_RDWR, 0777);
            //simmap->d.scs2.fd_telemetry = shm_open(SCS2_TELEMETRY_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->d.scs2.fd_telemetry == -1)
            {
                return SIMAPI_ERROR_NODATA;
            }
            simmap->d.scs2.telemetry_map_addr = mmap(NULL, sizeof(simmap->d.scs2.scs2_telemetry), PROT_READ, MAP_SHARED, simmap->d.scs2.fd_telemetry, 0);
            if (simmap->d.scs2.telemetry_map_addr == MAP_FAILED)
            {
                return 30;
            }
            simmap->d.scs2.has_telemetry=true;

            break;
    }

    return error;
}


int simfree(SimData* simdata, SimMap* simmap, SimulatorAPI simulator)
{
    int error = SIMAPI_ERROR_NONE;
    if(simulator == -1)
    {
        return error;
    }

    switch ( simulator )
    {
        case SIMULATOR_SIMAPI_TEST :
            if (munmap(simmap->addr, sizeof(SimData)) == -1)
            {
                return 100;
            }

            if (close(simmap->fd) == -1)
            {
                return 200;
            }
            break;

        case SIMULATOR_ASSETTO_CORSA :
            if(simmap->d.ac.has_physics==true)
            {
                if (munmap(simmap->d.ac.physics_map_addr, sizeof(simmap->d.ac.ac_physics)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.ac.fd_physics) == -1)
                {
                    return 200;
                }

                simmap->d.ac.has_physics = false;
            }
            if(simmap->d.ac.has_static==true)
            {
                if (munmap(simmap->d.ac.static_map_addr, sizeof(simmap->d.ac.ac_static)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.ac.fd_static) == -1)
                {
                    return 200;
                }

                simmap->d.ac.has_static = false;
            }

            if(simmap->d.ac.has_graphic==true)
            {
                if (munmap(simmap->d.ac.graphic_map_addr, sizeof(simmap->d.ac.ac_graphic)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.ac.fd_graphic) == -1)
                {
                    return 200;
                }

                simmap->d.ac.has_graphic = false;
            }

            if(simmap->d.ac.has_crewchief==true)
            {
                if (munmap(simmap->d.ac.crewchief_map_addr, sizeof(simmap->d.ac.ac_crewchief)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.ac.fd_crewchief) == -1)
                {
                    return 200;
                }

                simmap->d.ac.has_crewchief = false;
            }
            break;

        case SIMULATOR_PROJECTCARS2 :
            if(simmap->d.pcars2.has_telemetry==true)
            {
                if (munmap(simmap->d.pcars2.telemetry_map_addr, sizeof(simmap->d.pcars2.pcars2_telemetry)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.pcars2.fd_telemetry) == -1)
                {
                    return 200;
                }

                simmap->d.pcars2.has_telemetry = false;
            }
            break;

        case SIMULATOR_RFACTOR2 :
            if(simmap->d.rf2.has_telemetry==true)
            {
                if (munmap(simmap->d.rf2.telemetry_map_addr, sizeof(simmap->d.rf2.rf2_telemetry)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.rf2.fd_telemetry) == -1)
                {
                    return 200;
                }

                simmap->d.rf2.has_telemetry = false;
            }
            break;
        case SIMULATOR_SCSTRUCKSIM2 :
            if(simmap->d.scs2.has_telemetry==true)
            {
                if (munmap(simmap->d.scs2.telemetry_map_addr, sizeof(simmap->d.scs2.scs2_telemetry)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.scs2.fd_telemetry) == -1)
                {
                    return 200;
                }

                simmap->d.scs2.has_telemetry = false;
            }
            break;
    }

    bzero(simdata, sizeof(SimData));
    return error;
}

int freesimmap(SimMap* simmap)
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
    shm_unlink(SIMAPI_MEM_FILE);
    
    if (close(simmap->fd) == -1)
    {
        return 200;
    }

    free(simmap);
    return 0;
}

int opensimmap(SimMap* simmap)
{
    simmap->fd = shm_open(SIMAPI_MEM_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
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

    compatmap->acphysics_fd = shm_open(AC_PHYSICS_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acphysics_fd == -1)
    {
        printf("open");
        return 10;
    }
    int res = ftruncate(compatmap->acphysics_fd, sizeof(struct SPageFilePhysics));
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    void* addr = mmap(NULL, sizeof(struct SPageFilePhysics), PROT_WRITE, MAP_SHARED, compatmap->acphysics_fd, 0);
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
    res = ftruncate(compatmap->acgraphics_fd, sizeof(struct SPageFileGraphic));
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = mmap(NULL, sizeof(struct SPageFileGraphic), PROT_WRITE, MAP_SHARED, compatmap->acgraphics_fd, 0);
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
    res = ftruncate(compatmap->acstatic_fd, sizeof(struct SPageFileStatic));
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = mmap(NULL, sizeof(struct SPageFileStatic), PROT_WRITE, MAP_SHARED, compatmap->acstatic_fd, 0);
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
    res = ftruncate(compatmap->accrew_fd, sizeof(struct SPageFileCrewChief));
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = mmap(NULL, sizeof(struct SPageFileCrewChief), PROT_WRITE, MAP_SHARED, compatmap->accrew_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->accrew_addr = addr;
    return 0;
}

int freesimcompatmap(SimCompatMap* compatmap)
{
    if (munmap(compatmap->acphysics_addr, sizeof(struct SPageFilePhysics)) == -1)
    {
        return 100;
    }
    shm_unlink(AC_PHYSICS_FILE);
    
    if (close(compatmap->acphysics_fd) == -1)
    {
        return 200;
    }
    
    if (munmap(compatmap->acstatic_addr, sizeof(struct SPageFileStatic)) == -1)
    {
        return 100;
    }
    shm_unlink(AC_STATIC_FILE);
    
    if (close(compatmap->acstatic_fd) == -1)
    {
        return 200;
    }
    
    if (munmap(compatmap->acgraphics_addr, sizeof(struct SPageFileGraphic)) == -1)
    {
        return 100;
    }
    shm_unlink(AC_GRAPHIC_FILE);
    
    if (close(compatmap->acgraphics_fd) == -1)
    {
        return 200;
    }
    
    if (munmap(compatmap->accrew_addr, sizeof(struct SPageFileCrewChief)) == -1)
    {
        return 100;
    }
    shm_unlink(AC_CREWCHIEF_FILE);
    
    if (close(compatmap->accrew_fd) == -1)
    {
        return 200;
    }
    return 0;
}
