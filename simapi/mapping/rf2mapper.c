#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>

#include "../simmap.h"
#include "../simdata.h"
#include "../simapi.h"
#include "../simmapper.h"
#include "../rf2.h"

#include "../../include/rf2data.h"

static int droundint(double d)
{
    return trunc(nearbyint(d));
}

static LapTime rf2_convert_to_simdata_laptime(double rf2_laptime)
{
    if(rf2_laptime <= 0)
    {
        return (LapTime){0, 0, 0};
    }
    LapTime l;
    l.hours = rf2_laptime/60/60;
    l.minutes = rf2_laptime/60-(l.hours*60);
    l.seconds = rf2_laptime-(l.minutes*60);
    l.fraction = (rf2_laptime*1000)-(l.minutes*60000)-(l.seconds*1000);
    return l;
}

static int rf2_phase_to_simdata_flag(int rf2_flag)
{

    int courseflag = 0;
    if(rf2_flag < 5 || rf2_flag > 8)
    {
        courseflag = 0;
    }
    else
    {
        courseflag = rf2_flag - 5;
    }

    return courseflag;
}

static int rf2_flag_to_simdata_flag(int rf2_flag)
{

    int playerflag = 0;
    if(rf2_flag == 6)
    {
        playerflag = 4;
    }

    return playerflag;
}


void map_rfactor2_data(SimData* simdata, SimMap* simmap)
{

    char* a;
    char* b;

    a = simmap->rf2.telemetry_map_addr;

    // Find the player vehicle.

    int sco = 0; // Index of player vehicle in scoring.
    int veh = 0; // Index of player vehicle in telemetry.

    if (simmap->rf2.has_scoring == true) {
        b = simmap->rf2.scoring_map_addr;

        int id = -1;
        int num_scored_vehicles = *(int *)(char*)(b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mNumVehicles));
        for (int i = 0; i < num_scored_vehicles; i++) {
            signed char control = *(signed char*)(char*)(b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * i) + offsetof(rF2VehicleScoring, mControl));
            if (control == 0) {
                sco = i;
                id = *(int*)(char*)(b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * i) + offsetof(rF2VehicleScoring, mID));
                break;
            }
        }

        if (id != -1) {
            int num_telemetry_vehicles = *(int*)(char*)(a + offsetof(struct rF2Telemetry, mNumVehicles));
            for (int i = 0; i < num_telemetry_vehicles; i++) {
                int slot_id = *(int*)(char*)(a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * i) + offsetof(rF2VehicleTelemetry, mID));
                if (id == slot_id) {
                    veh = i;
                    break;
                }
            }
        }
    }

    // basic telemetry


    simdata->velocity = abs(droundint(3.6 * (*(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + ((sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mLocalVel)) + (sizeof(double) * 2)))));
    simdata->rpms = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + ((sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mEngineRPM)));
    simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + ((sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mGear)));
    simdata->maxrpm = droundint( *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + ((sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mEngineMaxRPM))));
    simdata->gas = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mUnfilteredThrottle));
    simdata->brake = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mUnfilteredBrake));
    simdata->clutch = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mUnfilteredClutch));
    simdata->steer = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mUnfilteredSteering));
    simdata->fuel = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mFuel));
    simdata->brakebias = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mRearBrakeBias));
    simdata->handbrake = 0;
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
    simdata->gear += 1;

    // tyre effects
    //simdata->abs = *(float*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles));
    simdata->tyreRPS[0] = -1 * *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mRotation) + (sizeof(TelemWheelV01) * 0)));
    simdata->tyreRPS[1] = -1 * *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mRotation) + (sizeof(TelemWheelV01) * 1)));
    simdata->tyreRPS[2] = -1 * *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mRotation) + (sizeof(TelemWheelV01) * 2)));
    simdata->tyreRPS[3] = -1 * *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mRotation) + (sizeof(TelemWheelV01) * 3)));

    simdata->Xvelocity = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mLocalVel) + (sizeof(double) * 0 ));
    simdata->Zvelocity = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mLocalVel) + (sizeof(double) * 1 ));
    simdata->Yvelocity = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mLocalVel) + (sizeof(double) * 2 ));

    rF2Vec3 orix = *(rF2Vec3*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mOri) + (sizeof(rF2Vec3) * 0 ));
    rF2Vec3 oriy = *(rF2Vec3*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mOri) + (sizeof(rF2Vec3) * 1 ));
    rF2Vec3 oriz = *(rF2Vec3*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mOri) + (sizeof(rF2Vec3) * 2 ));

    simdata->worldXvelocity = (orix.x * simdata->Xvelocity) + (orix.z * simdata->Yvelocity) + (orix.y * simdata->Zvelocity);
    simdata->worldYvelocity = (oriz.x * simdata->Xvelocity) + (oriz.z * simdata->Yvelocity) + (oriz.y * simdata->Zvelocity);
    simdata->worldZvelocity = (oriy.x * simdata->Xvelocity) + (oriy.z * simdata->Yvelocity) + (oriy.y * simdata->Zvelocity);

    simdata->Xvelocity = -1 * simdata->Xvelocity;
    simdata->Yvelocity = -1 * simdata->Yvelocity;
    simdata->Zvelocity = -1 * simdata->Zvelocity;

    //advanced ui
    if (simmap->rf2.has_scoring == true )
    {
        uint8_t s = *(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mGamePhase));
        // TODO: will need to track something additional since on session over a value of 8 will still be present when
        // the user has returned to the menu
        if (s > 2)
        {
            simdata->simstatus = 2;
        }
        else
        {
            simdata->simstatus = 0;
        }
        s = *(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mGamePhase));
        switch (s)
        {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 9:
                simdata->session = 0;
            case 5:
            case 6:
            case 7:
            case 8:
                simdata->session = 1;
            case 10:
            case 11:
            case 12:
            case 13:
                simdata->session = 2;
            default:
                simdata->session = 0;
        }


        simdata->tyrewear[0] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mWear) + (sizeof(TelemWheelV01) * 0)));
        simdata->tyrewear[1] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mWear) + (sizeof(TelemWheelV01) * 1)));
        simdata->tyrewear[2] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mWear) + (sizeof(TelemWheelV01) * 2)));
        simdata->tyrewear[3] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mWear) + (sizeof(TelemWheelV01) * 3)));

        simdata->tyretemp[0] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mTireCarcassTemperature) + (sizeof(TelemWheelV01) * 0)));
        simdata->tyretemp[1] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mTireCarcassTemperature) + (sizeof(TelemWheelV01) * 1)));
        simdata->tyretemp[2] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mTireCarcassTemperature) + (sizeof(TelemWheelV01) * 2)));
        simdata->tyretemp[3] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mTireCarcassTemperature) + (sizeof(TelemWheelV01) * 3)));

        for(int k = 0; k<4; k++)
        {
            simdata->tyretemp[k] = simdata->tyretemp[k] - 273.15;
        }

        simdata->braketemp[0] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mBrakeTemp) + (sizeof(TelemWheelV01) * 0)));
        simdata->braketemp[1] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mBrakeTemp) + (sizeof(TelemWheelV01) * 1)));
        simdata->braketemp[2] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mBrakeTemp) + (sizeof(TelemWheelV01) * 2)));
        simdata->braketemp[3] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mBrakeTemp) + (sizeof(TelemWheelV01) * 3)));

        simdata->tyrepressure[0] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mPressure) + (sizeof(TelemWheelV01) * 0)));
        simdata->tyrepressure[1] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mPressure) + (sizeof(TelemWheelV01) * 1)));
        simdata->tyrepressure[2] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mPressure) + (sizeof(TelemWheelV01) * 2)));
        simdata->tyrepressure[3] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mPressure) + (sizeof(TelemWheelV01) * 3)));

        simdata->airtemp = *(double*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mAmbientTemp));
        simdata->tracktemp = *(double*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mTrackTemp));

        double trackdist = *(double*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mLapDist));
        double pos = *(double*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * sco) + offsetof(rF2VehicleScoring, mLapDist));
        if(pos < 0)
        {
            pos = (-1 * pos) + .5;
        }
        simdata->tracksamples = ceil(trackdist * 4);
        simdata->playerspline = (pos/trackdist);

        simdata->lap = *(uint32_t*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mLapNumber));
        simdata->lap++;
        simdata->position = *(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * sco) + offsetof(rF2VehicleScoring, mPlace));

        simdata->lastlap = rf2_convert_to_simdata_laptime(*(double*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * sco) + offsetof(rF2VehicleScoring, mLastLapTime)));
        simdata->bestlap = rf2_convert_to_simdata_laptime(*(double*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * sco) + offsetof(rF2VehicleScoring, mBestLapTime)));
        simdata->currentlap = rf2_convert_to_simdata_laptime(*(double*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * sco) + offsetof(rF2VehicleScoring, mTimeIntoLap)));


        simdata->numlaps = *(uint32_t*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mMaxLaps));
        if(simdata->numlaps == INT_MAX)
        {
            simdata->numlaps = 0;
        }
        //simdata->session
        simdata->sectorindex = *(uint32_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * sco) + offsetof(rF2VehicleScoring, mSector));
        //simdata->lastsectorinms
        simdata->playerflag = rf2_flag_to_simdata_flag(*(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * sco) + offsetof(rF2VehicleScoring, mFlag)));
        simdata->courseflag = rf2_phase_to_simdata_flag(*(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mGamePhase)));
        double z = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mElapsedTime));
        simdata->sessiontime = rf2_convert_to_simdata_laptime(*(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mElapsedTime)));

        // Car and Track
        size_t actsize = 0;
        size_t actsize2 = 0;
        int strsize = 64;
        for(int i=0; i<strsize; i++)
        {
            simdata->car[i] = *(char*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * sco) + offsetof(rF2VehicleScoring, mVehicleName) + (sizeof(char)*i));
            //simdata->track[i] = *(char*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mTrackName) + (sizeof(char)*i));
            simdata->track[i] = *(char*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mTrackName) + (sizeof(char)*i));
            if(simdata->car[i] != '\0')
            {
                actsize++;
            }
            if(simdata->track[i] != '\0')
            {
                actsize2++;
            }
        }

        // Driver
        strsize = 32;
        actsize = 0;
        for(int i=0; i<strsize; i++)
        {
            simdata->driver[i] = *(char*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * sco) + offsetof(rF2VehicleScoring, mDriverName) + (sizeof(char)*i));
            if(simdata->driver[i] != '\0')
            {
                actsize++;
            }
        }
        simdata->driver[32] = '\0';

        //Tyre Compound
        strsize = 18;
        actsize = 0;
        for(int i=0; i<strsize; i++)
        {
            simdata->tyrecompound[i] = *(char*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mRearTireCompoundName) + (sizeof(char)*i));
            if(simdata->tyrecompound[i] != '\0')
            {
                actsize++;
            }
        }

        simdata->numcars = *(uint32_t*) (char*) (a + offsetof(struct rF2Telemetry, mNumVehicles));
        int numcars = simdata->numcars;
        if (numcars > MAXCARS)
        {
            numcars = MAXCARS;
        }
        for(int i=0; i<numcars; i++)
        {

            simdata->cars[i].lap = *(uint32_t*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * i) + offsetof(rF2VehicleTelemetry, mLapNumber));
            simdata->cars[i].pos = *(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * i) + offsetof(rF2VehicleScoring, mPlace));
            uint8_t pitstate = *(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * i) + offsetof(rF2VehicleScoring, mPitState));
            uint8_t garagestall = *(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * i) + offsetof(rF2VehicleScoring, mInGarageStall));
            simdata->cars[i].ingarage = false;
            simdata->cars[i].inpitstopped = false;
            simdata->cars[i].inpitentrance = false;
            simdata->cars[i].inpitexit = false;
            if(pitstate == 2)
            {
                simdata->cars[i].inpitentrance = true;
            }
            if(pitstate == 3)
            {
                simdata->cars[i].inpitstopped = true;
            }
            if(pitstate == 4)
            {
                simdata->cars[i].inpitexit = true;
            }
            if(pitstate >= 2)
            {
                simdata->cars[i].inpit = true;
            }
            if(pitstate == 2 || pitstate == 4)
            {
                simdata->cars[i].inpitlane = true;
            }
            if(garagestall > 0)
            {
                simdata->cars[i].ingarage = true;
            }
            if(simdata->cars[i].ingarage == true)
            {
                simdata->cars[i].inpit = true;
            }

            simdata->cars[i].lastlap = rf2_convert_to_simdata_laptime(*(double*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * i) + offsetof(rF2VehicleScoring, mLastLapTime)));
            simdata->cars[i].bestlap = rf2_convert_to_simdata_laptime(*(double*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * i) + offsetof(rF2VehicleScoring, mBestLapTime)));


            strsize = 32;
            actsize = 0;
            for(int k=0; k<strsize; k++)
            {
                simdata->cars[i].driver[k] = *(char*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mDriverName) + (sizeof(rF2VehicleScoring) * i) + (sizeof(char)*k));
                if(simdata->cars[i].driver[k] != '\0')
                {
                    actsize++;
                }
            }

            actsize = 0;
            strsize = 64;
            for(int k=0; k<strsize; k++)
            {
                simdata->cars[i].car[k] = *(char*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mVehicleName) + (sizeof(rF2VehicleScoring) * i) + (sizeof(char)*k));
                if(simdata->cars[i].driver[k] != '\0')
                {
                    actsize++;
                }
            }

            simdata->cars[i].xpos = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * i) + offsetof(rF2VehicleTelemetry, mPos) + (sizeof(double) * 0 ));
            simdata->cars[i].zpos = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * i) + offsetof(rF2VehicleTelemetry, mPos) + (sizeof(double) * 1 ));
            simdata->cars[i].ypos = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * i) + offsetof(rF2VehicleTelemetry, mPos) + (sizeof(double) * 2 ));
        }


        simdata->worldposx = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mPos) + (sizeof(double) * 0 ));
        simdata->worldposz = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mPos) + (sizeof(double) * 1 ));
        simdata->worldposy = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * veh) + offsetof(rF2VehicleTelemetry, mPos) + (sizeof(double) * 2 ));

        SetProximityData(simdata, numcars, 1);
    }

}
