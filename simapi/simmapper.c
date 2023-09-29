#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "simapi.h"
#include "simmapper.h"
#include "test.h"
#include "ac.h"
#include "rf2.h"

#include <sys/stat.h>
#include <sys/types.h>
#if defined(OS_WIN)
    #include <windows.h>
#else
    #include <dirent.h> // for *Nix directory access
    #include <unistd.h>
#endif


#include "../include/acdata.h"
#include "../include/rf2data.h"

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

void getSim(SimData* simdata, SimMap* simmap, bool* simstate, Simulator* sim)
{

    if (does_sim_file_exist("/dev/shm/acpmf_physics"))
    {
        if (does_sim_file_exist("/dev/shm/acpmf_static"))
        {
            *sim = SIMULATOR_ASSETTO_CORSA;
            int error = siminit(simdata, simmap, 1);
            simdatamap(simdata, simmap, 1);
            if (error == 0 && simdata->simstatus > 1)
            {
                //slogi("found Assetto Corsa, starting application...");
                *simstate = true;
            }
        }
    }
    if (does_sim_file_exist("/dev/shm/$rFactor2SMMP_Telemetry$"))
    {
        *sim = SIMULATOR_RFACTOR2;
        int error = siminit(simdata, simmap, 2);
        simdatamap(simdata, simmap, 2);
        if (error == 0)
        {
            //slogi("found Assetto Corsa, starting application...");
            *simstate = true;
        }
    }
}

#ifndef SIMMAP_ALL
int simdatamap(SimData* simdata, SimMap* simmap, Simulator simulator)
{
    char* a;
    char* b;
    char* c;
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
            }
            if (simmap->d.ac.has_graphic == true )
            {
                c = simmap->d.ac.graphic_map_addr;
                simdata->simstatus = *(int*) (char*) (c + offsetof(struct SPageFileGraphic, status));
                simdata->lap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, completedLaps));
            }
            simdata->rpms = *(uint32_t*) (char*) (a + offsetof(struct SPageFilePhysics, rpms));
            simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct SPageFilePhysics, gear));
            simdata->velocity = ceil( *(float*) (char*) (a + offsetof(struct SPageFilePhysics, speedKmh)));
            simdata->altitude = 1;
            break;
        
        case SIMULATOR_RFACTOR2 :

            a = simmap->d.rf2.telemetry_map_addr;

            simdata->velocity = ceil(3.6 * ( *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles[0].mLocalVel.z))));
            simdata->rpms = ceil( *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles[0].mEngineRPM)));
            simdata->maxrpm = ceil( *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles[0].mEngineMaxRPM)));
            simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles[0].mGear));
            break;
    
    }
}
#endif

#ifdef SIMMAP_ALL
int simdatamap(SimData* simdata, SimMap* simmap, Simulator simulator)
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
                simdata->lastlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iLastTime));
                simdata->bestlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iBestTime));
                simdata->time = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iCurrentTime));
                simdata->numlaps = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, numberOfLaps));
                simdata->session = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, session));
                simdata->inpit = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, isInPit));
                simdata->sectorindex = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, currentSectorIndex));
                simdata->lastsectorinms = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, lastSectorTime));

                float timeleft = *(float*) (char*) (c + offsetof(struct SPageFileGraphic, sessionTimeLeft));
                if (timeleft < 0)
                    simdata->timeleft = 0;
                else
                    simdata->timeleft = ceil(timeleft);
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
            simdata->velocity = ceil( *(float*) (char*) (a + offsetof(struct SPageFilePhysics, speedKmh)));
            simdata->gas = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, gas));
            simdata->clutch = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, clutch));
            simdata->steer = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, steerAngle));
            simdata->brake = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brake));
            simdata->handbrake = 0;
            simdata->fuel = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, fuel));

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

            simdata->velocity = ceil(3.6 * ( *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles[0].mLocalVel.z))));
            simdata->rpms = ceil( *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles[0].mEngineRPM)));
            simdata->maxrpm = ceil( *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles[0].mEngineMaxRPM)));
            simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles[0].mGear));
            break;
    
    }
}
#endif

int siminit(SimData* simdata, SimMap* simmap, Simulator simulator)
{
    //slogi("searching for simulator data...");
    int error = SIMAPI_ERROR_NONE;

    void* a;
    switch ( simulator )
    {
        case SIMULATOR_SIMAPI_TEST :
            simmap->fd = shm_open(TEST_MEM_FILE_LOCATION, O_RDONLY, S_IRUSR | S_IWUSR);
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
            simmap->fd = shm_open(AC_PHYSICS_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->fd == -1)
            {
                //slogd("could not open Assetto Corsa physics engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->d.ac.physics_map_addr = mmap(NULL, sizeof(simmap->d.ac.ac_physics), PROT_READ, MAP_SHARED, simmap->fd, 0);
            if (simmap->d.ac.physics_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa physics data");
                return 30;
            }
            simmap->d.ac.has_physics=true;

            simmap->fd = shm_open(AC_STATIC_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->fd == -1)
            {
                //slogd("could not open Assetto Corsa static data");
                return 10;
            }
            simmap->d.ac.static_map_addr = mmap(NULL, sizeof(simmap->d.ac.ac_static), PROT_READ, MAP_SHARED, simmap->fd, 0);
            if (simmap->d.ac.static_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->d.ac.has_static=true;

            simmap->fd = shm_open(AC_GRAPHIC_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->fd == -1)
            {
                //slogd("could not open Assetto Corsa graphic data");
                return 10;
            }
            simmap->d.ac.graphic_map_addr = mmap(NULL, sizeof(simmap->d.ac.ac_graphic), PROT_READ, MAP_SHARED, simmap->fd, 0);
            if (simmap->d.ac.graphic_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->d.ac.has_graphic=true;
            //slogi("found data for Assetto Corsa...");
            simmap->fd = shm_open(AC_CREWCHIEF_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->fd == -1)
            {
                //slogd("could not open Assetto Corsa graphic data");
                return 10;
            }
            simmap->d.ac.crewchief_map_addr = mmap(NULL, sizeof(simmap->d.ac.ac_crewchief), PROT_READ, MAP_SHARED, simmap->fd, 0);
            if (simmap->d.ac.crewchief_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->d.ac.has_crewchief=true;

            break;

        case SIMULATOR_RFACTOR2 :

            simmap->d.rf2.has_telemetry=false;
            simmap->d.rf2.has_scoring=false;
            simmap->fd = shm_open(RF2_TELEMETRY_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->fd == -1)
            {
                //slogd("could not open RFactor2 Telemetry engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->d.rf2.telemetry_map_addr = mmap(NULL, sizeof(simmap->d.rf2.rf2_telemetry), PROT_READ, MAP_SHARED, simmap->fd, 0);
            if (simmap->d.rf2.telemetry_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve RFactor2 telemetry data");
                return 30;
            }
            simmap->d.rf2.has_telemetry=true;


            //slogi("found data for RFactor2...");
            break;
    }

    return error;
}
