#ifndef __PacketLapData_h__
#define __PacketLapData_h__


enum PitStatus: uint8_t {
    PIT_STATUS_NONE = 0,
    PIT_STATUS_PITTING = 1,
    PIT_STATUS_IN_PIT_AREA = 2,
};

static const char* const PitStatus_name[] = {
    "NONE", "PITTING", "IN_PIT_AREA",
};

enum DriverStatus: uint8_t {
    DRIVER_STATUS_IN_GARAGE = 0,
    DRIVER_STATUS_FLYING_LAP = 1,
    DRIVER_STATUS_IN_LAP = 2,
    DRIVER_STATUS_OUT_LAP = 3,
    DRIVER_STATUS_ON_TRACK = 4,
};
static const char* const DriverStatus_name[] = {
    "IN_GARAGE", "FLYING_LAP", "IN_LAP", "OUT_LAP", "ON_TRACK",
};

enum ResultStatus: uint8_t {
    RESULT_STATUS_INVALID = 0,
    RESULT_STATUS_INACTIVE = 1,
    RESULT_STATUS_ACTIVE = 2,
    RESULT_STATUS_FINISHED = 3,
    RESULT_STATUS_DISQUALIFIED = 4,
    RESULT_STATUS_NOT_CLASSIFIED = 5,
    RESULT_STATUS_RETIRED = 6,
};
static const char* const ResultStatus_name[] = {
    "INVALID", "INACTIVE", "ACTIVE", "FINISHED", "DISQUALIFIED",
    "NOT_CLASSIFIED", "RETIRED",
};


typedef struct //LapData
{
    float       m_lastLapTime;           // Last lap time in seconds
    float       m_currentLapTime;        // Current time around the lap in seconds
    float       m_bestLapTime;           // Best lap time of the session in seconds
    float       m_sector1Time;           // Sector 1 time in seconds
    float       m_sector2Time;           // Sector 2 time in seconds
    float       m_lapDistance;           // Distance vehicle is around current lap in metres – could
                                         // be negative if line hasn’t been crossed yet
    float       m_totalDistance;         // Total distance travelled in session in metres – could
                                         // be negative if line hasn’t been crossed yet
    float       m_safetyCarDelta;        // Delta in seconds for safety car
    uint8_t     m_carPosition;           // Car race position
    uint8_t     m_currentLapNum;         // Current lap number
    uint8_t     m_pitStatus;             // 0 = none, 1 = pitting, 2 = in pit area
    uint8_t     m_sector;                // 0 = sector1, 1 = sector2, 2 = sector3
    uint8_t     m_currentLapInvalid;     // Current lap invalid - 0 = valid, 1 = invalid
    uint8_t     m_penalties;             // Accumulated time penalties in seconds to be added
    uint8_t     m_gridPosition;          // Grid position the vehicle started the race in
    uint8_t     m_driverStatus;          // Status of driver - 0 = in garage, 1 = flying lap
                                         // 2 = in lap, 3 = out lap, 4 = on track
    uint8_t     m_resultStatus;          // Result status - 0 = invalid, 1 = inactive, 2 = active
                                         // 3 = finished, 4 = disqualified, 5 = not classified
                                         // 6 = retired
} LapData;

struct PacketLapData
{
    LapData         m_lapData[20];         // Lap data for all cars on track

};

#endif
