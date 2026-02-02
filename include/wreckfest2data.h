///////////////////////////////////////////////////////////////////////////
// Wreckfest 2 Telemetry Data Format (Pino) - C Compatible Version
///////////////////////////////////////////////////////////////////////////
// Copyright (c) Bugbear Entertainment ltd.
// All Rights Reserved.
///////////////////////////////////////////////////////////////////////////

#ifndef _WRECKFEST2DATA_H
#define _WRECKFEST2DATA_H

#include <stdint.h>

// Save current packing alignment and set it to 1 byte (no padding)
#pragma pack(push, 1)

// Type aliases
typedef char WF2_S8;
typedef unsigned char WF2_U8;
typedef short WF2_S16;
typedef unsigned short WF2_U16;
typedef int WF2_S32;
typedef unsigned int WF2_U32;

// Constants
#define WF2_PARTICIPANTS_MAX 36
#define WF2_TRACK_ID_LENGTH_MAX 64
#define WF2_TRACK_NAME_LENGTH_MAX 96
#define WF2_CAR_ID_LENGTH_MAX 64
#define WF2_CAR_NAME_LENGTH_MAX 96
#define WF2_PLAYER_NAME_LENGTH_MAX 24
#define WF2_DAMAGE_PARTS_MAX 56
#define WF2_DAMAGE_BITS_PER_PART 3
#define WF2_DAMAGE_BYTES_PER_PARTICIPANT 21  // (56 * 3 + 7) / 8

// Packet Types
enum WF2_PacketType
{
    WF2_PACKET_TYPE_MAIN = 0,
    WF2_PACKET_TYPE_PARTICIPANTS_LEADERBOARD = 1,
    WF2_PACKET_TYPE_PARTICIPANTS_TIMING = 2,
    WF2_PACKET_TYPE_PARTICIPANTS_TIMING_SECTORS = 3,
    WF2_PACKET_TYPE_PARTICIPANTS_MOTION = 4,
    WF2_PACKET_TYPE_PARTICIPANTS_INFO = 5,
    WF2_PACKET_TYPE_PARTICIPANTS_DAMAGE = 6
};

// Game Status Flags
enum WF2_GameStatusFlags
{
    WF2_GAME_STATUS_PAUSED = (1 << 0),
    WF2_GAME_STATUS_REPLAY = (1 << 1),
    WF2_GAME_STATUS_SPECTATE = (1 << 2),
    WF2_GAME_STATUS_MULTIPLAYER_CLIENT = (1 << 3),
    WF2_GAME_STATUS_MULTIPLAYER_SERVER = (1 << 4),
    WF2_GAME_STATUS_IN_RACE = (1 << 5)
};

// Packet Header
typedef struct
{
    WF2_U32 signature;         // 1869769584 - identifies Pino packet
    WF2_U8 packetType;
    WF2_U8 statusFlags;
    WF2_S32 sessionTime;       // Primary timestamp, millisecond
    WF2_S32 raceTime;          // Race time from lights out, millisecond
} WF2_PacketHeader;

// Session Information
typedef struct
{
    char trackId[WF2_TRACK_ID_LENGTH_MAX];
    char trackName[WF2_TRACK_NAME_LENGTH_MAX];
    float trackLength;         // meters

    WF2_S16 laps;
    WF2_S16 eventLength;       // seconds

    WF2_U8 gridSize;
    WF2_U8 gridSizeRemaining;

    WF2_U8 sectorCount;
    float sectorFract1;
    float sectorFract2;

    WF2_U8 gameMode;
    WF2_U8 damageMode;
    WF2_U8 status;

    char reserved[26];
} WF2_Session;

// Motion: Orientation
typedef struct
{
    float positionX;           // meters
    float positionY;
    float positionZ;

    float orientationQuaternionX;
    float orientationQuaternionY;
    float orientationQuaternionZ;
    float orientationQuaternionW;

    WF2_U16 extentsX;          // centimeters
    WF2_U16 extentsY;
    WF2_U16 extentsZ;
} WF2_Orientation;

// Motion: Velocity
typedef struct
{
    float velocityLocalX;      // m/s
    float velocityLocalY;
    float velocityLocalZ;

    float angularVelocityX;    // rad/s
    float angularVelocityY;
    float angularVelocityZ;

    float accelerationLocalX;  // m/s^2
    float accelerationLocalY;
    float accelerationLocalZ;
} WF2_Velocity;

// Car: Tire
typedef struct
{
    float rps;                 // rad/s
    float camber;              // radian
    float slipRatio;
    float slipAngle;           // radian
    float radiusUnloaded;      // meter

    float loadVertical;        // newton
    float forceLat;
    float forceLong;

    float temperatureInner;    // Kelvin
    float temperatureTread;

    float suspensionVelocity;  // m/s
    float suspensionDisplacement;  // meter
    float suspensionDispNorm;  // 0-1

    float positionVertical;    // meter

    WF2_U8 surfaceType;

    char reserved[15];
} WF2_Tire;

// Car: Engine
typedef struct
{
    WF2_U8 flags;

    WF2_S32 rpm;
    WF2_S32 rpmMax;
    WF2_S32 rpmRedline;
    WF2_S32 rpmIdle;

    float torque;              // newton-metre
    float power;               // watt

    float tempBlock;           // Kelvin
    float tempWater;

    float pressureManifold;    // kilopascal
    float pressureOil;

    char reserved[15];
} WF2_Engine;

// Car: Driveline
typedef struct
{
    WF2_U8 type;               // FWD/RWD/AWD

    WF2_U8 gear;               // 0=R, 1=N, 2=1st...
    WF2_U8 gearMax;

    float speed;               // m/s

    char reserved[17];
} WF2_Driveline;

// Car: Input
typedef struct
{
    float throttle;            // 0-1
    float brake;
    float clutch;
    float handbrake;
    float steering;            // -1 to +1
} WF2_Input;

// Car: Assists
typedef struct
{
    WF2_U8 flags;
    WF2_U8 assistGearbox;
    WF2_U8 levelAbs;
    WF2_U8 levelTcs;
    WF2_U8 levelEsc;

    char reserved[3];
} WF2_Assists;

// Car: Chassis
typedef struct
{
    float trackWidth[2];       // AXLE_LOCATION_COUNT = 2
    float wheelBase;

    WF2_S32 steeringWheelLockToLock;  // degrees
    float steeringLock;        // radian

    float cornerWeights[4];    // newton

    char reserved[16];
} WF2_Chassis;

// Car: Full
typedef struct
{
    WF2_Assists assists;
    WF2_Chassis chassis;
    WF2_Driveline driveline;
    WF2_Engine engine;
    WF2_Input input;
    WF2_Orientation orientation;
    WF2_Velocity velocity;
    WF2_Tire tires[4];         // FL, FR, RL, RR

    char reserved[14];
} WF2_CarFull;

// Input Extended
typedef struct
{
    WF2_U8 flags;
    float ffbForce;            // normalized

    char reserved[15];
} WF2_InputExtended;

// Participant: Leaderboard
typedef struct
{
    WF2_U8 status;
    WF2_U8 trackStatus;

    WF2_U16 lapCurrent;        // 1st lap = 1
    WF2_U8 position;           // 1st = 1
    WF2_U8 health;             // 0-100%

    WF2_U16 wrecks;
    WF2_U16 frags;
    WF2_U16 assists;

    WF2_S32 score;
    WF2_S32 points;

    WF2_S32 deltaLeader;       // milliseconds

    WF2_U16 lapTiming;

    char reserved[6];
} WF2_ParticipantLeaderboard;

// Participant: Timing
typedef struct
{
    WF2_U32 lapTimeCurrent;    // milliseconds
    WF2_U32 lapTimePenaltyCurrent;
    WF2_U32 lapTimeLast;
    WF2_U32 lapTimeBest;
    WF2_U8 lapBest;

    WF2_S32 deltaAhead;        // milliseconds
    WF2_S32 deltaBehind;

    float lapProgress;         // 0-1

    char reserved[3];
} WF2_ParticipantTiming;

// Participant: Timing Sectors
typedef struct
{
    WF2_U32 sectorTimeCurrentLap1;
    WF2_U32 sectorTimeCurrentLap2;

    WF2_U32 sectorTimeLastLap1;
    WF2_U32 sectorTimeLastLap2;

    WF2_U32 sectorTimeBestLap1;
    WF2_U32 sectorTimeBestLap2;

    WF2_U32 sectorTimeBest1;
    WF2_U32 sectorTimeBest2;
    WF2_U32 sectorTimeBest3;
} WF2_ParticipantTimingSectors;

// Participant: Info
typedef struct
{
    char carId[WF2_CAR_ID_LENGTH_MAX];
    char carName[WF2_CAR_NAME_LENGTH_MAX];
    char playerName[WF2_PLAYER_NAME_LENGTH_MAX];

    WF2_U8 participantIndex;

    WF2_S32 lastNormalTrackStatusTime;  // milliseconds
    WF2_S32 lastCollisionTime;
    WF2_S32 lastResetTime;

    char reserved[16];
} WF2_ParticipantInfo;

// Participant: Damage
typedef struct
{
    WF2_U8 damageStates[WF2_DAMAGE_BYTES_PER_PARTICIPANT];  // bit-packed
} WF2_ParticipantDamage;

// Main Packet - Contains all player telemetry
typedef struct
{
    WF2_PacketHeader header;

    WF2_U16 marshalFlagsPlayer;

    WF2_ParticipantLeaderboard participantPlayerLeaderboard;
    WF2_ParticipantTiming participantPlayerTiming;
    WF2_ParticipantTimingSectors participantPlayerTimingSectors;
    WF2_ParticipantInfo participantPlayerInfo;
    WF2_ParticipantDamage participantPlayerDamage;
    WF2_CarFull carPlayer;
    WF2_Session session;
    WF2_U16 playerStatusFlags;
    WF2_InputExtended inputExtended;

    char reserved[106];
} WF2_PacketMain;

// Restore the original packing alignment
#pragma pack(pop)

#endif // _WRECKFEST2DATA_H
