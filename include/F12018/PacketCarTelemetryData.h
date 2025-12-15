#ifndef __PacketCarTelemetryData_h__
#define __PacketCarTelemetryData_h__


typedef struct //CarTelemetryData
{
    uint16_t  m_speed;                      // Speed of car in kilometres per hour
    uint8_t   m_throttle;                   // Amount of throttle applied (0 to 100)
    int8_t    m_steer;                      // Steering (-100 (full lock left) to 100 (full lock right))
    uint8_t   m_brake;                      // Amount of brake applied (0 to 100)
    uint8_t   m_clutch;                     // Amount of clutch applied (0 to 100)
    int8_t    m_gear;                       // Gear selected (1-8, N=0, R=-1)
    uint16_t  m_engineRPM;                  // Engine RPM
    uint8_t   m_drs;                        // 0 = off, 1 = on
    uint8_t   m_revLightsPercent;           // Rev lights indicator (percentage)
    uint16_t  m_brakesTemperature[4];       // Brakes temperature (celsius)
    uint16_t  m_tyresSurfaceTemperature[4]; // Tyres surface temperature (celsius)
    uint16_t  m_tyresInnerTemperature[4];   // Tyres inner temperature (celsius)
    uint16_t  m_engineTemperature;          // Engine temperature (celsius)
    float     m_tyresPressure[4];           // Tyres pressure (PSI)

} CarTelemetryData;

struct PacketCarTelemetryData
{
    CarTelemetryData    m_carTelemetryData[20];

    uint32_t            m_buttonStatus;         // Bit flags specifying which buttons are being
                                                // pressed currently - see appendices

};

#endif
