#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "../simmap.h"
#include "../simdata.h"
#include "../simapi.h"
#include "../simmapper.h"
#include "../pcars2.h"

#include "../../include/pcars2data.h"

static int droundint(double d)
{
    return trunc(nearbyint(d));
}

static LapTime pcars2_convert_to_simdata_laptime(float pcars2_laptime)
{
    if(pcars2_laptime <= 0)
    {
        return (LapTime){0, 0, 0};
    }
    LapTime l;
    l.seconds = floor(pcars2_laptime);
    float a = pcars2_laptime-l.seconds;
    l.fraction = a * 1000;
    l.hours = l.seconds/60/60;
    l.minutes = l.seconds/60-(l.hours*60);
    l.seconds = l.seconds-(l.minutes*60);
    return l;
}

static int pcars2_flag_to_simdata_flag(int pcars2_flag)
{
    switch ( pcars2_flag )
    {
        case FLAG_COLOUR_NONE:
            return SIMAPI_FLAG_GREEN;
        case FLAG_COLOUR_GREEN:
            return SIMAPI_FLAG_GREEN;
        case FLAG_COLOUR_BLUE:
            return SIMAPI_FLAG_BLUE;
        case FLAG_COLOUR_WHITE_SLOW_CAR:
            return SIMAPI_FLAG_WHITE;
        case FLAG_COLOUR_WHITE_FINAL_LAP:
            return SIMAPI_FLAG_WHITE;
        case FLAG_COLOUR_RED:
            return SIMAPI_FLAG_RED;
        case FLAG_COLOUR_YELLOW:
            return SIMAPI_FLAG_YELLOW;
        case FLAG_COLOUR_DOUBLE_YELLOW:
            return SIMAPI_FLAG_YELLOW;
        case FLAG_COLOUR_BLACK_AND_WHITE:
            return SIMAPI_FLAG_BLACK_WHITE;
        case FLAG_COLOUR_BLACK_ORANGE_CIRCLE:
            return SIMAPI_FLAG_BLACK_ORANGE;
        case FLAG_COLOUR_BLACK:
            return SIMAPI_FLAG_BLACK;
        case FLAG_COLOUR_CHEQUERED:
            return SIMAPI_FLAG_CHEQUERED;
        default:
            return SIMAPI_FLAG_GREEN;
    }
}

static int pcars2_state_to_simdata_flag(int pcars2_state)
{
    if(pcars2_state == 8)
    {
        return SIMAPI_FLAG_RED;
    }
    if(pcars2_state > 0)
    {
        return SIMAPI_FLAG_YELLOW;
    }
    return SIMAPI_FLAG_GREEN;
}

void map_project_cars2_data(SimData* simdata, SimMap* simmap, bool udp, char* base)
{

    char* a;
    char* b;
    char* c;
    char* d;

    if(udp == false)
    {

        a = simmap->pcars2.telemetry_map_addr;
        // basic telemetry
        uint8_t s = *(uint8_t*) (char*) (a + offsetof(struct pcars2APIStruct, mGameState));
        simdata->simstatus = 0;
        if (s > 1 && s < 5)
        {
            simdata->simstatus = 2;
        }

        simdata->velocity = droundint(3.6 * (*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mSpeed))));
        simdata->rpms = droundint(*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mRpm)));
        simdata->maxrpm = droundint(*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mMaxRPM)));
        simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mGear));
        simdata->fuel = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mFuelLevel));
        simdata->gas = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mThrottle));
        simdata->brake = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mBrake));
        simdata->steer = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mSteering));
        // bool
        simdata->abs = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mAntiLockActive));
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


        // tyre effects
        // m/s
        simdata->Xvelocity = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mLocalVelocity) + (sizeof(float) * 0 ));
        simdata->Zvelocity = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mLocalVelocity) + (sizeof(float) * 1 ));
        simdata->Yvelocity = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mLocalVelocity) + (sizeof(float) * 2 ));

        simdata->worldXvelocity = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mWorldVelocity) + (sizeof(float) * 0 ));
        simdata->worldZvelocity = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mWorldVelocity) + (sizeof(float) * 1 ));
        simdata->worldYvelocity = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mWorldVelocity) + (sizeof(float) * 2 ));


        simdata->tyreRPS[0] = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreRPS) + (sizeof(float) * 0));
        simdata->tyreRPS[1] = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreRPS) + (sizeof(float) * 1));
        simdata->tyreRPS[2] = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreRPS) + (sizeof(float) * 2));
        simdata->tyreRPS[3] = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreRPS) + (sizeof(float) * 3));

        simdata->tyrewear[0] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreWear) + (sizeof(float) * 0));
        simdata->tyrewear[1] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreWear) + (sizeof(float) * 1));
        simdata->tyrewear[2] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreWear) + (sizeof(float) * 2));
        simdata->tyrewear[3] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreWear) + (sizeof(float) * 3));

        simdata->tyretemp[0] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreInternalAirTemp) + (sizeof(float) * 0));
        simdata->tyretemp[1] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreInternalAirTemp) + (sizeof(float) * 1));
        simdata->tyretemp[2] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreInternalAirTemp) + (sizeof(float) * 2));
        simdata->tyretemp[3] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreInternalAirTemp) + (sizeof(float) * 3));

        simdata->braketemp[0] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mBrakeTempCelsius) + (sizeof(float) * 0));
        simdata->braketemp[1] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mBrakeTempCelsius) + (sizeof(float) * 1));
        simdata->braketemp[2] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mBrakeTempCelsius) + (sizeof(float) * 2));
        simdata->braketemp[3] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mBrakeTempCelsius) + (sizeof(float) * 3));

        simdata->tyrepressure[0] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mAirPressure) + (sizeof(float) * 0));
        simdata->tyrepressure[1] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mAirPressure) + (sizeof(float) * 1));
        simdata->tyrepressure[2] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mAirPressure) + (sizeof(float) * 2));
        simdata->tyrepressure[3] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mAirPressure) + (sizeof(float) * 3));

        simdata->tyrecontact0[0] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreY) + (sizeof(float) * 0));
        simdata->tyrecontact0[1] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreY) + (sizeof(float) * 1));
        simdata->tyrecontact0[2] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreY) + (sizeof(float) * 2));
        simdata->tyrecontact0[3] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreY) + (sizeof(float) * 3));

        // advanced ui
        //simdata->airdensity = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, airDensity));
        simdata->airtemp = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mAmbientTemperature));
        simdata->tracktemp = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTrackTemperature));

        simdata->lap = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mCurrentLap));
        simdata->position = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mRacePosition));
        float lastlap = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mLastLapTime));
        float bestlap = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mBestLapTime));
        float currentlap = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mCurrentTime));
        simdata->lastlap = pcars2_convert_to_simdata_laptime(lastlap);
        simdata->bestlap = pcars2_convert_to_simdata_laptime(bestlap);
        simdata->currentlap = pcars2_convert_to_simdata_laptime(currentlap);

        simdata->numlaps = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mLapsInEvent));
        simdata->lapisvalid = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mLapInvalidated));
        simdata->lapisvalid = !simdata->lapisvalid;
        simdata->courseflag = pcars2_state_to_simdata_flag(*(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mYellowFlagState)));
        simdata->playerflag = pcars2_flag_to_simdata_flag(*(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mHighestFlagColour)));

        float trackdist = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTrackLength));
        simdata->trackspline = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTrackLength));
        float pos = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mCurrentLapDistance));
        simdata->playertrackpos = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mCurrentLapDistance));
        simdata->tracksamples = ceil(trackdist);
        int track_samples = ceil(trackdist * 4);
        simdata->playerspline = (pos/trackdist);

        int actsize = 0;
        int actsize2 = 0;
        int actsize3 = 0;
        int strsize = 64;
        for(int i=0; i<strsize; i++)
        {
            simdata->car[i] = *(char*) (char*) (a + offsetof(struct pcars2APIStruct, mCarName) + (sizeof(char)*i));
            simdata->track[i] = *(char*) (char*) (a + offsetof(struct pcars2APIStruct, mTrackLocation) + (sizeof(char)*i));
            simdata->driver[i] = *(char*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mName) + (sizeof(char)*i));
            if(simdata->car[i] != '\0')
            {
                actsize++;
            }
            if(simdata->track[i] != '\0')
            {
                actsize2++;
            }
            if(simdata->driver[i] != '\0')
            {
                actsize3++;
            }
        }
        actsize = 0;
        strsize = 40;
        for(int i=0; i<strsize; i++)
        {
            simdata->tyrecompound[i] = *(char*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreCompound) + (sizeof(char)*i));
            if(simdata->tyrecompound[i] != '\0')
            {
                actsize++;
            }
        }

        simdata->numcars = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mNumParticipants));
        int numcars = simdata->numcars;
        if (numcars > MAXCARS)
        {
            numcars = MAXCARS;
        }
        for(int i=0; i<numcars; i++)
        {
            int actsize = 0;
            int actsize2 = 0;
            int strsize = 64;
            for(int k=0; k<strsize; k++)
            {
                simdata->cars[i].driver[k] = *(char*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mName) + (sizeof(ParticipantInfo)*i) + (sizeof(char)*k));
                simdata->cars[i].car[k] = *(char*) (char*) (a + offsetof(struct pcars2APIStruct, mCarNames) + STRING_LENGTH_MAX + ((sizeof(char) * k)));
                if(simdata->cars[i].driver[k] != '\0')
                {
                    actsize++;
                }
            }
            simdata->cars[i].lap = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mCurrentLap) + (sizeof(ParticipantInfo)*i));
            simdata->cars[i].pos = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mRacePosition) + (sizeof(ParticipantInfo)*i));
            simdata->cars[i].lastlap = pcars2_convert_to_simdata_laptime(*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mLastLapTimes) + ((sizeof(float) * i))));
            simdata->cars[i].bestlap = pcars2_convert_to_simdata_laptime(*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mFastestLapTimes) + ((sizeof(float) * i))));

            simdata->cars[i].speed = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mSpeeds) + ((sizeof(float) * i)));
            // TODO move to it's own function
            simdata->cars[i].inpitlane = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mPitModes) + ((sizeof(float) * i)));
            if(simdata->cars[i].inpitlane == 1 || simdata->cars[i].inpitlane == 3)
            {
                simdata->cars[i].inpitlane = 1;
            }
            else
            {
                simdata->cars[i].inpitlane = 0;
            }
            simdata->cars[i].inpit = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mPitModes) + ((sizeof(float) * i)));
            if(simdata->cars[i].inpit == 2 || simdata->cars[i].inpit > 3)
            {
                simdata->cars[i].inpit = 1;
            }
            else
            {
                simdata->cars[i].inpit = 0;
            }

            simdata->cars[i].trackpos = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mCurrentLapDistance) + (sizeof(ParticipantInfo)*i));

            simdata->cars[i].xpos = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mWorldPosition) + (sizeof(ParticipantInfo)*i) + (sizeof(float) * 0));
            simdata->cars[i].zpos = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mWorldPosition) + (sizeof(ParticipantInfo)*i) + (sizeof(float) * 1));
            simdata->cars[i].ypos = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mWorldPosition) + (sizeof(ParticipantInfo)*i) + (sizeof(float) * 2));
        }

        // realtime telemetry
        simdata->worldposx = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mWorldPosition) + (sizeof(float) * 0));
        simdata->worldposz = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mWorldPosition) + (sizeof(float) * 1));
        simdata->worldposy = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mWorldPosition) + (sizeof(float) * 2));


        SetProximityData(simdata, numcars, 1);
        return;
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
            //char* msg;
            //asprintf(&msg, "project cars 2 packet type %i", packet_type);
            //simapi_log(SIMAPI_LOGLEVEL_TRACE, msg);
            //free(msg);
            switch ( packet_type )
            {
                case 0:
                    simdata->car[0] ='d';
                    simdata->car[1]='e';
                    simdata->car[2]='f';
                    simdata->car[3]='a';
                    simdata->car[4]='u';
                    simdata->car[5]='l';
                    simdata->car[6]='t';
                    simdata->car[7]='\0';

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
        return;
    }
}
