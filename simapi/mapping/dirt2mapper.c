#include <stddef.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../simdata.h"
#include "../simmapper.h"
#include "../simmap.h"
#include "../../include/dirt2data.h"

#define DR2_RPM_SCALE 10.0f

static int froundint(float f)
{
    return (int)trunc(nearbyint(f));
}

void map_dirt_rally_2_data(SimData* simdata, SimMap* simmap, char* base)
{
    if (base != NULL)
    {
        struct dirt2_udp_packet* packet = (struct dirt2_udp_packet*)base;
        
        if (simmap != NULL)
        {
            memcpy(&simmap->dirt2.dirt2_telemetry, packet, sizeof(struct dirt2_udp_packet));
            simmap->dirt2.has_telemetry = true;
        }

        simdata->rpms = froundint(packet->fields.engineRPM * DR2_RPM_SCALE);
        simdata->velocity = froundint(packet->fields.speed * 3.6f); // m/s to km/h
        
        // Gear mapping: -1=R, 0=N, 1=1st, 2=2nd...
        float raw_gear = packet->fields.gear;
        int gear_int = (int)raw_gear;
        
        if (gear_int < 0) {
            simdata->gear = 0; // Reverse
        } else {
            simdata->gear = (uint32_t)gear_int + 1;
        }
        
        simdata->maxrpm = froundint(packet->fields.maxRPM * DR2_RPM_SCALE);

        if (simdata->gear == 0) {
            strcpy(simdata->gearc, "R");
        } else if (simdata->gear == 1) {
            strcpy(simdata->gearc, "N");
        } else {
            sprintf(simdata->gearc, "%d", simdata->gear - 1);
        }
        
        // Input mapping
        simdata->gas = (double)packet->fields.throttle;
        simdata->steer = (double)packet->fields.steering;
        simdata->brake = (double)packet->fields.brake;
        simdata->clutch = (double)packet->fields.clutch;

        // Velocities
        simdata->worldXvelocity = (double)packet->fields.velX;
        simdata->worldYvelocity = (double)packet->fields.velY;
        simdata->worldZvelocity = (double)packet->fields.velZ;

        // Orientation Vectors (DiRT Rally 2.0 extradata=3)
        // 11-13: Right Vector (rx, ry, rz)
        // 14-16: Forward Vector (fx, fy, fz)
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

        // G-Forces / Accelerations (Vert and Lon are provided by the game, Lat must be calculated)
        simdata->Zvelocity = (double)packet->fields.gForceVert;
        simdata->Yvelocity = (double)packet->fields.gForceLon;
        
        // Calculate Lateral G-force (Xvelocity) by transforming world acceleration to local space
        if (simmap != NULL && simmap->dirt2.has_last_vel) {
            float dt = packet->fields.runTime - simmap->dirt2.last_runTime;
            if (dt > 0.0001f) {
                // World acceleration (m/s^2)
                float ax = (packet->fields.velX - simmap->dirt2.last_velX) / dt;
                float ay = (packet->fields.velY - simmap->dirt2.last_velY) / dt;
                float az = (packet->fields.velZ - simmap->dirt2.last_velZ) / dt;
                
                // Add gravity (assuming Y is world-up)
                const float gravity = 9.80665f;
                ay += gravity; 
                
                // Lateral G is the projection of (world_accel + gravity) onto the car's local Right vector
                float lat_accel = ax * rx + ay * ry + az * rz;
                simdata->Xvelocity = (double)(lat_accel / gravity);
            }
        }
        
        if (simmap != NULL) {
            simmap->dirt2.last_velX = packet->fields.velX;
            simmap->dirt2.last_velY = packet->fields.velY;
            simmap->dirt2.last_velZ = packet->fields.velZ;
            simmap->dirt2.last_runTime = packet->fields.runTime;
            simmap->dirt2.has_last_vel = true;
        }

        // Store vectors for debugging in view_telemetry
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

        // Wheel Speeds
        simdata->tyreRPS[0] = (double)packet->fields.wheelSpeedRL;
        simdata->tyreRPS[1] = (double)packet->fields.wheelSpeedRR;
        simdata->tyreRPS[2] = (double)packet->fields.wheelSpeedFL;
        simdata->tyreRPS[3] = (double)packet->fields.wheelSpeedFR;

        // Status
        if (packet->fields.runTime > 0) {
            simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;
        } else {
            simdata->simstatus = SIMAPI_STATUS_MENU;
        }

        simdata->lap = (uint32_t)packet->fields.lap;
        simdata->trackdistancearound = (double)packet->fields.trackLength;
        simdata->playerspline = (double)packet->fields.progress;

        // Position
        simdata->worldposx = (double)packet->fields.posX;
        simdata->worldposy = (double)packet->fields.posY;
        simdata->worldposz = (double)packet->fields.posZ;

        // Fuel
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

        // Suspensions
        simdata->suspension[0] = (double)packet->fields.suspRL;
        simdata->suspension[1] = (double)packet->fields.suspRR;
        simdata->suspension[2] = (double)packet->fields.suspFL;
        simdata->suspension[3] = (double)packet->fields.suspFR;

        strncpy(simdata->car, "DR2", 4);
    }
}

