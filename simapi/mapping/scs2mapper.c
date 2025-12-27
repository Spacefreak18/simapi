#include <fcntl.h>
#include <math.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../scs2.h"
#include "../simapi.h"
#include "../simdata.h"
#include "../simmap.h"
#include "../simmapper.h"

#include "../../include/scs2data.h"

static int droundint(double d) { return trunc(nearbyint(d)); }

void map_trucks_data(SimData *simdata, SimMap *simmap) {

  char *a;
  char *b;
  char *c;
  char *d;

  a = simmap->scs2.telemetry_map_addr;

  simdata->simstatus = 2;
  simdata->car[0] = 'd';
  simdata->car[1] = 'e';
  simdata->car[2] = 'f';
  simdata->car[3] = 'a';
  simdata->car[4] = 'u';
  simdata->car[5] = 'l';
  simdata->car[6] = 't';
  simdata->car[7] = '\0';

  simdata->velocity = droundint(
      3.6 * (*(float *)(char *)(a + offsetof(struct scs2TelemetryMap_s,
                                             truck_f.speed))));
  simdata->rpms =
      droundint(*(float *)(char *)(a + offsetof(struct scs2TelemetryMap_s,
                                                truck_f.engineRpm)));
  simdata->brake =
      droundint(*(float *)(char *)(a + offsetof(struct scs2TelemetryMap_s,
                                                truck_f.userBrake)));
  simdata->gas =
      droundint(*(float *)(char *)(a + offsetof(struct scs2TelemetryMap_s,
                                                truck_f.userThrottle)));
  simdata->gear = *(uint32_t *)(char *)(a + offsetof(struct scs2TelemetryMap_s,
                                                     truck_i.gear));
  simdata->tyreRPS[0] =
      *(float *)(char *)(a + offsetof(struct scs2TelemetryMap_s,
                                      truck_f.truck_wheelVelocity));
  simdata->tyreRPS[1] =
      *(float *)(char *)(a +
                         offsetof(struct scs2TelemetryMap_s,
                                  truck_f.truck_wheelVelocity) +
                         (sizeof(float) * 1));
  simdata->tyreRPS[2] =
      *(float *)(char *)(a +
                         offsetof(struct scs2TelemetryMap_s,
                                  truck_f.truck_wheelVelocity) +
                         (sizeof(float) * 2));
  simdata->tyreRPS[3] =
      *(float *)(char *)(a +
                         offsetof(struct scs2TelemetryMap_s,
                                  truck_f.truck_wheelVelocity) +
                         (sizeof(float) * 3));
  simdata->maxrpm =
      droundint(*(float *)(char *)(a + offsetof(struct scs2TelemetryMap_s,
                                                config_f.engineRpmMax)));

  if (simdata->gear > 0) {
    simdata->gearc[0] = (simdata->gear / 2) + 48;
    if (simdata->gear % 2 == 0) {
      simdata->gearc[1] = 72;
    } else {
      simdata->gearc[1] = 76;
      simdata->gearc[0]++;
    }
    simdata->gearc[2] = 0;
  } else {
    if (simdata->gear < 0) {
      simdata->gearc[0] = 82;
    }
    if (simdata->gear == 0) {
      simdata->gearc[0] = 78;
    }
    simdata->gearc[1] = 0;
  }
  ++simdata->gear;

  simdata->altitude = 1;
}
