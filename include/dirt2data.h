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
  float runTime;   /**< Total time the session has been running
                      (seconds) */
  float lapTime;   /**< Current lap time (seconds) */
  float distance;  /**< [UNVERIFIED] Total distance travelled in the session
                      (meters) */
  float progress;  /**< Current lap progress (0.0 to 1.0, accumulative for each
                      lap) */
  float posX;      /**< World position X (East/West) */
  float posY;      /**< World position Y (Up/Down) */
  float posZ;      /**< World position Z (North/South) */
  float speed;     /**< Speed (m/s) */
  float velX;      /**< Velocity in world X direction */
  float velY;      /**< Velocity in world Y direction */
  float velZ;      /**< Velocity in world Z direction */
  float rightX;    /**< Local right vector X component */
  float rightY;    /**< Local right vector Y component */
  float rightZ;    /**< Local right vector Z component */
  float forwardX;  /**< Local forward vector X component */
  float forwardY;  /**< Local forward vector Y component */
  float forwardZ;  /**< Local forward vector Z component */
  float suspRL;    /**< Suspension position Rear Left */
  float suspRR;    /**< Suspension position Rear Right */
  float suspFL;    /**< Suspension position Front Left */
  float suspFR;    /**< Suspension position Front Right */
  float suspVelRL; /**< Suspension velocity Rear Left */
  float suspVelRR; /**< Suspension velocity Rear Right */
  float suspVelFL; /**< Suspension velocity Front Left */
  float suspVelFR; /**< Suspension velocity Front Right */
  float wheelSpeedRL;  /**< Wheel speed Rear Left (m/s) */
  float wheelSpeedRR;  /**< Wheel speed Rear Right (m/s) */
  float wheelSpeedFL;  /**< Wheel speed Front Left (m/s) */
  float wheelSpeedFR;  /**< Wheel speed Front Right (m/s) */
  float throttle;      /**< Throttle input (0.0 to 1.0) */
  float steering;      /**< Steering input (-1.0 to 1.0) */
  float brake;         /**< Brake input (0.0 to 1.0) */
  float clutch;        /**< Clutch input (0.0 to 1.0) */
  float gear;          /**< Current gear (0=neutral, 1-10=gears, -1=reverse) */
  float gForceLat;     /**< Lateral G-Force (Field 34) */
  float gForceLon;     /**< Longitudinal G-Force (Field 35) */
  float lap;           /**< Current lap number */
  float engineRPM;     /**< Engine RPM (raw value is RPM / 10) */
  float sliProSupport; /**< [UNVERIFIED] Sli-Pro support data */
  float racePos;       /**< Current race position */
  float kersLevel;     /**< [UNVERIFIED] KERS level (likely unused in DR2) */
  float kersMaxLevel; /**< [UNVERIFIED] KERS max level (likely unused in DR2) */
  float drs;          /**< [UNVERIFIED] DRS status (likely unused in DR2) */
  float tractionControl; /**< [UNVERIFIED] Traction control setting/activity */
  float antiLockBrakes;  /**< [UNVERIFIED] ABS setting/activity */
  float fuelInTank;      /**< Current fuel in tank (liters) (always 0.0) */
  float fuelCapacity;    /**< Fuel tank capacity (liters) (always 0.0) */
  float inPit;       /**< [UNVERIFIED] Pit lane status (0=on track, 1=in pit) */
  float sector;      /**< Current sector index */
  float sector1Time; /**< Time for sector 1 (seconds) */
  float sector2Time; /**< Time for sector 2 (seconds) */
  float brakesTempRL;   /**< Brake temperature Rear Left (Celsius) */
  float brakesTempRR;   /**< Brake temperature Rear Right (Celsius) */
  float brakesTempFL;   /**< Brake temperature Front Left (Celsius) */
  float brakesTempFR;   /**< Brake temperature Front Right (Celsius) */
  float tyrePressureRL; /**< Tyre pressure Rear Left (always 0.0) */
  float tyrePressureRR; /**< Tyre pressure Rear Right (always 0.0) */
  float tyrePressureFL; /**< Tyre pressure Front Left (always 0.0) */
  float tyrePressureFR; /**< Tyre pressure Front Right (always 0.0) */
  float lapsCompleted;  /**< Number of laps completed */
  float totalLaps;      /**< Total laps in session */
  float trackLength;    /**< Track length (meters) */
  float lastLapTime;    /**< Duration of the last completed lap (seconds) */
  float maxRPM;         /**< Maximum engine RPM (raw value is RPM / 10) */
  float idleRPM;        /**< Idle engine RPM (raw value is RPM / 10) */
  float maxGears;       /**< Maximum number of gears */
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
