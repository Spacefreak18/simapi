#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "simapi.h"
#include "simmapper.h"
#include "simdata.h"
#include "test.h"
#include "ac.h"
#include "rf2.h"

#include "../include/acdata.h"
#include "../include/rf2data.h"


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
                int i = 0;
                char* car = (char*) malloc(sizeof(char) * (strsize));
                char* track = (char*) malloc(sizeof(char) * (strsize));
                char* driver = (char*) malloc(sizeof(char) * (strsize));
                while (i < strsize)
                {
                    car[i] = *(char*) (char*) ((b + offsetof(struct SPageFileStatic, carModel)) + (sizeof(char16_t) * i));
                    track[i] = *(char*) (char*) ((b + offsetof(struct SPageFileStatic, track)) + (sizeof(char16_t) * i));
                    driver[i] = *(char*) (char*) ((b + offsetof(struct SPageFileStatic, playerName)) + (sizeof(char16_t) * i));
                    i++;
                }
                
                simdata->car = car;
                simdata->track = track;
                simdata->driver = driver;
            }
            
            if ( simmap->d.ac.has_graphic == true )
            {
                c = simmap->d.ac.graphic_map_addr;
                simdata->lap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, completedLaps));
                simdata->position = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, position));
                simdata->lastlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iLastTime));
                simdata->bestlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iBestTime));
                simdata->time = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iCurrentTime));
                simdata->numlaps = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, numberOfLaps));
                
                simdata->timeleft = *(float*) (char*) (c + offsetof(struct SPageFileGraphic, sessionTimeLeft));
            }
            
            simdata->rpms = *(uint32_t*) (char*) (a + offsetof(struct SPageFilePhysics, rpms));
            simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct SPageFilePhysics, gear));
            simdata->velocity = ceil( *(float*) (char*) (a + offsetof(struct SPageFilePhysics, speedKmh)));
            simdata->gas = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, gas));
            simdata->brake = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brake));
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
