#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../../include/dirt2data.h"
#include "../simdata.h"
#include "../simmap.h"
#include "../simmapper.h"

#define DR2_RPM_SCALE 10.0f

static int froundint(float f) { return (int)trunc(nearbyint(f)); }

/**
 * @brief Maps raw DiRT Rally 2.0 UDP telemetry to the universal SimData
 * structure.
 *
 * DiRT Rally 2.0 sends UDP packets in the "extradata=3" format. This function
 * extracts those fields and maps them to the common SimData format used by
 * SimAPI.
 *
 * Key mapping highlights:
 * - RPM: Scaled by 10.0 (internal game representation).
 * - Gear: Shifted by +1 to match SimAPI range (0=R, 1=N, 2=1st...).
 * - Euler Angles: Derived from the Forward and Right vectors provided in the
 * UDP packet.
 * - G-Forces: Lateral (X) and Longitudinal (Y) are provided directly by the
 * game.
 *
 * @param simdata The target SimData structure to populate.
 * @param simmap The simulator mapping context.
 * @param base Raw UDP packet pointer.
 */
void map_dirt_rally_2_data(SimData *simdata, SimMap *simmap, char *base) {
  if (base != NULL) {
    struct dirt2_udp_packet *packet = (struct dirt2_udp_packet *)base;

    if (simmap != NULL) {
      memcpy(&simmap->dirt2.dirt2_telemetry, packet,
             sizeof(struct dirt2_udp_packet));
      simmap->dirt2.has_telemetry = true;
    }

    // Basic telemetry
    simdata->rpms = froundint(packet->fields.engineRPM * DR2_RPM_SCALE);
    simdata->velocity = froundint(packet->fields.speed * 3.6f); // m/s to km/h

    // Gear mapping: game(-1..10) -> simapi(0..11)
    float raw_gear = packet->fields.gear;
    int gear_int = (int)raw_gear;

    if (gear_int < 0) {
      simdata->gear = 0; // Reverse
    } else {
      simdata->gear = (uint32_t)gear_int + 1;
    }

    simdata->maxrpm = froundint(packet->fields.maxRPM * DR2_RPM_SCALE);

    // Character representation of gear
    if (simdata->gear == 0) {
      strcpy(simdata->gearc, "R");
    } else if (simdata->gear == 1) {
      strcpy(simdata->gearc, "N");
    } else {
      sprintf(simdata->gearc, "%d", simdata->gear - 1);
    }

    // Input mapping (already 0.0 to 1.0)
    simdata->gas = (double)packet->fields.throttle;
    simdata->steer = (double)packet->fields.steering;
    simdata->brake = (double)packet->fields.brake;
    simdata->clutch = (double)packet->fields.clutch;

    // World Velocities
    simdata->worldXvelocity = (double)packet->fields.velX;
    simdata->worldYvelocity = (double)packet->fields.velY;
    simdata->worldZvelocity = (double)packet->fields.velZ;

    // Orientation Vectors (DiRT Rally 2.0 extradata=3 provides Right and
    // Forward)
    float rx = packet->fields.rightX;
    float ry = packet->fields.rightY;
    float rz = packet->fields.rightZ;
    float fx = packet->fields.forwardX;
    float fy = packet->fields.forwardY;
    float fz = packet->fields.forwardZ;

    // Up Vector = Forward x Right (Cross Product)
    float ux = fy * rz - fz * ry;
    float uy = fz * rx - fx * rz;
    float uz = fx * ry - fy * rx;

    // G-Forces / Accelerations
    // Lateral and Longitudinal are provided directly by the game.
    simdata->Xvelocity = (double)packet->fields.gForceLat;
    simdata->Yvelocity = (double)packet->fields.gForceLon;
    simdata->Zvelocity =
        0.0; // Vertical G-force not explicitly provided in 66-float struct

    // Store vectors in SimData for legacy support or debugging
    simdata->tyrecontact0[0] = (double)rx;
    simdata->tyrecontact0[1] = (double)ry;
    simdata->tyrecontact0[2] = (double)rz;
    simdata->tyrecontact1[0] = (double)ux;
    simdata->tyrecontact1[1] = (double)uy;
    simdata->tyrecontact1[2] = (double)uz;
    simdata->tyrecontact2[0] = (double)fx;
    simdata->tyrecontact2[1] = (double)fy;
    simdata->tyrecontact2[2] = (double)fz;

    // Convert vectors to Euler angles (Degrees)
    // 1. Yaw (Heading): Direction the car is facing in the XZ plane
    simdata->heading = atan2((double)fx, (double)fz) * (180.0 / M_PI);

    // 2. Pitch (Nose Up +)
    double pitch_input = fmax(-1.0f, fmin(1.0f, (double)fy));
    simdata->pitch = asin(pitch_input) * (180.0 / M_PI);

    // 3. Roll (Right side down +)
    simdata->roll = atan2((double)-ry, (double)uy) * (180.0 / M_PI);

    // Wheel Speeds (m/s)
    simdata->tyreRPS[0] = (double)packet->fields.wheelSpeedRL;
    simdata->tyreRPS[1] = (double)packet->fields.wheelSpeedRR;
    simdata->tyreRPS[2] = (double)packet->fields.wheelSpeedFL;
    simdata->tyreRPS[3] = (double)packet->fields.wheelSpeedFR;

    // Game Status
    // DiRT Rally 2.0 extradata=3 is typically only sent when the simulation is
    // active. We set status to ACTIVEPLAY even if runTime is 0 to support Rally
    // Cross countdowns where telemetry starts before the clock.
    if (packet->fields.runTime >= 0) {
      simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;
    } else {
      simdata->simstatus = SIMAPI_STATUS_MENU;
    }

    // Lap and Progress
    simdata->lap = (uint32_t)packet->fields.lap;
    simdata->trackdistancearound = (double)packet->fields.trackLength;
    simdata->playerspline = (double)packet->fields.progress;

    // Position
    simdata->worldposx = (double)packet->fields.posX;
    simdata->worldposy = (double)packet->fields.posY;
    simdata->worldposz = (double)packet->fields.posZ;

    // Fluids
    simdata->fuel = (double)packet->fields.fuelInTank;

    // Brake Temperatures
    simdata->braketemp[0] = (double)packet->fields.brakesTempRL;
    simdata->braketemp[1] = (double)packet->fields.brakesTempRR;
    simdata->braketemp[2] = (double)packet->fields.brakesTempFL;
    simdata->braketemp[3] = (double)packet->fields.brakesTempFR;

    // Tyre Pressures
    simdata->tyrepressure[0] = (double)packet->fields.tyrePressureRL;
    simdata->tyrepressure[1] = (double)packet->fields.tyrePressureRR;
    simdata->tyrepressure[2] = (double)packet->fields.tyrePressureFL;
    simdata->tyrepressure[3] = (double)packet->fields.tyrePressureFR;

    // Suspension positions
    simdata->suspension[0] = (double)packet->fields.suspRL;
    simdata->suspension[1] = (double)packet->fields.suspRR;
    simdata->suspension[2] = (double)packet->fields.suspFL;
    simdata->suspension[3] = (double)packet->fields.suspFR;

    simdata->suspvelocity[0] = (double)packet->fields.suspVelRL;
    simdata->suspvelocity[1] = (double)packet->fields.suspVelRR;
    simdata->suspvelocity[2] = (double)packet->fields.suspVelFL;
    simdata->suspvelocity[3] = (double)packet->fields.suspVelFR;

    simdata->distance = (double)packet->fields.distance;
    simdata->position = (uint32_t)packet->fields.racePos;
    simdata->numlaps = (uint32_t)packet->fields.totalLaps;
    simdata->playerlaps = (uint32_t)packet->fields.lapsCompleted;
    simdata->sectorindex = (uint8_t)packet->fields.sector;
    simdata->sector1time = (double)packet->fields.sector1Time;
    simdata->sector2time = (double)packet->fields.sector2Time;
    simdata->fuelcapacity = (double)packet->fields.fuelCapacity;
    simdata->idlerpm = froundint(packet->fields.idleRPM * DR2_RPM_SCALE);
    simdata->maxgears = (uint32_t)packet->fields.maxGears;

    // Timing
    simdata->currentlapinseconds = (uint32_t)packet->fields.lapTime;
    simdata->lastlapinseconds = (uint32_t)packet->fields.lastLapTime;

    // Splitting seconds into LapTime
    simdata->currentlap.hours = (uint32_t)(packet->fields.lapTime / 3600);
    simdata->currentlap.minutes =
        (uint32_t)(((int)packet->fields.lapTime % 3600) / 60);
    simdata->currentlap.seconds = (uint32_t)((int)packet->fields.lapTime % 60);
    simdata->currentlap.fraction =
        (uint32_t)((packet->fields.lapTime - floor(packet->fields.lapTime)) *
                   1000);

    simdata->lastlap.hours = (uint32_t)(packet->fields.lastLapTime / 3600);
    simdata->lastlap.minutes =
        (uint32_t)(((int)packet->fields.lastLapTime % 3600) / 60);
    simdata->lastlap.seconds = (uint32_t)((int)packet->fields.lastLapTime % 60);
    simdata->lastlap.fraction = (uint32_t)((packet->fields.lastLapTime -
                                            floor(packet->fields.lastLapTime)) *
                                           1000);

    strncpy(simdata->car, "DR2", 4);
  }
}
