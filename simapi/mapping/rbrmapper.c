#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../../include/rbrdata.h"
#include "../simdata.h"
#include "../simmap.h"
#include "../simmapper.h"

static int froundint(float f)
{
    return (int)trunc(nearbyint(f));
}

/**
 * @brief Maps raw Richard Burns Rally UDP telemetry to the universal SimData
 * structure.
 *
 * Richard Burns Rally sends UDP packets containing comprehensive telemetry data.
 * This function extracts those fields and maps them to the common SimData format
 * used by SimAPI.
 *
 * Key mapping highlights:
 * - Speed: Provided by RBR (likely in kph or mph, assuming kph)
 * - RPM: Directly from engine data
 * - Gear: RBR uses 0=Neutral, 1-6=gears, -1=Reverse (needs shifting for SimAPI)
 * - Orientation: Roll, pitch, yaw are provided directly
 * - Wheels: Suspension and tire data available per wheel (LF, RF, LB, RB)
 *
 * @param simdata The target SimData structure to populate.
 * @param simmap The simulator mapping context.
 * @param base Raw UDP packet pointer.
 */
void map_richard_burns_rally_data(SimData *simdata, SimMap *simmap, char* base)
{
    if (base != NULL)
    {
        RBR_TelemetryData *packet = (RBR_TelemetryData *)base;

        if (simmap != NULL)
        {
            memcpy(&simmap->rbr.rbr_telemetry, packet, sizeof(RBR_TelemetryData));
            simmap->rbr.has_telemetry = true;
        }

        // Debug output - print first few values to check data integrity
        static int debug_counter = 0;
        if (debug_counter++ % 60 == 0) { // Print every 60 packets
            fprintf(stderr, "RBR Debug: totalSteps=%u, speed=%.2f, rpm=%.2f, gear=%d, throttle=%.2f\n",
                    packet->totalSteps_, packet->car_.speed_, packet->car_.engine_.rpm_,
                    packet->control_.gear_, packet->control_.throttle_);
        }

        // Basic telemetry
        simdata->rpms = froundint(packet->car_.engine_.rpm_);
        simdata->velocity = froundint(packet->car_.speed_); // Assuming kph

        // RBR doesn't provide max/idle RPM or max gears in telemetry, set to 0
        simdata->maxrpm = 0;
        simdata->idlerpm = 0;
        simdata->maxgears = 0;

        // Gear mapping: RBR uses -1=R, 0=N, 1-6=gears -> SimAPI uses 0=R, 1=N, 2-7=gears
        int gear_int = packet->control_.gear_;
        if (gear_int < 0)
        {
            simdata->gear = 0; // Reverse
        }
        else if (gear_int == 0)
        {
            simdata->gear = 1; // Neutral
        }
        else
        {
            simdata->gear = (uint32_t)gear_int + 1; // Forward gears
        }

        // Character representation of gear
        if (simdata->gear == 0)
        {
            strcpy(simdata->gearc, "R");
        }
        else if (simdata->gear == 1)
        {
            strcpy(simdata->gearc, "N");
        }
        else
        {
            sprintf(simdata->gearc, "%d", simdata->gear - 1);
        }

        // Input mapping (already 0.0 to 1.0 presumably)
        simdata->gas = (double)packet->control_.throttle_;
        simdata->steer = (double)packet->control_.steering_;
        simdata->brake = (double)packet->control_.brake_;
        simdata->clutch = (double)packet->control_.clutch_;

        // World Velocities
        simdata->worldXvelocity = (double)packet->car_.velocities_.surge_;
        simdata->worldYvelocity = (double)packet->car_.velocities_.heave_;
        simdata->worldZvelocity = (double)packet->car_.velocities_.sway_;

        // Orientation (RBR provides angles directly)
        // Converting from radians to degrees
        simdata->heading = (double)packet->car_.yaw_ * (180.0 / M_PI);
        simdata->pitch = (double)packet->car_.pitch_ * (180.0 / M_PI);
        simdata->roll = (double)packet->car_.roll_ * (180.0 / M_PI);

        // Accelerations (G-Forces)
        // RBR provides accelerations in m/s^2, convert to G-forces (divide by 9.81)
        simdata->Xvelocity = (double)packet->car_.accelerations_.surge_ / 9.81;
        simdata->Yvelocity = (double)packet->car_.accelerations_.sway_ / 9.81;
        simdata->Zvelocity = (double)packet->car_.accelerations_.heave_ / 9.81;

        // Position in world coordinates
        simdata->worldposx = (double)packet->car_.positionX_;
        simdata->worldposy = (double)packet->car_.positionY_;
        simdata->worldposz = (double)packet->car_.positionZ_;

        // Suspension travel (RBR order: LF, RF, LB, RB)
        // SimAPI uses: RL, RR, FL, FR (Rear Left, Rear Right, Front Left, Front Right)
        // Mapping: LF->FL(2), RF->FR(3), LB->RL(0), RB->RR(1)
        simdata->suspension[2] = (double)packet->car_.suspensionLF_.springDeflection_;
        simdata->suspension[3] = (double)packet->car_.suspensionRF_.springDeflection_;
        simdata->suspension[0] = (double)packet->car_.suspensionLB_.springDeflection_;
        simdata->suspension[1] = (double)packet->car_.suspensionRB_.springDeflection_;

        // Suspension velocities
        simdata->suspvelocity[2] = (double)packet->car_.suspensionLF_.damper_.pistonVelocity_;
        simdata->suspvelocity[3] = (double)packet->car_.suspensionRF_.damper_.pistonVelocity_;
        simdata->suspvelocity[0] = (double)packet->car_.suspensionLB_.damper_.pistonVelocity_;
        simdata->suspvelocity[1] = (double)packet->car_.suspensionRB_.damper_.pistonVelocity_;

        // Tire temperatures (using tread temperature)
        simdata->tyretemp[2] = (double)packet->car_.suspensionLF_.wheel_.tire_.treadTemperature_;
        simdata->tyretemp[3] = (double)packet->car_.suspensionRF_.wheel_.tire_.treadTemperature_;
        simdata->tyretemp[0] = (double)packet->car_.suspensionLB_.wheel_.tire_.treadTemperature_;
        simdata->tyretemp[1] = (double)packet->car_.suspensionRB_.wheel_.tire_.treadTemperature_;

        // Tire pressures
        simdata->tyrepressure[2] = (double)packet->car_.suspensionLF_.wheel_.tire_.pressure_;
        simdata->tyrepressure[3] = (double)packet->car_.suspensionRF_.wheel_.tire_.pressure_;
        simdata->tyrepressure[0] = (double)packet->car_.suspensionLB_.wheel_.tire_.pressure_;
        simdata->tyrepressure[1] = (double)packet->car_.suspensionRB_.wheel_.tire_.pressure_;

        // Brake temperatures
        simdata->braketemp[2] = (double)packet->car_.suspensionLF_.wheel_.brakeDisk_.temperature_;
        simdata->braketemp[3] = (double)packet->car_.suspensionRF_.wheel_.brakeDisk_.temperature_;
        simdata->braketemp[0] = (double)packet->car_.suspensionLB_.wheel_.brakeDisk_.temperature_;
        simdata->braketemp[1] = (double)packet->car_.suspensionRB_.wheel_.brakeDisk_.temperature_;

        // Engine temperatures (using available fields)
        // simdata->airtemp = (double)packet->car_.engine_.engineTemperature_;
        // simdata->tracktemp = (double)packet->car_.engine_.engineCoolantTemperature_;

        // Stage/Race information
        simdata->trackdistancearound = (double)packet->stage_.distanceToEnd_;
        simdata->playerspline = (double)packet->stage_.progress_;

        // Timing
        simdata->currentlapinseconds = (uint32_t)packet->stage_.raceTime_;

        // Split seconds into LapTime structure
        simdata->currentlap.hours = (uint32_t)(packet->stage_.raceTime_ / 3600);
        simdata->currentlap.minutes =
            (uint32_t)(((int)packet->stage_.raceTime_ % 3600) / 60);
        simdata->currentlap.seconds = (uint32_t)((int)packet->stage_.raceTime_ % 60);
        simdata->currentlap.fraction =
            (uint32_t)((packet->stage_.raceTime_ - floor(packet->stage_.raceTime_)) * 1000);

        // Game Status
        // RBR is actively sending telemetry, so assume active play
        simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;

        strncpy(simdata->car, "RBR", 4);
    }
}
