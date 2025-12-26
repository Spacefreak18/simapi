#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../simapi/simdata.h"

volatile int running = 1;

void signal_handler(int signum) { running = 0; }

void print_lap_time(const char *label, LapTime lt) {
  printf("%s: %02u:%02u:%02u.%03u\n", label, lt.hours, lt.minutes, lt.seconds,
         lt.fraction);
}

int main(int argc, char *argv[]) {
  int fd;
  SimData *simdata;

  // Set up signal handler for clean exit
  signal(SIGINT, signal_handler);

  printf("Opening SIMAPI.DAT shared memory...\n");

  // Open the shared memory file
  fd = open("/dev/shm/SIMAPI.DAT", O_RDONLY);
  if (fd == -1) {
    perror("Failed to open shared memory");
    return 1;
  }

  // Map the shared memory
  simdata =
      (SimData *)mmap(NULL, sizeof(SimData), PROT_READ, MAP_SHARED, fd, 0);
  if (simdata == MAP_FAILED) {
    perror("Failed to map shared memory");
    close(fd);
    return 1;
  }

  printf("Connected to telemetry data. Press Ctrl+C to exit.\n");
  printf("Waiting for DiRT Rally 2.0 to start sending data...\n\n");

  while (running) {
    if (simdata->simon == 1 && simdata->simstatus >= SIMAPI_STATUS_MENU) {
      printf("\033[H\033[J"); // Clear screen and move cursor to home
      printf("--- DiRT Rally 2.0 Telemetry (simapi) ---\n\n");

      printf("Vehicle:\n");
      printf("  Car:    %s\n", simdata->car);
      printf("  RPM:    %d / %d (Idle: %d)\n", simdata->rpms, simdata->maxrpm,
             simdata->idlerpm);
      printf("  Speed:  %d km/h\n", simdata->velocity);
      printf("  Gear:   %s (Max: %d)\n", simdata->gearc, simdata->maxgears);
      printf("  Fuel:   %.2f / %.2f L\n", simdata->fuel, simdata->fuelcapacity);

      printf("\nInputs:\n");
      printf("  Gas:    %.2f | Brake:  %.2f\n", simdata->gas, simdata->brake);
      printf("  Clutch: %.2f | Steer:  %.3f\n", simdata->clutch,
             simdata->steer);

      printf("\nOrientation (Euler):\n");
      printf("  Roll:   %6.1f deg\n", simdata->roll);
      printf("  Pitch:  %6.1f deg\n", simdata->pitch);
      printf("  Yaw:    %6.1f deg (Heading)\n", simdata->heading);

      printf("\nForces (G):\n");
      printf("  Lat:    %6.2f | Long:   %6.2f | Vert:   %6.2f\n",
             simdata->Xvelocity, simdata->Yvelocity, simdata->Zvelocity);

      printf("\nWheels:\n");
      printf("  Speeds (m/s): RL: %5.1f | RR: %5.1f | FL: %5.1f | FR: %5.1f\n",
             simdata->tyreRPS[0], simdata->tyreRPS[1], simdata->tyreRPS[2],
             simdata->tyreRPS[3]);
      printf("  T-Press (PSI): RL: %5.2f | RR: %5.2f | FL: %5.2f | FR: %5.2f\n",
             simdata->tyrepressure[0], simdata->tyrepressure[1],
             simdata->tyrepressure[2], simdata->tyrepressure[3]);
      printf("  B-Temp (C):    RL: %5.1f | RR: %5.1f | FL: %5.1f | FR: %5.1f\n",
             simdata->braketemp[0], simdata->braketemp[1],
             simdata->braketemp[2], simdata->braketemp[3]);

      printf("\nSuspension:\n");
      printf("  Pos: RL: %6.3f | RR: %6.3f | FL: %6.3f | FR: %6.3f\n",
             simdata->suspension[0], simdata->suspension[1],
             simdata->suspension[2], simdata->suspension[3]);
      printf("  Vel: RL: %6.3f | RR: %6.3f | FL: %6.3f | FR: %6.3f\n",
             simdata->suspvelocity[0], simdata->suspvelocity[1],
             simdata->suspvelocity[2], simdata->suspvelocity[3]);

      printf("\nSession & Race:\n");
      printf("  Status:   %d\n", simdata->simstatus);
      printf("  Lap:      %d / %d (Completed: %d)\n", simdata->lap,
             simdata->numlaps, simdata->playerlaps);
      printf("  Position: %d\n", simdata->position);
      printf("  Progress: %.3f\n", simdata->playerspline);
      printf("  Distance: %.1f m (of %.1f m)\n", simdata->distance,
             simdata->trackdistancearound);
      printf("  Sector:   %d\n", simdata->sectorindex + 1);
      printf("  Sec 1:    %7.3f s | Sec 2:    %7.3f s\n", simdata->sector1time,
             simdata->sector2time);
      print_lap_time("  Current", simdata->currentlap);
      print_lap_time("  Last   ", simdata->lastlap);

      printf("\nDebug (World Pos):\n");
      printf("  X: %.2f | Y: %.2f | Z: %.2f\n", simdata->worldposx,
             simdata->worldposy, simdata->worldposz);

      fflush(stdout);
    } else {
      printf("\rWaiting for game data... (Status: %d, SimOn: %d)          ",
             simdata->simstatus, simdata->simon);
      fflush(stdout);
    }

    usleep(100000); // 100ms delay
  }

  printf("\n\nExiting...\n");

  // Clean up
  munmap(simdata, sizeof(SimData));
  close(fd);

  return 0;
}
