#include <stddef.h>
#include <math.h>

#include "../simmap.h"
#include "../simdata.h"
#include "../simapi.h"
#include "../simmapper.h"
#include "../ac.h"

#include "../../include/acdata.h"

static LapTime ac_convert_to_simdata_laptime(int ac_laptime)
{
    LapTime l;
    l.minutes = ac_laptime/60000;
    l.seconds = ac_laptime/1000-(l.minutes*60);
    l.fraction = ac_laptime-(l.minutes*60000)-(l.seconds*1000);
    return l;
}

static float spLineLengthToDistanceRoundTrack(float trackLength, float spLine)
{
    if (spLine < 0.0)
    {
        spLine -= 1;
    }
    return spLine * trackLength;
}

static int droundint(double d)
{
    return trunc(nearbyint(d));
}


int acc_flag_to_simdata_flag(int ac_flag)
{

    switch ( ac_flag )
    {
        case ACC_NO_FLAG:
            return SIMAPI_FLAG_GREEN;
        case ACC_GREEN_FLAG:
            return SIMAPI_FLAG_GREEN;
        case ACC_BLUE_FLAG:
            return SIMAPI_FLAG_BLUE;
        case ACC_WHITE_FLAG:
            return SIMAPI_FLAG_WHITE;
        case ACC_YELLOW_FLAG:
            return SIMAPI_FLAG_YELLOW;
        case ACC_PENALTY_FLAG:
            return SIMAPI_FLAG_BLACK_WHITE;
        case ACC_BLACK_FLAG:
            return SIMAPI_FLAG_BLACK;
        case ACC_CHECKERED_FLAG:
            return SIMAPI_FLAG_CHEQUERED;
        case ACC_ORANGE_FLAG:
            return SIMAPI_FLAG_ORANGE;
        default:
            return SIMAPI_FLAG_GREEN;
    }
}

int acc_get_global_flag(int yellow, int white, int chequered, int green, int red)
{
    if(chequered > 0)
    {
        return SIMAPI_FLAG_CHEQUERED;
    }
    if(red > 0)
    {
        return SIMAPI_FLAG_RED;
    }
    if(yellow > 0)
    {
        return SIMAPI_FLAG_YELLOW;
    }
    if(white > 0)
    {
        return SIMAPI_FLAG_WHITE;
    }

    return SIMAPI_FLAG_GREEN;
}


void map_assetto_corsa_data(SimData* simdata, SimMap* simmap, SimulatorEXE simexe)
{

    char* a;
    char* b;
    char* c;
    char* d;

    a = simmap->ac.physics_map_addr;

    // basic telemetry
    simdata->rpms = *(uint32_t*) (char*) (a + offsetof(struct SPageFilePhysics, rpms));
    simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct SPageFilePhysics, gear));
    simdata->velocity = droundint( *(float*) (char*) (a + offsetof(struct SPageFilePhysics, speedKmh)));

    simdata->gas = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, gas));
    simdata->clutch = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, clutch));
    simdata->steer = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, steerAngle));
    simdata->brake = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brake));
    simdata->heading = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, heading) + sizeof(float));
    simdata->brakebias = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeBias));
    simdata->handbrake = 0;
    simdata->fuel = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, fuel));
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


    // tyre effects
    simdata->abs = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, abs));
    simdata->tyreRPS[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelAngularSpeed) + (sizeof(float) * 0));
    simdata->tyreRPS[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelAngularSpeed) + (sizeof(float) * 1));
    simdata->tyreRPS[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelAngularSpeed) + (sizeof(float) * 2));
    simdata->tyreRPS[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelAngularSpeed) + (sizeof(float) * 3));

    simdata->Xvelocity = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, localVelocity) + (sizeof(float) * 0 ));
    simdata->Zvelocity = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, localVelocity) + (sizeof(float) * 1 ));
    simdata->Yvelocity = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, localVelocity) + (sizeof(float) * 2 ));

    simdata->worldXvelocity = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, velocity) + (sizeof(float) * 0 ));
    simdata->worldZvelocity = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, velocity) + (sizeof(float) * 1 ));
    simdata->worldYvelocity = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, velocity) + (sizeof(float) * 2 ));

    simdata->suspension[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, suspensionTravel) + (sizeof(float) * 0));
    simdata->suspension[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, suspensionTravel) + (sizeof(float) * 1));
    simdata->suspension[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, suspensionTravel) + (sizeof(float) * 2));
    simdata->suspension[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, suspensionTravel) + (sizeof(float) * 3));


    //advanced ui
    simdata->tyrewear[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreWear) + (sizeof(float) * 0));
    simdata->tyrewear[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreWear) + (sizeof(float) * 1));
    simdata->tyrewear[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreWear) + (sizeof(float) * 2));
    simdata->tyrewear[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreWear) + (sizeof(float) * 3));

    simdata->tyretemp[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreCoreTemperature) + (sizeof(float) * 0));
    simdata->tyretemp[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreCoreTemperature) + (sizeof(float) * 1));
    simdata->tyretemp[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreCoreTemperature) + (sizeof(float) * 2));
    simdata->tyretemp[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreCoreTemperature) + (sizeof(float) * 3));

    //simdata->tyrecontact0[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 0));
    //simdata->tyrecontact0[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 1));
    //simdata->tyrecontact0[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 2));
    //simdata->tyrecontact0[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 3));

    //simdata->tyrecontact1[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 4) + (sizeof(float) * 0));
    //simdata->tyrecontact1[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 4) + (sizeof(float) * 1));
    //simdata->tyrecontact1[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 4) + (sizeof(float) * 2));
    //simdata->tyrecontact1[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 4) + (sizeof(float) * 3));

    //simdata->tyrecontact2[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 8) + (sizeof(float) * 0));
    //simdata->tyrecontact2[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 8) + (sizeof(float) * 1));
    //simdata->tyrecontact2[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 8) + (sizeof(float) * 2));
    //simdata->tyrecontact2[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 8) + (sizeof(float) * 3));

    simdata->tyrecontact0[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 0));
    simdata->tyrecontact1[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 1));
    simdata->tyrecontact2[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 2));

    simdata->tyrecontact1[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 3) + (sizeof(float) * 0));
    simdata->tyrecontact1[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 3) + (sizeof(float) * 1));
    simdata->tyrecontact2[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 3) + (sizeof(float) * 2));

    simdata->tyrecontact0[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 6) + (sizeof(float) * 0));
    simdata->tyrecontact1[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 6) + (sizeof(float) * 1));
    simdata->tyrecontact2[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 6) + (sizeof(float) * 2));

    simdata->tyrecontact0[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 9) + (sizeof(float) * 0));
    simdata->tyrecontact1[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 9) + (sizeof(float) * 1));
    simdata->tyrecontact2[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 9) + (sizeof(float) * 2));

    simdata->braketemp[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeTemp) + (sizeof(float) * 0));
    simdata->braketemp[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeTemp) + (sizeof(float) * 1));
    simdata->braketemp[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeTemp) + (sizeof(float) * 2));
    simdata->braketemp[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeTemp) + (sizeof(float) * 3));

    simdata->tyrepressure[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelsPressure) + (sizeof(float) * 0));
    simdata->tyrepressure[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelsPressure) + (sizeof(float) * 1));
    simdata->tyrepressure[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelsPressure) + (sizeof(float) * 2));
    simdata->tyrepressure[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelsPressure) + (sizeof(float) * 3));

    simdata->turboboostperct = 0;
    simdata->turboboostperct = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, turboBoost));

    simdata->airdensity = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, airDensity));
    simdata->airtemp = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, airTemp));
    simdata->tracktemp = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, roadTemp));


    if ( simmap->ac.has_graphic == true )
    {
        c = simmap->ac.graphic_map_addr;

        // temporary workaround for beta data from ACEvo and ACRally
        if(simexe == SIMULATOREXE_ASSETTO_CORSA_EVO || simexe == SIMULATOREXE_ASSETTO_CORSA_RALLY)
        {
            simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;
        }
        else
        {
            simdata->simstatus = *(int*) (char*) (c + offsetof(struct SPageFileGraphic, status));
        }

        simdata->lap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, completedLaps));
        simdata->position = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, position));
        uint32_t lastlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iLastTime));
        simdata->lastlap = ac_convert_to_simdata_laptime(lastlap);
        uint32_t bestlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iBestTime));
        simdata->bestlap = ac_convert_to_simdata_laptime(bestlap);
        uint32_t currentlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iCurrentTime));
        simdata->currentlap = ac_convert_to_simdata_laptime(currentlap);
        //simdata->time = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iCurrentTime));
        simdata->numlaps = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, numberOfLaps));
        simdata->session = *(int32_t*) (char*) (c + offsetof(struct SPageFileGraphic, session));
        simdata->sectorindex = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, currentSectorIndex));
        simdata->lastsectorinms = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, lastSectorTime));
        simdata->playerflag = acc_flag_to_simdata_flag(*(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, Flag)));

        int yellow = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, GlobalYellow));
        int white = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, GlobalWhite));
        int chequered = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, GlobalChequered));
        int green = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, GlobalGreen));
        int red = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, GlobalRed));
        simdata->courseflag = acc_get_global_flag( yellow, white, chequered, green, red);

        int strsize = 32;
        for(int i=0; i<strsize; i++)
        {
            simdata->tyrecompound[i] = *(char*) (char*) ((c + offsetof(struct SPageFileGraphic, tyreCompound)) + (sizeof(char16_t) * i));
        }
        //simdata->tyrecompound = simmap->ac.compound;

        //float timeleft = *(float*) (char*) (c + offsetof(struct SPageFileGraphic, sessionTimeLeft));
        //if (timeleft < 0)
        //    simdata->timeleft = 0;
        //else
        //    simdata->timeleft = droundint(timeleft);
    }

    simdata->maxturbo = 0;
    if (simmap->ac.has_static == true )
    {
        b = simmap->ac.static_map_addr;
        simdata->maxrpm = *(uint32_t*) (char*) (b + offsetof(struct SPageFileStatic, maxRpm));
        simdata->maxturbo = *(float*) (char*) (b + offsetof(struct SPageFileStatic, MaxTurboBoost));

        simdata->tyrediameter[0] = *(float*) (char*) (b + offsetof(struct SPageFileStatic, tyreRadius) + (sizeof(float) * 0));
        simdata->tyrediameter[1] = *(float*) (char*) (b + offsetof(struct SPageFileStatic, tyreRadius) + (sizeof(float) * 1));
        simdata->tyrediameter[2] = *(float*) (char*) (b + offsetof(struct SPageFileStatic, tyreRadius) + (sizeof(float) * 2));
        simdata->tyrediameter[3] = *(float*) (char*) (b + offsetof(struct SPageFileStatic, tyreRadius) + (sizeof(float) * 3));
        for(int j=0; j<4; j++)
        {
            simdata->tyrediameter[j] = simdata->tyrediameter[j] * 2;
        }

        int strsize = 32;
        for(int i=0; i<strsize; i++)
        {
            simdata->car[i] = *(char*) (char*) ((b + offsetof(struct SPageFileStatic, carModel)) + (sizeof(char16_t) * i));
            simdata->track[i] = *(char*) (char*) ((b + offsetof(struct SPageFileStatic, track)) + (sizeof(char16_t) * i));
            simdata->driver[i] = *(char*) (char*) ((b + offsetof(struct SPageFileStatic, playerName)) + (sizeof(char16_t) * i));
        }

    }
    simdata->turboboost = simdata->turboboostperct * simdata->maxturbo;

    // realtime telemetry
    if (simmap->ac.has_crewchief == true && simdata->simexe != SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE )
    {
        d = simmap->ac.crewchief_map_addr;

        simdata->worldposx = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, x)));
        simdata->worldposz = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, y)));
        simdata->worldposy = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, z)));
        double heading = simdata->heading;
        double player_rotation = heading;


        float player_spline = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, spLineLength)));
        float track_spline = *(float*) (char*) (b + offsetof(struct SPageFileStatic, TrackSPlineLength));
        simdata->playerspline = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, spLineLength)));
        simdata->trackspline = *(float*) (char*) (b + offsetof(struct SPageFileStatic, TrackSPlineLength));
        simdata->trackdistancearound = spLineLengthToDistanceRoundTrack(track_spline, player_spline);
        int track_samples = track_spline * 4;
        simdata->tracksamples = track_samples;
        simdata->playertrackpos = (int) simdata->trackdistancearound;

        simdata->numcars = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, numVehicles));
        int numcars = simdata->numcars;
        if (numcars > MAXCARS)
        {
            numcars = MAXCARS;
        }

        int strsize = 32;
        for(int i=0; i<numcars; i++)
        {
            simdata->cars[i].lap = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, lapCount)));
            simdata->cars[i].pos = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, carLeaderboardPosition)));
            uint32_t lastlap = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, lastLapTimeMS)));
            uint32_t bestlap = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, bestLapMS)));
            simdata->cars[i].bestlap = ac_convert_to_simdata_laptime(bestlap);
            simdata->cars[i].lastlap = ac_convert_to_simdata_laptime(lastlap);
            //uint32_t currentlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iCurrentTime));
            //simdata->currentlap = ac_convert_to_simdata_laptime(currentlap);
            simdata->cars[i].inpitlane = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, isCarInPitline)));
            simdata->cars[i].inpit = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, isCarInPit)));

            int strsize = 32;
            for(int k=0; k<strsize; k++)
            {
                simdata->cars[i].driver[k] = *(char*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, driverName)) + (sizeof(char) * k));
                simdata->cars[i].car[k] = *(char*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, carModel)) + (sizeof(char) * k));
            }
            simdata->cars[i].speed = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, speedMS)));

            //simdata->cars[i].carspline = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, spLineLength)));
            float spline = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, spLineLength)));
            simdata->cars[i].trackpos = spLineLengthToDistanceRoundTrack(track_spline, spline);
            //simdata->cars[i].trackpos = (int) track_spline * spline;
            simdata->cars[i].xpos = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, x)));
            simdata->cars[i].zpos = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, y)));
            simdata->cars[i].ypos = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, z)));
        }
        SetProximityData(simdata, numcars, -1);

        simdata->playerlaps = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, lapCount)));
        simdata->lapisvalid = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, currentLapInvalid)));
        simdata->lapisvalid = !simdata->lapisvalid;

        int currentlapinticks = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, currentLapTimeMS)));
        int lastlapinticks = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, lastLapTimeMS)));
        simdata->currentlapinseconds = currentlapinticks * 100000;
        simdata->lastlapinseconds = lastlapinticks * 100000;
    }

    simdata->altitude = 1;
}
