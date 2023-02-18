#pragma once

#ifndef _ACDATA_H
#define _ACDATA_H

#include <uchar.h>

typedef int AC_STATUS;

#define AC_OFF 0
#define AC_REPLAY 1
#define AC_LIVE 2
#define AC_PAUSE 3

typedef int AC_SESSION_TYPE;

#define AC_UNKNOWN -1
#define AC_PRACTICE 0
#define AC_QUALIFY 1
#define AC_RACE 2
#define AC_HOTLAP 3
#define AC_TIME_ATTACK 4
#define AC_DRIFT 5
#define AC_DRAG 6


#pragma pack(push)
#pragma pack(4)

typedef struct
{
  float x, y, z;
}
acsVec3;

typedef struct
{
    int carId;
    char driverName[64];
    char carModel[64];
    float speedMS;
    int bestLapMS;
    int lapCount;
    int currentLapInvalid;
    int currentLapTimeMS;
    int lastLapTimeMS;
    acsVec3 worldPosition;
    int isCarInPitline;
    int isCarInPit;
    int carLeaderboardPosition;
    int carRealTimeLeaderboardPosition;
    float spLineLength;
    int isConnected;
    float suspensionDamage[4];
    float engineLifeLeft;
    float tyreInflation[4];
}
acsVehicleInfo;

struct SPageFilePhysics
{
    int packetId;
    float gas;
    float brake;
    float fuel;
    int gear;
    int rpms;
    float steerAngle;
    float speedKmh;
    float velocity[3];
    float accG[3];
    float wheelSlip[4];
    float wheelLoad[4];
    float wheelsPressure[4];
    float wheelAngularSpeed[4];
    float tyreWear[4];
    float tyreDirtyLevel[4];
    float tyreCoreTemperature[4];
    float camberRAD[4];
    float suspensionTravel[4];
    float drs;
    float tc;
    float heading;
    float pitch;
    float roll;
    float cgHeight;
    float carDamage[5];
    int numberOfTyresOut;
    int pitLimiterOn;
    float abs;
    float kersCharge;
    float kersInput;
    int autoShifterOn;
    float rideHeight[2];
    float turboBoost;
    float ballast;
    float airDensity;
    float airTemp;
    float roadTemp;
    float localAngularVel[3];
    float finalFF;
    float performanceMeter;
    int engineBrake;
    int ersRecoveryLevel;
    int ersPowerLevel;
    int ersHeatCharging;
    int ersIsCharging;
    float kersCurrentKJ;
    int drsAvailable;
    int drsEnabled;
    float brakeTemp[4];
    float clutch;
    float tyreTempI[4];
    float tyreTempM[4];
    float tyreTempO[4];
    int isAIControlled;
    acsVec3 tyreContactPoint[4];
    acsVec3 tyreContactNormal[4];
    acsVec3 tyreContactHeading[4];
    float brakeBias;
    acsVec3 localVelocity;
};


struct SPageFileGraphic
{
    int packetId;
    AC_STATUS status;
    AC_SESSION_TYPE session;
    char16_t currentTime[15];
    char16_t lastTime[15];
    char16_t bestTime[15];
    char16_t split[15];
    int completedLaps;
    int position;
    int iCurrentTime;
    int iLastTime;
    int iBestTime;
    float sessionTimeLeft;
    float distanceTraveled;
    int isInPit;
    int currentSectorIndex;
    int lastSectorTime;
    int numberOfLaps;
    char tyreCompound[33];

    float replayTimeMultiplier;
    float normalizedCarPosition;
    float carCoordinates[3];
};


struct SPageFileStatic
{
    char16_t smVersion[15];
    char16_t acVersion[15];
    // session static info
    int numberOfSessions;
    int numCars;
    char16_t carModel[33];
    char16_t track[33];
    char16_t playerName[33];
    char16_t playerSurname[33];
    char16_t playerNick[33];
    int sectorCount;

    // car static info
    float maxTorque;
    float maxPower;
    int	maxRpm;
    float maxFuel;
    float suspensionMaxTravel[4];
    float tyreRadius[4];
   
    // since 1.5
    float MaxTurboBoost;
    float Deprecated1; // AirTemp since 1.6 in physic
    float Deprecated2; // RoadTemp since 1.6 in physic
    int PenaltiesEnabled;
    float AidFuelRate;
    float AidTireRate;
    float AidMechanicalDamage;
    int AidAllowTyreBlankets;
    float AidStability;
    int AidAutoClutch;
    int AidAutoBlip;

    // since 1.7.1
    int HasDRS;
    int HasERS;
    int HasKERS;
    float KersMaxJoules;
    int EngineBrakeSettingsCount;
    int ErsPowerControllerCount;

    // since 1.7.2
    float TrackSPlineLength;
    char16_t TrackConfiguration[15];

    // since 1.10.2
    float ErsMaxJ;

    // since 1.13
    int IsTimedRace;
    int HasExtraLap;
    char16_t CarSkin[33];
    int ReversedGridPositions;
    int PitWindowStart;
    int PitWindowEnd;

};


struct SPageFileCrewChief
{
    int numVehicles;
    int focuseVehicle;
    char serverName[512];
    acsVehicleInfo vehicle[64];
    char acInstallPath[512];
    int isInternalMemoryModuleLoaded;
    char pluginVersion[32];
};


#pragma pack(pop)
#endif
