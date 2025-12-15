#ifndef __PacketCarSetupData_h__
#define __PacketCarSetupData_h__

typedef struct //CarSetupData
{
    uint8_t   m_frontWing;                // Front wing aero
    uint8_t   m_rearWing;                 // Rear wing aero
    uint8_t   m_onThrottle;               // Differential adjustment on throttle (percentage)
    uint8_t   m_offThrottle;              // Differential adjustment off throttle (percentage)
    float     m_frontCamber;              // Front camber angle (suspension geometry)
    float     m_rearCamber;               // Rear camber angle (suspension geometry)
    float     m_frontToe;                 // Front toe angle (suspension geometry)
    float     m_rearToe;                  // Rear toe angle (suspension geometry)
    uint8_t   m_frontSuspension;          // Front suspension
    uint8_t   m_rearSuspension;           // Rear suspension
    uint8_t   m_frontAntiRollBar;         // Front anti-roll bar
    uint8_t   m_rearAntiRollBar;          // Front anti-roll bar
    uint8_t   m_frontSuspensionHeight;    // Front ride height
    uint8_t   m_rearSuspensionHeight;     // Rear ride height
    uint8_t   m_brakePressure;            // Brake pressure (percentage)
    uint8_t   m_brakeBias;                // Brake bias (percentage)
    float     m_frontTyrePressure;        // Front tyre pressure (PSI)
    float     m_rearTyrePressure;         // Rear tyre pressure (PSI)
    uint8_t   m_ballast;                  // Ballast
    float     m_fuelLoad;                 // Fuel load

} CarSetupData;

struct PacketCarSetupData
{
    CarSetupData    m_carSetups[20];

};

#endif
