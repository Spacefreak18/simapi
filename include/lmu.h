#pragma once
#pragma pack(push, 4)

#include <stdint.h>
#include <stdbool.h>

/* =========================
   Constants
   ========================= */

#define LMU_MAX_MAPPED_VEHICLES 104
#define LMU_MAX_PATH_LENGTH     260

/* =========================
   InternalsPlugin
   ========================= */

typedef struct
{
    double x;
    double y;
    double z;
} LMUVect3;

typedef struct
{
    double   mSuspensionDeflection;
    double   mRideHeight;
    double   mSuspForce;
    double   mBrakeTemp;
    double   mBrakePressure;
    double   mRotation;
    double   mLateralPatchVel;
    double   mLongitudinalPatchVel;
    double   mLateralGroundVel;
    double   mLongitudinalGroundVel;
    double   mCamber;
    double   mLateralForce;
    double   mLongitudinalForce;
    double   mTireLoad;
    double   mGripFract;
    double   mPressure;
    double   mTemperature[3];
    double   mWear;
    char     mTerrainName[16];
    uint8_t  mSurfaceType;
    _Bool    mFlat;
    _Bool    mDetached;
    uint8_t  mStaticUndeflectedRadius;
    double   mVerticalTireDeflection;
    double   mWheelYLocation;
    double   mToe;
    double   mTireCarcassTemperature;
    double   mTireInnerLayerTemperature[3];
    uint8_t  mExpansion[24];
} LMUWheel;

typedef struct
{
    int32_t  mID;
    double   mDeltaTime;
    double   mElapsedTime;
    int32_t  mLapNumber;
    double   mLapStartET;
    char     mVehicleName[64];
    char     mTrackName[64];
    LMUVect3 mPos;
    LMUVect3 mLocalVel;
    LMUVect3 mLocalAccel;
    LMUVect3 mOri[3];
    LMUVect3 mLocalRot;
    LMUVect3 mLocalRotAccel;
    int32_t  mGear;
    double   mEngineRPM;
    double   mEngineWaterTemp;
    double   mEngineOilTemp;
    double   mClutchRPM;
    double   mUnfilteredThrottle;
    double   mUnfilteredBrake;
    double   mUnfilteredSteering;
    double   mUnfilteredClutch;
    double   mFilteredThrottle;
    double   mFilteredBrake;
    double   mFilteredSteering;
    double   mFilteredClutch;
    double   mSteeringShaftTorque;
    double   mFront3rdDeflection;
    double   mRear3rdDeflection;
    double   mFrontWingHeight;
    double   mFrontRideHeight;
    double   mRearRideHeight;
    double   mDrag;
    double   mFrontDownforce;
    double   mRearDownforce;
    double   mFuel;
    double   mEngineMaxRPM;
    uint8_t  mScheduledStops;
    _Bool    mOverheating;
    _Bool    mDetached;
    _Bool    mHeadlights;
    uint8_t  mDentSeverity[8];
    double   mLastImpactET;
    double   mLastImpactMagnitude;
    LMUVect3 mLastImpactPos;
    double   mEngineTorque;
    int32_t  mCurrentSector;
    uint8_t  mSpeedLimiter;
    uint8_t  mMaxGears;
    uint8_t  mFrontTireCompoundIndex;
    uint8_t  mRearTireCompoundIndex;
    double   mFuelCapacity;
    uint8_t  mFrontFlapActivated;
    uint8_t  mRearFlapActivated;
    uint8_t  mRearFlapLegalStatus;
    uint8_t  mIgnitionStarter;
    char     mFrontTireCompoundName[18];
    char     mRearTireCompoundName[18];
    uint8_t  mSpeedLimiterAvailable;
    uint8_t  mAntiStallActivated;
    uint8_t  mUnused[2];
    float    mVisualSteeringWheelRange;
    double   mRearBrakeBias;
    double   mTurboBoostPressure;
    float    mPhysicsToGraphicsOffset[3];
    float    mPhysicalSteeringWheelRange;
    double   mDeltaBest;
    double   mBatteryChargeFraction;
    double   mElectricBoostMotorTorque;
    double   mElectricBoostMotorRPM;
    double   mElectricBoostMotorTemperature;
    double   mElectricBoostWaterTemperature;
    uint8_t  mElectricBoostMotorState;
    uint8_t  mExpansion[103];
    LMUWheel mWheels[4];
} LMUVehicleTelemetry;

/* =========================
   Scoring
   ========================= */

typedef struct
{
    int32_t  mID;
    char     mDriverName[32];
    char     mVehicleName[64];
    int16_t  mTotalLaps;
    int8_t   mSector;
    int8_t   mFinishStatus;
    double   mLapDist;
    double   mPathLateral;
    double   mTrackEdge;
    double   mBestSector1;
    double   mBestSector2;
    double   mBestLapTime;
    double   mLastSector1;
    double   mLastSector2;
    double   mLastLapTime;
    double   mCurSector1;
    double   mCurSector2;
    int16_t  mNumPitstops;
    int16_t  mNumPenalties;
    _Bool    mIsPlayer;
    int8_t   mControl;
    _Bool    mInPits;
    uint8_t  mPlace;
    char     mVehicleClass[32];
    double   mTimeBehindNext;
    int32_t  mLapsBehindNext;
    double   mTimeBehindLeader;
    int32_t  mLapsBehindLeader;
    double   mLapStartET;
    LMUVect3 mPos;
    LMUVect3 mLocalVel;
    LMUVect3 mLocalAccel;
    LMUVect3 mOri[3];
    LMUVect3 mLocalRot;
    LMUVect3 mLocalRotAccel;
    uint8_t  mHeadlights;
    uint8_t  mPitState;
    uint8_t  mServerScored;
    uint8_t  mIndividualPhase;
    int32_t  mQualification;
    double   mTimeIntoLap;
    double   mEstimatedLapTime;
    char     mPitGroup[24];
    uint8_t  mFlag;
    _Bool    mUnderYellow;
    uint8_t  mCountLapFlag;
    _Bool    mInGarageStall;
    uint8_t  mUpgradePack[16];
    float    mPitLapDist;
    float    mBestLapSector1;
    float    mBestLapSector2;
    uint64_t mSteamID;
    char     mVehFilename[32];
    int16_t  mAttackMode;
    uint8_t  mFuelFraction;
    _Bool    mDRSState;
    uint8_t  mExpansion[4];
} LMUVehicleScoring;

/* =========================
   Top-level Shared Memory
   ========================= */

typedef struct
{
    char     mTrackName[64];
    int32_t  mSession;
    double   mCurrentET;
    double   mEndET;
    int32_t  mMaxLaps;
    double   mLapDist;
    uint8_t  mResultsStreamPointer[8];
    int32_t  mNumVehicles;
    uint8_t  mGamePhase;
    int8_t   mYellowFlagState;
    uint8_t  mSectorFlag[3];
    uint8_t  mStartLight;
    uint8_t  mNumRedLights;
    _Bool    mInRealtime;
    char     mPlayerName[32];
    char     mPlrFileName[64];
    double   mDarkCloud;
    double   mRaining;
    double   mAmbientTemp;
    double   mTrackTemp;
    LMUVect3 mWind;
    double   mMinPathWetness;
    double   mMaxPathWetness;
    uint8_t  mGameMode;
    _Bool    mIsPasswordProtected;
    uint16_t mServerPort;
    uint32_t mServerPublicIP;
    int32_t  mMaxPlayers;
    char     mServerName[32];
    float    mStartET;
    double   mAvgPathWetness;
    uint8_t  mExpansion[200];
    uint8_t  mVehiclePointer[8];
} LMUScoringInfo;

typedef struct
{
    uint64_t mAppWindow;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mRefreshRate;
    uint32_t mWindowed;
    uint8_t  mOptionsLocation;
    char     mOptionsPage[31];
    uint8_t  mExpansion[204];
} LMUApplicationState;

/* =========================
   Shared Memory Root
   ========================= */

typedef struct
{
    LMUScoringInfo      scoringInfo;
    uint8_t             scoringStreamSize[12];
    LMUVehicleScoring   vehScoringInfo[LMU_MAX_MAPPED_VEHICLES];
    char                scoringStream[65536];
} LMUScoringData;

typedef struct
{
    uint8_t               activeVehicles;
    uint8_t               playerVehicleIdx;
    _Bool                 playerHasVehicle;
    LMUVehicleTelemetry   telemInfo[LMU_MAX_MAPPED_VEHICLES];
} LMUTelemetryData;

typedef struct
{
    char userData[LMU_MAX_PATH_LENGTH];
    char customVariables[LMU_MAX_PATH_LENGTH];
    char stewardResults[LMU_MAX_PATH_LENGTH];
    char playerProfile[LMU_MAX_PATH_LENGTH];
    char pluginsFolder[LMU_MAX_PATH_LENGTH];
} LMUPathData;

typedef struct
{
    struct {
        uint32_t SME_ENTER;
        uint32_t SME_EXIT;
        uint32_t SME_STARTUP;
        uint32_t SME_SHUTDOWN;
        uint32_t SME_LOAD;
        uint32_t SME_UNLOAD;
        uint32_t SME_START_SESSION;
        uint32_t SME_END_SESSION;
        uint32_t SME_ENTER_REALTIME;
        uint32_t SME_EXIT_REALTIME;
        uint32_t SME_UPDATE_SCORING;
        uint32_t SME_UPDATE_TELEMETRY;
        uint32_t SME_INIT_APPLICATION;
        uint32_t SME_UNINIT_APPLICATION;
        uint32_t SME_SET_ENVIRONMENT;
        uint32_t SME_FFB;
    } events;

    int32_t             gameVersion;
    float               FFBTorque;
    LMUApplicationState appInfo;
} LMUGeneric;

struct LMUObject
{
    LMUGeneric       generic;
    LMUPathData      paths;
    LMUScoringData   scoring;
    LMUTelemetryData telemetry;
};


#pragma pack(pop)
