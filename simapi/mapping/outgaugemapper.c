#include <stddef.h>
#include <math.h>
#include <stdio.h>

#include "../simmap.h"
#include "../simdata.h"
#include "../simapi.h"
#include "../simmapper.h"
#include "../outgauge.h"

#include "../../include/outgauge.h"


static int froundint(float f)
{
    return trunc(nearbyint(f));
}

void map_outgauge_outsim_data(SimData* simdata, SimMap* simmap, SimulatorEXE simexe, char* base)
{
    if(base != NULL)
    {
        char* a = base;
        int id = 0;
        
        id = *(uint8_t*) (char*) (a + offsetof(struct outgauge, id));
        if(id == 2 || simexe == SIMULATOREXE_BEAMNG)
        {
            simdata->gear = *(uint8_t*) (char*) (a + offsetof(struct outgauge, gear));
            //int time = *(uint64_t*) (char*) (a + offsetof(struct outgauge, time));
            --simdata->gear;
            simdata->rpms = froundint( *(float*) (char*) (a + offsetof(struct outgauge, rpm)) );
            simdata->velocity = froundint( (*(float*) (char*) (a + offsetof(struct outgauge, speed))) * 3.6 );
            simdata->turboboost = *(float*) (char*) (a + offsetof(struct outgauge, speed));
            simdata->fuel = *(float*) (char*) (a + offsetof(struct outgauge, fuel));
            simdata->gas = *(float*) (char*) (a + offsetof(struct outgauge, throttle));
            simdata->brake = *(float*) (char*) (a + offsetof(struct outgauge, brake));

            simdata->altitude = 1;
            return;
        }
        id = 0;
      
        id = *(uint8_t*) (char*) (a + offsetof(struct outsim, ID));
        if(id == 1)
        {
            float pos = *(float*) (char*) (a + offsetof(struct outsim, CurrentLapDist));
            float dist = *(float*) (char*) (a + offsetof(struct outsim, IndexedDistance));
            //fprintf(stderr, "pos x: %f %f\n", pos, dist);
            return;
        }
    }
}
