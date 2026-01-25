#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../simmap.h"
#include "../simdata.h"
#include "../simapi.h"
#include "../simmapper.h"
#include "../f1.h"

#include "../../include/f12018.h"

#define head_s sizeof(struct PacketHeader)


void map_f1_2018_data(SimData* simdata, SimMap* simmap, char* base)
{

    if(base != NULL)
    {
        char* a = base;
        uint8_t id = 0;
        id = *(uint8_t*) (char*) (a + 3);
        uint8_t player_index = *(uint8_t*) (char*) (a + 20);

        switch (id)
        {
            case PACKET_ID_CAR_TELEMETRY:
                simdata->velocity = *(uint16_t*) (char*) (a + head_s + sizeof(CarTelemetryData) * player_index + offsetof(CarTelemetryData, m_speed));
                simdata->rpms = *(uint16_t*) (char*) (a + head_s + sizeof(CarTelemetryData) * player_index + offsetof(CarTelemetryData, m_engineRPM));
                simdata->steer = *(int8_t*) (char*) (a + head_s + sizeof(CarTelemetryData) * player_index + offsetof(CarTelemetryData, m_steer));
                simdata->brake = *(uint8_t*) (char*) (a + head_s + sizeof(CarTelemetryData) * player_index + offsetof(CarTelemetryData, m_brake));
                simdata->gas = *(uint8_t*) (char*) (a + head_s + sizeof(CarTelemetryData) * player_index + offsetof(CarTelemetryData, m_throttle));
                simdata->gear = *(uint8_t*) (char*) (a + head_s + sizeof(CarTelemetryData) * player_index + offsetof(CarTelemetryData, m_gear));
                simdata->gear++;
                break;
            case PACKET_ID_CAR_STATUS:
                simdata->maxrpm = *(uint16_t*) (char*) (a + head_s + offsetof(CarStatusData, m_maxRPM));
                break;
            case PACKET_ID_PARTICIPANTS:
                uint8_t numcars = *(uint8_t*) (char*) (a + head_s + offsetof(struct PacketParticipantsData, m_numCars));
                uint8_t i = 0;
                while(i < numcars)
                {

                    uint8_t driverid = *(uint8_t*) (char*) (a + head_s + 1 + (sizeof(ParticipantData) * i) + offsetof(ParticipantData, m_driverId));
                    uint8_t teamid = *(uint8_t*) (char*) (a + head_s + + 1 + (sizeof(ParticipantData) * i) + offsetof(ParticipantData, m_teamId));

                    // not a huge fan of this as it forces you to have an updated array for each release of the game to have accurate team and driver names
                    if(DriverID_name[driverid] != NULL)
                    {
                        if(i == player_index)
                        {
                            strncpy(simdata->driver, DriverID_name[driverid], MAXCARS);
                        }
                        strncpy(simdata->cars[i].driver, DriverID_name[driverid], MAXCARS);
                    }
                    if(TeamID_name[teamid] != NULL)
                    {
                        if(i == player_index)
                        {
                            strncpy(simdata->car, TeamID_name[teamid], MAXCARS);
                        }
                        strncpy(simdata->cars[i].car, TeamID_name[teamid], MAXCARS);
                    }
                    i++;
                }
                break;
            case PACKET_ID_SESSION:
                simdata->tracktemp = *(int8_t*) (char*) (a + head_s + offsetof(struct PacketSessionData, m_trackTemperature));
                simdata->airtemp = *(int8_t*) (char*) (a + head_s + offsetof(struct PacketSessionData, m_airTemperature));
                break;

            case PACKET_ID_LAP_DATA:
                float lastlapseconds = *(float*) (char*) (a + head_s + (sizeof(LapData) * player_index) + offsetof(LapData, m_lastLapTime));
                float currentlapseconds = *(float*) (char*) (a + head_s + (sizeof(LapData) * player_index) + offsetof(LapData, m_currentLapTime));
                float bestlapseconds = *(float*) (char*) (a + head_s + (sizeof(LapData) * player_index) + offsetof(LapData, m_bestLapTime));

                break;
            default:
                break;
        }
    }

}
