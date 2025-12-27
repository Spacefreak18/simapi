#ifndef _SIMMAPPER_H
#define _SIMMAPPER_H

// #include "ac.h"
// #include "rf2.h"
// #include "pcars2.h"
// #include "scs2.h"
#include <signal.h>

#include "simapi.h"
#include "simdata.h"

/**
 * @brief Information about the currently detected simulator.
 */
typedef struct {
  bool isSimOn;    /**< True if a simulator is running and detected. */
  bool SimUsesUDP; /**< True if the simulator uses UDP telemetry. */
  bool SimSupportsBasicTelemetry;    /**< Supports basic telemetry (speed, rpm,
                                        etc). */
  bool SimSupportsTyreEffects;       /**< Supports tyre-specific effects (temp,
                                        pressure). */
  bool SimSupportsRealtimeTelemetry; /**< Supports high-frequency realtime
                                        telemetry. */
  bool SimSupportsAdvancedUI; /**< Supports advanced UI features (leaderboards,
                                 etc). */
  SimulatorAPI mapapi;        /**< The API being used for mapping. */
  SimulatorAPI simulatorapi;  /**< The detected simulator API. */
  SimulatorEXE simulatorexe;  /**< The detected simulator executable. */
  pid_t pid;                  /**< Process ID of the detected simulator. */
} SimInfo;

// typedef struct
//{
//     void* addr;
//     int fd;
//     union
//     {
//         ACMap* ac;
//         RF2Map* rf2;
//         PCars2Map* pcars2;
//         SCS2Map* scs2;
//     } d;
// }
// SimMap;

// struct _simmap;
typedef struct _simmap SimMap;

/**
 * @brief Compatibility structure for legacy shared memory mapping.
 */
typedef struct {
  void *pcars2_addr;     /**< Memory address for PCars2 shared memory. */
  int pcars2_fd;         /**< File descriptor for PCars2 shared memory. */
  void *acphysics_addr;  /**< Memory address for AC physics. */
  int acphysics_fd;      /**< File descriptor for AC physics. */
  void *acgraphics_addr; /**< Memory address for AC graphics. */
  int acgraphics_fd;     /**< File descriptor for AC graphics. */
  void *acstatic_addr;   /**< Memory address for AC static data. */
  int acstatic_fd;       /**< File descriptor for AC static data. */
  void *accrew_addr;     /**< Memory address for AC CrewChief. */
  int accrew_fd;         /**< File descriptor for AC CrewChief. */
} SimCompatMap;

/**
 * @brief Checks if a simulator requires a bridge for telemetry.
 * @param s The simulator executable enum.
 * @return True if a bridge is required.
 */
bool does_sim_need_bridge(SimulatorEXE s);

/**
 * @brief Detects if a simulator process is currently running.
 * @param si Pointer to SimInfo to populate with PID.
 * @return The detected SimulatorEXE enum.
 */
SimulatorEXE getSimExe(SimInfo *si);

/**
 * @brief Main entry point to detect and initialize the active simulator.
 * @param simdata Pointer to the SimData structure to populate.
 * @param simmap Pointer to the SimMap structure for memory mapping.
 * @param force_udp Force using UDP telemetry if available.
 * @param setup_udp Function pointer for UDP socket setup.
 * @param simd True if running as a daemon.
 * @return SimInfo populated with detection results.
 */
SimInfo getSim(SimData *simdata, SimMap *simmap, bool force_udp,
               int (*setup_udp)(int), bool simd);

/**
 * @brief Initializes memory mapping for a specific simulator API.
 * @param simdata Pointer to SimData.
 * @param simmap Pointer to SimMap.
 * @param simulator The simulator API to initialize.
 * @return 0 on success, error code otherwise.
 */
int siminit(SimData *simdata, SimMap *simmap, SimulatorAPI simulator);

/**
 * @brief Initializes UDP-based telemetry mapping.
 */
int siminitudp(SimData *simdata, SimMap *simmap, SimulatorAPI simulator);

/**
 * @brief Maps native simulator data into the universal SimData structure.
 * @param simdata target data structure.
 * @param simmap source mapping handle.
 * @param simmap2 secondary mapping handle (optional).
 * @param simulator API type.
 * @param udp true if source is UDP.
 * @param base base address for some mapping types.
 * @return 0 on success.
 */
int simdatamap(SimData *simdata, SimMap *simmap, SimMap *simmap2,
               SimulatorAPI simulator, bool udp, char *base);

/**
 * @brief Frees resources associated with a simulator mapping.
 */
int simfree(SimData *simdata, SimMap *simmap, SimulatorAPI simulator);

/**
 * @brief Converts a game string to its SimulatorEXE enum value.
 * @param game The game identifier string (e.g., "ac", "rf2").
 * @return The corresponding SimulatorEXE enum value.
 */
int simapi_strtogame(const char *game);

/**
 * @brief Converts a SimulatorEXE enum value to its short string identifier.
 * @param sim The SimulatorEXE enum.
 * @return String identifier (e.g., "ac", "rf2").
 */
char *simapi_gametostr(SimulatorEXE sim);

/**
 * @brief Converts a SimulatorEXE enum value to its full descriptive name.
 * @param sim The SimulatorEXE enum.
 * @return Descriptive string (e.g., "Assetto Corsa").
 */
char *simapi_gametofullstr(SimulatorEXE sim);

/**
 * @brief Allocates and initializes a new SimMap structure.
 * @return Pointer to the new SimMap.
 */
SimMap *createSimMap(void);

/**
 * @brief Returns the base memory address of a SimMap.
 * @param simmap Pointer to the SimMap.
 * @return Base memory pointer.
 */
void *getSimMapPtr(SimMap *simmap);

/**
 * @brief Force-maps internal SimData into the shared SimMap memory.
 * @param simmap Destination mapping.
 * @param simdata Source data.
 * @return 0 on success.
 */
int simdmap(SimMap *simmap, SimData *simdata);

/**
 * @brief Opens the universal shared memory mapping.
 */
int opensimmap(SimMap *simmap);

/**
 * @brief Closes a SimMap and releases associated memory.
 */
int freesimmap(SimMap *simmap, bool issimd);

/**
 * @brief Opens the legacy compatibility shared memory mappings.
 */
int opensimcompatmap(SimCompatMap *compatmap);

/**
 * @brief Closes legacy compatibility mappings.
 */
int freesimcompatmap(SimCompatMap *compatmap);

/**
 * @brief Calculates proximity data for nearby cars.
 * @param simdata Pointer to SimData.
 * @param cars Number of cars to process.
 * @param lr_flip Left/Right flip flag (e.g., -1 or 1).
 */
void SetProximityData(SimData *simdata, int cars, int8_t lr_flip);

/* Simulator-specific modular mapping functions */

int init_scs2_map(SimData *simdata, SimMap *simmap);
int free_scs2_map(SimData *simdata, SimMap *simmap);

void map_assetto_corsa_data(SimData *simdata, SimMap *simmap,
                            SimulatorEXE simexe);
int init_ac_map(SimData *simdata, SimMap *simmap);
int free_ac_map(SimData *simdata, SimMap *simmap);
void map_rfactor2_data(SimData *simdata, SimMap *simmap);
int init_rf2_map(SimData *simdata, SimMap *simmap);
int free_rf2_map(SimData *simdata, SimMap *simmap);
void map_project_cars2_data(SimData *simdata, SimMap *simmap, bool udp,
                            char *base);
int init_pcars2_map(SimData *simdata, SimMap *simmap);
int free_pcars2_map(SimData *simdata, SimMap *simmap);
void map_trucks_data(SimData *simdata, SimMap *simmap);
void map_outgauge_outsim_data(SimData *simdata, SimMap *simmap,
                              SimulatorEXE simexe, char *base);

#endif
