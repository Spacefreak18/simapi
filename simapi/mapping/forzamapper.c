#include <stddef.h>
#include <math.h>
#include <string.h>

#include "../simdata.h"
#include "../simmapper.h"
#include "../forzadef.h"

#include "../../include/forza.h"

void map_forza_data(SimData* simdata, SimMap* simmap, char* base)
{
    if(base == NULL)
    {
        return;
    }

    char* a = base;

    /* ---------------- Simulation state ---------------- */

    int32_t raceon = *(int32_t*)(a + offsetof(Forza_Dash, IsRaceOn));

    if(raceon == 1)
    {
        simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;
    }
    else
    {
        simdata->simstatus = SIMAPI_STATUS_MENU;
    }

    simdata->mtick = *(uint32_t*)(a + offsetof(Forza_Dash, TimestampMS));

    /* ---------------- Engine / drivetrain ---------------- */

    simdata->rpms = *(float*)(a + offsetof(Forza_Dash, CurrentEngineRpm));
    simdata->maxrpm = *(float*)(a + offsetof(Forza_Dash, EngineMaxRpm));
    simdata->idlerpm = *(float*)(a + offsetof(Forza_Dash, EngineIdleRpm));

    uint8_t gear = *(uint8_t*)(a + offsetof(Forza_Dash, Gear));
    simdata->gear = gear + 1;

    /* ---------------- Speed & velocity ---------------- */

    float speed_ms = *(float*)(a + offsetof(Forza_Dash, Speed));
    simdata->velocity = (uint32_t)(speed_ms * 3.6f);

    simdata->Xvelocity = *(float*)(a + offsetof(Forza_Dash, VelocityX));
    simdata->Yvelocity = *(float*)(a + offsetof(Forza_Dash, VelocityY));
    simdata->Zvelocity = *(float*)(a + offsetof(Forza_Dash, VelocityZ));

    /* ---------------- Position & orientation ---------------- */

    simdata->worldposx = *(float*)(a + offsetof(Forza_Dash, PositionX));
    simdata->worldposy = *(float*)(a + offsetof(Forza_Dash, PositionY));
    simdata->worldposz = *(float*)(a + offsetof(Forza_Dash, PositionZ));

    simdata->heading = *(float*)(a + offsetof(Forza_Dash, Yaw));
    simdata->pitch   = *(float*)(a + offsetof(Forza_Dash, Pitch));
    simdata->roll    = *(float*)(a + offsetof(Forza_Dash, Roll));

    /* ---------------- Inputs ---------------- */

    simdata->gas   = *(uint8_t*)(a + offsetof(Forza_Dash, Accel));
    simdata->brake = *(uint8_t*)(a + offsetof(Forza_Dash, Brake));
    simdata->clutch = *(uint8_t*)(a + offsetof(Forza_Dash, Clutch));
    simdata->handbrake = *(uint8_t*)(a + offsetof(Forza_Dash, HandBrake));

    simdata->steer = *(int8_t*)(a + offsetof(Forza_Dash, Steer));

    /* ---------------- Fuel / boost ---------------- */

    simdata->fuel = *(float*)(a + offsetof(Forza_Dash, Fuel));
    simdata->turboboost = *(float*)(a + offsetof(Forza_Dash, Boost));

    /* ---------------- Suspension ---------------- */

    simdata->suspension[0] = *(float*)(a + offsetof(Forza_Dash, SuspensionTravelMetersFrontLeft));
    simdata->suspension[1] = *(float*)(a + offsetof(Forza_Dash, SuspensionTravelMetersFrontRight));
    simdata->suspension[2] = *(float*)(a + offsetof(Forza_Dash, SuspensionTravelMetersRearLeft));
    simdata->suspension[3] = *(float*)(a + offsetof(Forza_Dash, SuspensionTravelMetersRearRight));

    /* ---------------- Wheel angular velocity (rad/s) ---------------- */

    simdata->tyreRPS[0] = *(float*)(a + offsetof(Forza_Dash, WheelRotationSpeedFrontLeft));
    simdata->tyreRPS[1] = *(float*)(a + offsetof(Forza_Dash, WheelRotationSpeedFrontRight));
    simdata->tyreRPS[2] = *(float*)(a + offsetof(Forza_Dash, WheelRotationSpeedRearLeft));
    simdata->tyreRPS[3] = *(float*)(a + offsetof(Forza_Dash, WheelRotationSpeedRearRight));

    /* ---------------- Tire temps ---------------- */

    simdata->tyretemp[0] = *(float*)(a + offsetof(Forza_Dash, TireTempFrontLeft));
    simdata->tyretemp[1] = *(float*)(a + offsetof(Forza_Dash, TireTempFrontRight));
    simdata->tyretemp[2] = *(float*)(a + offsetof(Forza_Dash, TireTempRearLeft));
    simdata->tyretemp[3] = *(float*)(a + offsetof(Forza_Dash, TireTempRearRight));

    /* ---------------- Tire wear ---------------- */

    simdata->tyrewear[0] = *(float*)(a + offsetof(Forza_Dash, TireWearFrontLeft));
    simdata->tyrewear[1] = *(float*)(a + offsetof(Forza_Dash, TireWearFrontRight));
    simdata->tyrewear[2] = *(float*)(a + offsetof(Forza_Dash, TireWearRearLeft));
    simdata->tyrewear[3] = *(float*)(a + offsetof(Forza_Dash, TireWearRearRight));

    /* ---------------- Lap data ---------------- */

    float best = *(float*)(a + offsetof(Forza_Dash, BestLap));
    float last = *(float*)(a + offsetof(Forza_Dash, LastLap));
    float current = *(float*)(a + offsetof(Forza_Dash, CurrentLap));

    simdata->bestlap.seconds = (uint32_t)best;
    simdata->lastlap.seconds = (uint32_t)last;
    simdata->currentlap.seconds = (uint32_t)current;

    simdata->lap = *(uint16_t*)(a + offsetof(Forza_Dash, LapNumber));
    simdata->position = *(uint8_t*)(a + offsetof(Forza_Dash, RacePosition));

    simdata->distance = *(float*)(a + offsetof(Forza_Dash, DistanceTraveled));
}
