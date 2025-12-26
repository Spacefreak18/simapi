#ifndef _DIRT2DATA_H
#define _DIRT2DATA_H

#include <stdint.h>

#pragma pack(push, 1)

// DiRT Rally 2.0 UDP packet structure (extradata=3)
// 66 floats (264 bytes)
/**
 * @struct DR2_UDP_ExtraData3
 * @brief DiRT Rally 2.0 UDP packet structure for "extradata=3".
 *
 * This structure contains 66 floating point values (264 bytes total).
 * Data is sent in little-endian format.
 */
typedef struct {
  float runTime;       /**< [UNVERIFIED] Total time the session has been running
                          (seconds) */
  float lapTime;       /**< [UNVERIFIED] Current lap time (seconds) */
  float distance;      /**< [UNVERIFIED] Total distance travelled in the session
                          (meters) */
  float progress;      /**< [UNVERIFIED] Current lap progress (0.0 to 1.0) */
  float posX;          /**< World position X (East/West) */
  float posY;          /**< World position Y (Up/Down) */
  float posZ;          /**< World position Z (North/South) */
  float speed;         /**< Speed (m/s) */
  float velX;          /**< Velocity in world X direction */
  float velY;          /**< Velocity in world Y direction */
  float velZ;          /**< Velocity in world Z direction */
  float rightX;        /**< Local right vector X component */
  float rightY;        /**< Local right vector Y component */
  float rightZ;        /**< Local right vector Z component */
  float forwardX;      /**< Local forward vector X component */
  float forwardY;      /**< Local forward vector Y component */
  float forwardZ;      /**< Local forward vector Z component */
  float suspRL;        /**< [UNVERIFIED] Suspension position Rear Left */
  float suspRR;        /**< [UNVERIFIED] Suspension position Rear Right */
  float suspFL;        /**< [UNVERIFIED] Suspension position Front Left */
  float suspFR;        /**< [UNVERIFIED] Suspension position Front Right */
  float suspVelRL;     /**< [UNVERIFIED] Suspension velocity Rear Left */
  float suspVelRR;     /**< [UNVERIFIED] Suspension velocity Rear Right */
  float suspVelFL;     /**< [UNVERIFIED] Suspension velocity Front Left */
  float suspVelFR;     /**< [UNVERIFIED] Suspension velocity Front Right */
  float wheelSpeedRL;  /**< Wheel speed Rear Left (m/s) */
  float wheelSpeedRR;  /**< Wheel speed Rear Right (m/s) */
  float wheelSpeedFL;  /**< Wheel speed Front Left (m/s) */
  float wheelSpeedFR;  /**< Wheel speed Front Right (m/s) */
  float throttle;      /**< Throttle input (0.0 to 1.0) */
  float steering;      /**< Steering input (-1.0 to 1.0) */
  float brake;         /**< Brake input (0.0 to 1.0) */
  float clutch;        /**< Clutch input (0.0 to 1.0) */
  float gear;          /**< Current gear (0=neutral, 1-10=gears, -1=reverse) */
  float gForceLat;     /**< [UNVERIFIED] Lateral G-Force (Field 34) */
  float gForceLon;     /**< [UNVERIFIED] Longitudinal G-Force (Field 35) */
  float lap;           /**< [UNVERIFIED] Current lap number */
  float engineRPM;     /**< Engine RPM (raw value is RPM / 10) */
  float sliProSupport; /**< [UNVERIFIED] Sli-Pro support data */
  float racePos;       /**< [UNVERIFIED] Current race position */
  float kersLevel;     /**< [UNVERIFIED] KERS level (likely unused in DR2) */
  float kersMaxLevel; /**< [UNVERIFIED] KERS max level (likely unused in DR2) */
  float drs;          /**< [UNVERIFIED] DRS status (likely unused in DR2) */
  float tractionControl; /**< [UNVERIFIED] Traction control setting/activity */
  float antiLockBrakes;  /**< [UNVERIFIED] ABS setting/activity */
  float fuelInTank;      /**< [UNVERIFIED] Current fuel in tank (liters) */
  float fuelCapacity;    /**< [UNVERIFIED] Fuel tank capacity (liters) */
  float inPit;       /**< [UNVERIFIED] Pit lane status (0=on track, 1=in pit) */
  float sector;      /**< [UNVERIFIED] Current sector index */
  float sector1Time; /**< [UNVERIFIED] Time for sector 1 (seconds) */
  float sector2Time; /**< [UNVERIFIED] Time for sector 2 (seconds) */
  float brakesTempRL; /**< [UNVERIFIED] Brake temperature Rear Left (Celsius) */
  float
      brakesTempRR; /**< [UNVERIFIED] Brake temperature Rear Right (Celsius) */
  float
      brakesTempFL; /**< [UNVERIFIED] Brake temperature Front Left (Celsius) */
  float
      brakesTempFR; /**< [UNVERIFIED] Brake temperature Front Right (Celsius) */
  float tyrePressureRL; /**< [UNVERIFIED] Tyre pressure Rear Left (PSI/BAR -
                           verify unit) */
  float tyrePressureRR; /**< [UNVERIFIED] Tyre pressure Rear Right (PSI/BAR -
                           verify unit) */
  float tyrePressureFL; /**< [UNVERIFIED] Tyre pressure Front Left (PSI/BAR -
                           verify unit) */
  float tyrePressureFR; /**< [UNVERIFIED] Tyre pressure Front Right (PSI/BAR -
                           verify unit) */
  float lapsCompleted;  /**< [UNVERIFIED] Number of laps completed */
  float totalLaps;      /**< [UNVERIFIED] Total laps in session */
  float trackLength;    /**< [UNVERIFIED] Track length (meters) */
  float lastLapTime;    /**< [UNVERIFIED] Duration of the last completed lap
                           (seconds) */
  float maxRPM;  /**< [UNVERIFIED] Maximum engine RPM (raw value is RPM / 10) */
  float idleRPM; /**< [UNVERIFIED] Idle engine RPM (raw value is RPM / 10) */
  float maxGears; /**< [UNVERIFIED] Maximum number of gears */
} DR2_UDP_ExtraData3;

// Main packet structure using a union for both named field access and array
// access
struct dirt2_udp_packet {
  union {
    float data[66];
    DR2_UDP_ExtraData3 fields;
  };
};

#pragma pack(pop)

// Note: This structure is based on DiRT Rally 2.0 extradata=3 specification.
// Gear mapping: 0=neutral, 1-10=gears, -1=reverse.

#endif
