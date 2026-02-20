#include <stddef.h>
#include <math.h>
#include <string.h>

#include "../simmap.h"
#include "../simdata.h"
#include "../simmapper.h"
#include "../../include/r3e.h"


void map_r3e_data(SimData* simdata, SimMap* simmap)
{

    char* a;

    a = simmap->r3e.telemetry_map_addr;
    /* ---------------- Simulation State ---------------- */

    int32_t paused = *(int32_t*)(a + offsetof(struct r3e_share, game_paused));
    int32_t menus  = *(int32_t*)(a + offsetof(struct r3e_share, game_in_menus));

    if(paused || menus)
    {
        simdata->simstatus = SIMAPI_STATUS_MENU;
    }
    else
    {
        simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;
    }

    /* ---------------- Speed ---------------- */

    float speed_ms = *(float*)(a + offsetof(struct r3e_share, car_speed));
    simdata->velocity = (uint32_t)(speed_ms * 3.6f);

    /* ---------------- Engine ---------------- */

    float rps = *(float*)(a + offsetof(struct r3e_share, engine_rps));
    float maxrps = *(float*)(a + offsetof(struct r3e_share, max_engine_rps));

    simdata->rpms = rps * 60.0f;
    simdata->maxrpm = maxrps * 60.0f;

    int32_t gear = *(int32_t*)(a + offsetof(struct r3e_share, gear));
    simdata->gear = gear + 1;

    simdata->maxgears = *(int32_t*)(a + offsetof(struct r3e_share, num_gears));

    /* ---------------- Inputs ---------------- */

    simdata->gas   = *(float*)(a + offsetof(struct r3e_share, throttle));
    simdata->brake = *(float*)(a + offsetof(struct r3e_share, brake));
    simdata->clutch = *(float*)(a + offsetof(struct r3e_share, clutch));
    simdata->steer = *(float*)(a + offsetof(struct r3e_share, steer_input_raw));

    /* ---------------- Fuel ---------------- */

    simdata->fuel =
        *(float*)(a + offsetof(struct r3e_share, fuel_left));

    simdata->fuelcapacity =
        *(float*)(a + offsetof(struct r3e_share, fuel_capacity));

    simdata->turboboost =
        *(float*)(a + offsetof(struct r3e_share, turbo_pressure));

    /* ---------------- Wheel Angular Velocity ---------------- */

    for(int i = 0; i < 4; i++)
    {
        simdata->tyreRPS[i] =
            *(float*)(a + offsetof(struct r3e_share, tire_rps) + sizeof(float) * i);
    }

    /* ---------------- Suspension ---------------- */

    for(int i = 0; i < 4; i++)
    {
        simdata->suspension[i] =
            *(float*)(a +
            offsetof(struct r3e_share, player) +
            offsetof(r3e_playerdata, suspension_deflection) +
            sizeof(float) * i);

        simdata->suspvelocity[i] =
            *(float*)(a +
            offsetof(struct r3e_share, player) +
            offsetof(r3e_playerdata, suspension_velocity) +
            sizeof(float) * i);
    }

    /* ---------------- Flags ---------------- */

    uint8_t green =
        *(uint8_t*)(a + offsetof(struct r3e_share, flags) +
        offsetof(r3e_flags, green));

    uint8_t yellow =
        *(uint8_t*)(a + offsetof(struct r3e_share, flags) +
        offsetof(r3e_flags, yellow));

    uint8_t blue =
        *(uint8_t*)(a + offsetof(struct r3e_share, flags) +
        offsetof(r3e_flags, blue));

    uint8_t black =
        *(uint8_t*)(a + offsetof(struct r3e_share, flags) +
        offsetof(r3e_flags, black));

    uint8_t white =
        *(uint8_t*)(a + offsetof(struct r3e_share, flags) +
        offsetof(r3e_flags, white));

    uint8_t chequered =
        *(uint8_t*)(a + offsetof(struct r3e_share, flags) +
        offsetof(r3e_flags, checkered));

    if(chequered)
        simdata->courseflag = SIMAPI_FLAG_CHEQUERED;
    else if(yellow)
        simdata->courseflag = SIMAPI_FLAG_YELLOW;
    else if(blue)
        simdata->courseflag = SIMAPI_FLAG_BLUE;
    else if(black)
        simdata->courseflag = SIMAPI_FLAG_BLACK;
    else if(white)
        simdata->courseflag = SIMAPI_FLAG_WHITE;
    else if(green)
        simdata->courseflag = SIMAPI_FLAG_GREEN;

    simdata->playerflag = simdata->courseflag;

    /* ---------------- Pit ---------------- */

    int32_t in_pitlane =
        *(int32_t*)(a + offsetof(struct r3e_share, in_pitlane));

    int32_t pit_state =
        *(int32_t*)(a + offsetof(struct r3e_share, pit_state));

    simdata->cars[0].inpitlane = (in_pitlane == 1);
    simdata->cars[0].inpit     = (pit_state >= 2);

    /* ---------------- Lap Times ---------------- */

    float lastlap =
        *(float*)(a + offsetof(struct r3e_share, lap_time_previous_self));

    float bestlap =
        *(float*)(a + offsetof(struct r3e_share, lap_time_best_self));

    float curlap =
        *(float*)(a + offsetof(struct r3e_share, lap_time_current_self));

    uint32_t t;

    t = (uint32_t)lastlap;
    simdata->lastlap.hours = t / 3600;
    simdata->lastlap.minutes = (t % 3600) / 60;
    simdata->lastlap.seconds = t % 60;
    simdata->lastlap.fraction = (uint32_t)((lastlap - floor(lastlap)) * 1000.0f);

    t = (uint32_t)bestlap;
    simdata->bestlap.hours = t / 3600;
    simdata->bestlap.minutes = (t % 3600) / 60;
    simdata->bestlap.seconds = t % 60;
    simdata->bestlap.fraction = (uint32_t)((bestlap - floor(bestlap)) * 1000.0f);

    t = (uint32_t)curlap;
    simdata->currentlap.hours = t / 3600;
    simdata->currentlap.minutes = (t % 3600) / 60;
    simdata->currentlap.seconds = t % 60;
    simdata->currentlap.fraction = (uint32_t)((curlap - floor(curlap)) * 1000.0f);

    simdata->currentlapinseconds = (uint32_t)curlap;
    simdata->lastlapinseconds    = (uint32_t)lastlap;

    /* ---------------- Position ---------------- */

    simdata->worldposx =
        *(float*)(a + offsetof(struct r3e_share, player) +
                  offsetof(r3e_playerdata, position) +
                  offsetof(r3e_vec3_f32, x));

    simdata->worldposy =
        *(float*)(a + offsetof(struct r3e_share, player) +
                  offsetof(r3e_playerdata, position) +
                  offsetof(r3e_vec3_f32, y));

    simdata->worldposz =
        *(float*)(a + offsetof(struct r3e_share, player) +
                  offsetof(r3e_playerdata, position) +
                  offsetof(r3e_vec3_f32, z));

    /* ---------------- Lap data ---------------- */

    simdata->lap =
        *(int32_t*)(a + offsetof(struct r3e_share, completed_laps)) + 1;

    simdata->position =
        *(int32_t*)(a + offsetof(struct r3e_share, position));

    simdata->distance =
        *(float*)(a + offsetof(struct r3e_share, lap_distance));

    /* ---------------- Lap Validity ---------------- */

    int32_t curlap_valid =
        *(int32_t*)(a + offsetof(struct r3e_share, lap_valid_state));
    
    simdata->lapisvalid = (curlap_valid != 0);
}
