#ifndef __PacketCarStatusData_h__
#define __PacketCarStatusData_h__

typedef struct //CarStatusData
{
    uint8_t     m_tractionControl;          // 0 (off) - 2 (high)
    uint8_t     m_antiLockBrakes;           // 0 (off) - 1 (on)
    uint8_t     m_fuelMix;                  // Fuel mix - 0 = lean, 1 = standard, 2 = rich, 3 = max
    uint8_t     m_frontBrakeBias;           // Front brake bias (percentage)
    uint8_t     m_pitLimiterStatus;         // Pit limiter status - 0 = off, 1 = on
    float       m_fuelInTank;               // Current fuel mass
    float       m_fuelCapacity;             // Fuel capacity
    uint16_t    m_maxRPM;                   // Cars max RPM, point of rev limiter
    uint16_t    m_idleRPM;                  // Cars idle RPM
    uint8_t     m_maxGears;                 // Maximum number of gears
    uint8_t     m_drsAllowed;               // 0 = not allowed, 1 = allowed, -1 = unknown
    uint8_t     m_tyresWear[4];             // Tyre wear percentage
    uint8_t     m_tyreCompound;             // Modern - 0 = hyper soft, 1 = ultra soft
    // 2 = super soft, 3 = soft, 4 = medium, 5 = hard
    // 6 = super hard, 7 = inter, 8 = wet
    // Classic - 0-6 = dry, 7-8 = wet
    uint8_t     m_tyresDamage[4];           // Tyre damage (percentage)
    uint8_t     m_frontLeftWingDamage;      // Front left wing damage (percentage)
    uint8_t     m_frontRightWingDamage;     // Front right wing damage (percentage)
    uint8_t     m_rearWingDamage;           // Rear wing damage (percentage)
    uint8_t     m_engineDamage;             // Engine damage (percentage)
    uint8_t     m_gearBoxDamage;            // Gear box damage (percentage)
    uint8_t     m_exhaustDamage;            // Exhaust damage (percentage)
    int8_t      m_vehicleFiaFlags;          // -1 = invalid/unknown, 0 = none, 1 = green
    // 2 = blue, 3 = yellow, 4 = red
    float       m_ersStoreEnergy;           // ERS energy store in Joules
    uint8_t     m_ersDeployMode;            // ERS deployment mode, 0 = none, 1 = low, 2 = medium
    // 3 = high, 4 = overtake, 5 = hotlap
    float       m_ersHarvestedThisLapMGUK;  // ERS energy harvested this lap by MGU-K
    float       m_ersHarvestedThisLapMGUH;  // ERS energy harvested this lap by MGU-H
    float       m_ersDeployedThisLap;       // ERS energy deployed this lap

} CarStatusData;

struct PacketCarStatusData
{
    CarStatusData       m_carStatusData[20];
};

#endif
