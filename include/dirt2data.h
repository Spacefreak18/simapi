#ifndef _DIRT2DATA_H
#define _DIRT2DATA_H

#include <stdint.h>

#pragma pack(push, 1)

// DiRT Rally 2.0 UDP packet structure (extradata=3)
// 66 floats (264 bytes)
typedef struct {
    float runTime;              // 0
    float lapTime;              // 1
    float distance;             // 2
    float progress;             // 3
    float posX;                 // 4
    float posY;                 // 5
    float posZ;                 // 6
    float speed;                // 7 (m/s)
    float velX;                 // 8
    float velY;                 // 9
    float velZ;                 // 10
    float rightX;               // 11
    float rightY;               // 12
    float rightZ;               // 13
    float forwardX;             // 14
    float forwardY;             // 15
    float forwardZ;             // 16
    float suspRL;               // 17
    float suspRR;               // 18
    float suspFL;               // 19
    float suspFR;               // 20
    float suspVelRL;            // 21
    float suspVelRR;            // 22
    float suspVelFL;            // 23
    float suspVelFR;            // 24
    float wheelSpeedRL;         // 25
    float wheelSpeedRR;         // 26
    float wheelSpeedFL;         // 27
    float wheelSpeedFR;         // 28
    float throttle;             // 29
    float steering;             // 30
    float brake;                // 31
    float clutch;               // 32
    float gear;                 // 33
    float gForceVert;           // 34
    float gForceLon;            // 35
    float lap;                  // 36
    float engineRPM;            // 37 (raw is RPM / 10)
    float sliProSupport;        // 38
    float racePos;              // 39
    float kersLevel;            // 40
    float kersMaxLevel;         // 41
    float drs;                  // 42
    float tractionControl;      // 43
    float antiLockBrakes;       // 44
    float fuelInTank;           // 45
    float fuelCapacity;         // 46
    float inPit;                // 47
    float sector;               // 48
    float sector1Time;          // 49
    float sector2Time;          // 50
    float brakesTempRL;         // 51
    float brakesTempRR;         // 52
    float brakesTempFL;         // 53
    float brakesTempFR;         // 54
    float tyrePressureRL;       // 55
    float tyrePressureRR;       // 56
    float tyrePressureFL;       // 57
    float tyrePressureFR;       // 58
    float lapsCompleted;        // 59
    float totalLaps;            // 60
    float trackLength;          // 61
    float lastLapTime;          // 62
    float maxRPM;               // 63 (raw is RPM / 10)
    float idleRPM;              // 64 (raw is RPM / 10)
    float maxGears;             // 65
} DR2_UDP_ExtraData3;

// Main packet structure using a union for both named field access and array access
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
