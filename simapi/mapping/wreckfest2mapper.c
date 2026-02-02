#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../simmap.h"
#include "../simdata.h"
#include "../simapi.h"
#include "../simmapper.h"
#include "../wreckfest2.h"
#include "../../include/wreckfest2data.h"

// Forward declarations for helper functions
static void map_main_packet(SimData* simdata, SimMap* simmap, char* base);
static void quaternion_to_euler(float qx, float qy, float qz, float qw,
                                 double* heading, double* pitch, double* roll);

/**
 * @brief Maps Wreckfest 2 Pino UDP telemetry to universal SimData
 *
 * Wreckfest 2 uses multi-packet telemetry at different frequencies:
 * - Main packet (60Hz) - Player telemetry, session data
 * - Participants packets (60Hz) - Leaderboard, timing, motion
 * - Damage/Info packets (2Hz/1Hz)
 *
 * All packets must be synchronized by sessionTime field in header.
 */
void map_wreckfest2_data(SimData* simdata, SimMap* simmap, char* base)
{
    if (base == NULL) return;

    // Identify packet type from header
    WF2_PacketHeader* header = (WF2_PacketHeader*)base;

    // Verify Pino packet signature (1869769584)
    if (header->signature != 1869769584) {
        return;
    }

    // Route based on packet type
    // For now, we primarily handle the Main packet which contains all essential player data
    switch (header->packetType) {
        case WF2_PACKET_TYPE_MAIN:
            map_main_packet(simdata, simmap, base);
            break;
        case WF2_PACKET_TYPE_PARTICIPANTS_LEADERBOARD:
            // Future: Handle participant leaderboard data for multiplayer
            break;
        case WF2_PACKET_TYPE_PARTICIPANTS_TIMING:
            // Future: Handle participant timing data for multiplayer
            break;
        default:
            // Ignore other packet types for now
            break;
    }
}

/**
 * @brief Maps the main telemetry packet containing player data
 */
static void map_main_packet(SimData* simdata, SimMap* simmap, char* base)
{
    WF2_PacketMain* packet = (WF2_PacketMain*)base;

    // Store packet for synchronization
    if (simmap != NULL) {
        memcpy(&simmap->wf2.main_packet, packet, sizeof(*packet));
        simmap->wf2.has_main_packet = true;
        simmap->wf2.last_session_time = packet->header.sessionTime;
    }

    // Basic telemetry from Car::Full
    simdata->rpms = (uint32_t)packet->carPlayer.engine.rpm;
    simdata->maxrpm = (uint32_t)packet->carPlayer.engine.rpmMax;
    simdata->idlerpm = (uint32_t)packet->carPlayer.engine.rpmIdle;

    // Gear mapping: Wreckfest uses 0=R, 1=N, 2=1st... which matches SimAPI
    simdata->gear = (uint32_t)packet->carPlayer.driveline.gear;
    simdata->maxgears = (uint32_t)packet->carPlayer.driveline.gearMax;

    // Set gear character representation
    if (simdata->gear == 0) {
        strcpy(simdata->gearc, "R");
    } else if (simdata->gear == 1) {
        strcpy(simdata->gearc, "N");
    } else {
        sprintf(simdata->gearc, "%d", simdata->gear - 1);
    }

    // Velocity: m/s to km/h
    simdata->velocity = (uint32_t)(packet->carPlayer.driveline.speed * 3.6f);

    // Inputs (already 0-1 range)
    simdata->gas = (double)packet->carPlayer.input.throttle;
    simdata->brake = (double)packet->carPlayer.input.brake;
    simdata->clutch = (double)packet->carPlayer.input.clutch;
    simdata->steer = (double)packet->carPlayer.input.steering;
    simdata->handbrake = (double)packet->carPlayer.input.handbrake;

    // Position from Motion::Orientation
    simdata->worldposx = (double)packet->carPlayer.orientation.positionX;
    simdata->worldposy = (double)packet->carPlayer.orientation.positionY;
    simdata->worldposz = (double)packet->carPlayer.orientation.positionZ;

    // Local velocity (m/s)
    simdata->Xvelocity = (double)packet->carPlayer.velocity.velocityLocalX;
    simdata->Yvelocity = (double)packet->carPlayer.velocity.velocityLocalY;
    simdata->Zvelocity = (double)packet->carPlayer.velocity.velocityLocalZ;

    // Angular velocity (rad/s) - store in world velocity fields
    simdata->worldXvelocity = (double)packet->carPlayer.velocity.angularVelocityX;
    simdata->worldYvelocity = (double)packet->carPlayer.velocity.angularVelocityY;
    simdata->worldZvelocity = (double)packet->carPlayer.velocity.angularVelocityZ;

    // Convert quaternion to Euler angles (degrees)
    quaternion_to_euler(
        packet->carPlayer.orientation.orientationQuaternionX,
        packet->carPlayer.orientation.orientationQuaternionY,
        packet->carPlayer.orientation.orientationQuaternionZ,
        packet->carPlayer.orientation.orientationQuaternionW,
        &simdata->heading,
        &simdata->pitch,
        &simdata->roll
    );

    // Tire data (4 tires: FL=0, FR=1, RL=2, RR=3 in Wreckfest)
    // Note: Wreckfest uses different order than some sims
    for (int i = 0; i < 4; i++) {
        simdata->tyreRPS[i] = (double)packet->carPlayer.tires[i].rps;

        // Temperature - convert Kelvin to Celsius if available
        float temp_tread = packet->carPlayer.tires[i].temperatureTread;
        if (temp_tread > 0.0f) {
            simdata->tyretemp[i] = (double)(temp_tread - 273.15);
        } else {
            simdata->tyretemp[i] = 0.0;
        }

        // Tire pressure - not available in current Pino implementation
        simdata->tyrepressure[i] = 0.0;

        // Suspension position and velocity
        simdata->suspension[i] = (double)packet->carPlayer.tires[i].suspensionDisplacement;
        simdata->suspvelocity[i] = (double)packet->carPlayer.tires[i].suspensionVelocity;
    }

    // Session info
    strncpy(simdata->track, packet->session.trackName, 127);
    simdata->track[127] = '\0';
    simdata->numlaps = (uint32_t)packet->session.laps;

    // Player info from participant data
    strncpy(simdata->driver, packet->participantPlayerInfo.playerName, 127);
    simdata->driver[127] = '\0';
    strncpy(simdata->car, packet->participantPlayerInfo.carName, 127);
    simdata->car[127] = '\0';

    // Lap and position from leaderboard
    simdata->lap = (uint32_t)packet->participantPlayerLeaderboard.lapCurrent;
    simdata->position = (uint32_t)packet->participantPlayerLeaderboard.position;
    simdata->playerlaps = simdata->lap > 0 ? simdata->lap - 1 : 0;

    // Timing from participant timing (convert milliseconds to seconds)
    simdata->currentlapinseconds = packet->participantPlayerTiming.lapTimeCurrent / 1000;
    simdata->lastlapinseconds = packet->participantPlayerTiming.lapTimeLast / 1000;

    // Convert milliseconds to LapTime struct
    uint32_t current_ms = packet->participantPlayerTiming.lapTimeCurrent;
    simdata->currentlap.hours = current_ms / 3600000;
    simdata->currentlap.minutes = (current_ms % 3600000) / 60000;
    simdata->currentlap.seconds = (current_ms % 60000) / 1000;
    simdata->currentlap.fraction = current_ms % 1000;

    uint32_t last_ms = packet->participantPlayerTiming.lapTimeLast;
    simdata->lastlap.hours = last_ms / 3600000;
    simdata->lastlap.minutes = (last_ms % 3600000) / 60000;
    simdata->lastlap.seconds = (last_ms % 60000) / 1000;
    simdata->lastlap.fraction = last_ms % 1000;

    uint32_t best_ms = packet->participantPlayerTiming.lapTimeBest;
    simdata->bestlap.hours = best_ms / 3600000;
    simdata->bestlap.minutes = (best_ms % 3600000) / 60000;
    simdata->bestlap.seconds = (best_ms % 60000) / 1000;
    simdata->bestlap.fraction = best_ms % 1000;

    // Lap progress along track route (0-1)
    simdata->playerspline = (double)packet->participantPlayerTiming.lapProgress;
    simdata->trackdistancearound = (double)packet->session.trackLength;

    // Game status
    if (packet->header.statusFlags & WF2_GAME_STATUS_IN_RACE) {
        simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;
    } else {
        simdata->simstatus = SIMAPI_STATUS_MENU;
    }

    // Engine temps (Kelvin to Celsius)
    simdata->airtemp = (double)(packet->carPlayer.engine.tempBlock - 273.15);
    simdata->tracktemp = (double)(packet->carPlayer.engine.tempWater - 273.15);

    // Engine power and torque
    simdata->turboboost = (double)packet->carPlayer.engine.pressureManifold;

    // Additional car data
    simdata->abs = (double)packet->carPlayer.assists.levelAbs;
}

/**
 * @brief Convert quaternion to Euler angles (heading, pitch, roll) in degrees
 *
 * Uses ZYX rotation order (yaw-pitch-roll)
 * Wreckfest 2 uses left-handed coordinate system: X=right, Y=up, Z=forward
 */
static void quaternion_to_euler(float qx, float qy, float qz, float qw,
                                 double* heading, double* pitch, double* roll)
{
    // Heading (Yaw) - rotation around Y axis
    double siny_cosp = 2.0 * (qw * qy + qx * qz);
    double cosy_cosp = 1.0 - 2.0 * (qy * qy + qz * qz);
    *heading = atan2(siny_cosp, cosy_cosp) * (180.0 / M_PI);

    // Pitch - rotation around X axis
    double sinp = 2.0 * (qw * qx - qz * qy);
    if (fabs(sinp) >= 1.0) {
        *pitch = copysign(90.0, sinp); // Use 90 degrees if out of range
    } else {
        *pitch = asin(sinp) * (180.0 / M_PI);
    }

    // Roll - rotation around Z axis
    double sinr_cosp = 2.0 * (qw * qz + qx * qy);
    double cosr_cosp = 1.0 - 2.0 * (qx * qx + qy * qy);
    *roll = atan2(sinr_cosp, cosr_cosp) * (180.0 / M_PI);
}
