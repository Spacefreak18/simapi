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

typedef int ACC_FLAG_TYPE;;
#define ACC_NO_FLAG = 0,
#define ACC_BLUE_FLAG = 1,
#define ACC_YELLOW_FLAG = 2,
#define ACC_BLACK_FLAG = 3,
#define ACC_WHITE_FLAG = 4,
#define ACC_CHECKERED_FLAG = 5,
#define ACC_PENALTY_FLAG = 6,
#define ACC_GREEN_FLAG = 7,
#define ACC_ORANGE_FLAG = 8

typedef int ACC_PENALTY_TYPE;
#define ACC_NONE = 0,
#define ACC_DRIVETHROUGH_CUTTING = 1,
#define ACC_STOPANDGO_10_CUTTING = 2,
#define ACC_STOPANDGO_20_CUTTING = 3,
#define ACC_STOPANDGO_30_CUTTING = 4,
#define ACC_DISQUALIFIED_CUTTING = 5,
#define ACC_REMOVEBESTLAPTIME_CUTTING = 6,
#define ACC_DRIVETHROUGH_PITSPEEDING = 7,
#define ACC_STOPANDGO_10_PITSPEEDING = 8,
#define ACC_STOPANDGO_20_PITSPEEDING = 9,
#define ACC_STOPANDGO_30_PITSPEEDING = 10,
#define ACC_DISQUALIFIED_PITSPEEDING = 11,
#define ACC_REMOVEBESTLAPTIME_PITSPEEDING = 12,
#define ACC_DISQUALIFIED_IGNOREDMANDATORYPIT = 13,
#define ACC_POSTRACETIME = 14,
#define ACC_DISQUALIFIED_TROLLING = 15,
#define ACC_DISQUALIFIED_PITENTRY = 16,
#define ACC_DISQUALIFIED_PITEXIT = 17,
#define ACC_DISQUALIFIED_WRONGWAY = 18,
#define ACC_DRIVETHROUGH_IGNOREDDRIVERSTINT = 19,
#define ACC_DISQUALIFIED_IGNOREDDRIVERSTINT = 20,
#define ACC_DISQUALIFIED_EXCEEDEDDRIVERSTINTLIMIT = 21

typedef int ACC_TRACK_GRIP_STATUS;
#define ACC_GREEN = 0,
#define ACC_FAST = 1,
#define ACC_OPTIMUM = 2,
#define ACC_GREASY = 3,
#define ACC_DAMP = 4,
#define ACC_WET = 5,
#define ACC_FLOODED = 6

typedef int ACC_RAIN_INTENSITY;
#define ACC_NO_RAIN = 0,
#define ACC_DRIZZLE = 1,
#define ACC_LIGHT_RAIN = 2,
#define ACC_MEDIUM_RAIN = 3,
#define ACC_HEAVY_RAIN = 4,
#define ACC_THUNDERSTORM = 5

#pragma pack(push)
#pragma pack(4)

typedef struct //acsVec3
{
  float x;
  float y;
  float z;
} acsVec3;

typedef struct //accVehicleInfo
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
} acsVehicleInfo;

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
    int P2PActivation; //Not used in ACC
    int P2PStatus; //Not used in ACC
    float CurrentMaxRPM; //Not used in ACC
    float MZ[4];
    float FX[4];
    float FY[4];
    float SlipRatio[4];
    float SlipAngle[4];
    int TCInAction; //Not used in ACC
    int ABSInAction; //Not used in ACC
    float SuspensionDamage[4];
    float TyreTemp[4];
    float WaterTemp;
    float BrakePressure[4];
    int frontBrakeCompound;
    int rearBrakeCompound;
    float padLife[4];
    float discLife[4];
    int ignitionOn;
    int starterEngineOn;
    int isEngineRunning;
    float kerbVibration;
    float slipVibration;
    float gVibration;
    float absbVibration;
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
    int CarID[60];

    int PlayerCarId;

    float PenaltyTime;
    ACC_FLAG_TYPE Flag;
    ACC_PENALTY_TYPE Penalty;
    int IdealLineOn;

    int IsInPitLane;
    float SurfaceGrip;

    int MandatoryPitDone;
    float WindSpeed;
    float WindDirection;
    int IsSetupMenuVisible;
    int MainDisplayIndex;
    int SecondaryDisplayIndex;
    int TC;
    int TCUT;
    int EngineMap;
    int ABS;
    float FuelXLap;
    int RainLights;
    int FlashingLights;
    int LightsStage;
    float ExhaustTemperature;
    int WiperLV;
    int DriverStingTotalTimeLeft;
    int DriverStingTimeLeft;
    int RainTyres;
    int SessionIndex;
    float UsedFuel; //Since last refuel

    char DeltaLapTime[15];
    int IDeltaLapTime;
    char EstimatedLapTime[15];
    int IEstimatedLapTime;

    int IsDeltaPositive;
    int ISplit; //Last split time in ms
    int IsValidLap;
    float FuelEstimatedLaps;

    char TrackStatus[33];
    int MissingMandatoryPits;

    int directionLightsLeft;
    int directionLightsRight;

    int GlobalYellow;
    int GlobalYellow1;
    int GlobalYellow2;
    int GlobalYellow3;
    int GlobalWhite;
    int GlobalGreen;
    int GlobalChequered;
    int GlobalRed;
    int mfdTyreSet;
    float mfdFuelToAdd;
    float mfdTyrePressureLF;
    float mfdTyrePressureRF;
    float mfdTyrePressureLR;
    float mfdTyrePressureRR;
    ACC_TRACK_GRIP_STATUS trackGripStatus;
    ACC_RAIN_INTENSITY rainIntensity;
    ACC_RAIN_INTENSITY rainIntensityIn10min;
    ACC_RAIN_INTENSITY rainIntensityIn30min;
    int currentTyreSet;
    int strategyTyreSet;
    int gapAhead;
    int gapBehind;
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
    int IsOnline;
    char dryTyresName[33];
    char wetTyresName[33];
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
