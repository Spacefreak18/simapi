#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include "../simapi/simdata.h"

volatile int running = 1;

void signal_handler(int signum) {
    running = 0;
}

int main(int argc, char* argv[]) {
    int fd;
    SimData* simdata;

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
    simdata = (SimData*) mmap(NULL, sizeof(SimData), PROT_READ, MAP_SHARED, fd, 0);
    if (simdata == MAP_FAILED) {
        perror("Failed to map shared memory");
        close(fd);
        return 1;
    }

    printf("Connected to telemetry data. Press Ctrl+C to exit.\n");
    printf("Waiting for DiRT Rally 2.0 to start sending data...\n\n");

    while (running) {
        // Check if we have valid data
        if (simdata->simon == 1 && simdata->simstatus >= SIMAPI_STATUS_MENU) {
            printf("\033[H\033[J"); // Clear screen and move cursor to home
            printf("--- DiRT Rally 2.0 Telemetry (simapi) ---\n\n");

            printf("RPM:   %d\n", simdata->rpms);
            printf("Speed: %d km/h\n", simdata->velocity);
            printf("Gear:  %s\n", simdata->gearc);

            printf("\nInputs:\n");
            printf("  Gas:    %.2f\n", simdata->gas);
            printf("  Brake:  %.2f\n", simdata->brake);
            printf("  Clutch: %.2f\n", simdata->clutch);
            printf("  Steer:  %.3f\n", simdata->steer);

            printf("\nOrientation:\n");
            printf("  Roll:   %6.1f deg\n", simdata->roll);
            printf("  Pitch:  %6.1f deg\n", simdata->pitch);
            printf("  Yaw:    %6.1f deg (Heading)\n", simdata->heading);

            printf("\nForces:\n");
            printf("  G-Lat:  %6.2f\n", simdata->Xvelocity);
            printf("  G-Long: %6.2f\n", simdata->Yvelocity);
            printf("  G-Vert: %6.2f\n", simdata->Zvelocity);

            printf("\nWheels (m/s):\n");
            printf("  RL: %.1f | RR: %.1f\n", simdata->tyreRPS[0], simdata->tyreRPS[1]);
            printf("  FL: %.1f | FR: %.1f\n", simdata->tyreRPS[2], simdata->tyreRPS[3]);

            if (simdata->simstatus >= SIMAPI_STATUS_ACTIVEPLAY) {
                printf("\nRace Status:\n");
                printf("  Lap:      %d\n", simdata->lap);
                printf("  Progress: %.3f\n", simdata->playerspline);
            }

            printf("\nDebug (World Pos):\n");
            printf("  X: %.2f | Y: %.2f | Z: %.2f\n", 
                   simdata->worldposx, simdata->worldposy, simdata->worldposz);

            printf("\nDebug (Vectors):\n");
            printf("  Forward: [%6.3f, %6.3f, %6.3f]\n", 
                   simdata->tyrecontact2[0], simdata->tyrecontact2[1], simdata->tyrecontact2[2]);
            printf("  Up:      [%6.3f, %6.3f, %6.3f]\n", 
                   simdata->tyrecontact1[0], simdata->tyrecontact1[1], simdata->tyrecontact1[2]);
            printf("  Right:   [%6.3f, %6.3f, %6.3f]\n", 
                   simdata->tyrecontact0[0], simdata->tyrecontact0[1], simdata->tyrecontact0[2]);

            fflush(stdout);
        } else {
            printf("\rWaiting for game data... (Status: %d, SimOn: %d)",
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
