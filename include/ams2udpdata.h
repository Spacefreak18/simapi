#include <stdint.h>

struct ams2UDPData {
    // starts with packet base (0-12)
    uint32_t mPacketNumber;               // 0 counter reflecting all the packets that have been sent during the game run
    uint32_t mCategoryPacketNumber;       // 4 counter of the packet groups belonging to the given category
    uint8_t mPartialPacketIndex;          // 8 If the data from this class had to be sent in several packets, the index number
    uint8_t mPartialPacketNumber;         // 9 If the data from this class had to be sent in several packets, the total number
    uint8_t mPacketType;                  // 10 what is the type of this packet
    uint8_t mPacketVersion;               // 11 what is the version of protocol for this handler

    // Participant info
    int8_t sViewedParticipantIndex;       // 12

    uint8_t sUnfilteredThrottle;          // 13 1
    uint8_t sUnfilteredBrake;             // 14 1
    int8_t sUnfilteredSteering;           // 15 1
    uint8_t sUnfilteredClutch;            // 16 1

    uint8_t sCarFlags;                    // 17 1
    int16_t sOilTempCelsius;              // 18 2
    uint16_t sOilPressureKPa;             // 20 2
    int16_t sWaterTempCelsius;            // 22 2
    uint16_t sWaterPressureKPa;           // 24 2
    uint16_t sFuelPressureKPa;            // 26 2
    uint8_t sFuelCapacity;                // 28 1
    uint8_t sBrake;                       // 29 1
    uint8_t sThrottle;                    // 30 1
    uint8_t sClutch;                      // 31 1
    float sFuelLevel;                     // 32 4
    float sSpeed;                         // 36 4
    uint16_t sRpm;                        // 40 2
    uint16_t sMaxRpm;                     // 42 2
    int8_t sSteering;                     // 44 1
    uint8_t sGearNumGears;                // 45 1
    uint8_t sBoostAmount;                 // 46 1
    uint8_t sCrashState;                  // 47 1
    float sOdometerKM;                    // 48 4
    float sOrientation[3];                // 52 12
    float sLocalVelocity[3];              // 64 12
    float sWorldVelocity[3];              // 76 12
    float sAngularVelocity[3];            // 88 12
    float sLocalAcceleration[3];          // 100 12
    float sWorldAcceleration[3];          // 112 12
    float sExtentsCentre[3];              // 124 12
    uint8_t sTyreFlags[4];                // 136 4
    uint8_t sTerrain[4];                  // 140 4
    float sTyreY[4];                      // 144 16
    float sTyreRPS[4];                    // 160 16
    uint8_t sTyreTemp[4];                 // 176 4
    float sTyreHeightAboveGround[4];      // 180 16
    uint8_t sTyreWear[4];                 // 196 4
    uint8_t sBrakeDamage[4];              // 200 4
    uint8_t sSuspensionDamage[4];         // 204 4
    int16_t sBrakeTempCelsius[4];         // 208 8
    uint16_t sTyreTreadTemp[4];           // 216 8
    uint16_t sTyreLayerTemp[4];           // 224 8
    uint16_t sTyreCarcassTemp[4];         // 232 8
    uint16_t sTyreRimTemp[4];             // 240 8
    uint16_t sTyreInternalAirTemp[4];     // 248 8
    uint16_t sTyreTempLeft[4];            // 256 8
    uint16_t sTyreTempCenter[4];          // 264 8
    uint16_t sTyreTempRight[4];           // 272 8
    float sWheelLocalPositionY[4];        // 280 16
    float sRideHeight[4];                  // 296 16
    float sSuspensionTravel[4];           // 312 16
    float sSuspensionVelocity[4];         // 328 16
    uint16_t sSuspensionRideHeight[4];    // 344 8
    uint16_t sAirPressure[4];              // 352 8
    float sEngineSpeed;                    // 360 4
    float sEngineTorque;                   // 364 4
    uint8_t sWings[2];                     // 368 2
    uint8_t sHandBrake;                   // 370 1
    // Car damage
    uint8_t sAeroDamage;                   // 371 1
    uint8_t sEngineDamage;                 // 372 1
    // HW state
    uint8_t sJoyPad1;                      // 376 4
    uint8_t sJoyPad2;                      // 376 4
    uint8_t sDPad;                         // 377 1
    uint8_t lfTyreCompound[40];           // 378 40
    uint8_t rfTyreCompound[40];           // 418 40
    uint8_t lrTyreCompound[40];           // 458 40
    uint8_t rrTyreCompound[40];           // 498 40

    float sTurboBoostPressure;             // 538 4
    float sFullPosition[3];                // 542 12
    uint8_t sBrakeBias;                    // 554 1
};
