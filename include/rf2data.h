// mostly lifted from rF2SharedMemoryMapPlugin by TheIronWolfModding
// https://github.com/TheIronWolfModding/rF2SharedMemoryMapPlugin/blob/master/Include/InternalsPlugin.hpp
#pragma once

#ifndef _RF2DATA_H
#define _RF2DATA_H

#include <stdbool.h>
#include <uchar.h>

#pragma pack(push)
#pragma pack(4)
#pragma warning(disable : 4121)   // Alignment sensitivity (ISI sets 4 byte pack)

typedef struct //rF2Vec3
{
  double x;
  double y;
  double z;
} rF2Vec3;

typedef struct //TelemWheelV01
{
  double mSuspensionDeflection;  // meters
  double mRideHeight;            // meters
  double mSuspForce;             // pushrod load in Newtons
  double mBrakeTemp;             // Celsius
  double mBrakePressure;         // currently 0.0-1.0, depending on driver input and brake balance; will convert to true brake pressure (kPa) in future

  double mRotation;              // radians/sec
  double mLateralPatchVel;       // lateral velocity at contact patch
  double mLongitudinalPatchVel;  // longitudinal velocity at contact patch
  double mLateralGroundVel;      // lateral velocity at contact patch
  double mLongitudinalGroundVel; // longitudinal velocity at contact patch
  double mCamber;                // radians (positive is left for left-side wheels, right for right-side wheels)
  double mLateralForce;          // Newtons
  double mLongitudinalForce;     // Newtons
  double mTireLoad;              // Newtons

  double mGripFract;             // an approximation of what fraction of the contact patch is sliding
  double mPressure;              // kPa (tire pressure)
  double mTemperature[3];        // Kelvin (subtract 273.15 to get Celsius), left/center/right (not to be confused with inside/center/outside!)
  double mWear;                  // wear (0.0-1.0, fraction of maximum) ... this is not necessarily proportional with grip loss
  char mTerrainName[16];         // the material prefixes from the TDF file
  unsigned char mSurfaceType;    // 0=dry, 1=wet, 2=grass, 3=dirt, 4=gravel, 5=rumblestrip, 6=special
  bool mFlat;                    // whether tire is flat
  bool mDetached;                // whether wheel is detached
  unsigned char mStaticUndeflectedRadius; // tire radius in centimeters

  double mVerticalTireDeflection;// how much is tire deflected from its (speed-sensitive) radius
  double mWheelYLocation;        // wheel's y location relative to vehicle y location
  double mToe;                   // current toe angle w.r.t. the vehicle

  double mTireCarcassTemperature;       // rough average of temperature samples from carcass (Kelvin)
  double mTireInnerLayerTemperature[3]; // rough average of temperature samples from innermost layer of rubber (before carcass) (Kelvin)

  unsigned char mExpansion[ 24 ];// for future use
} TelemWheelV01;

typedef struct //rF2VehicleTelemetry
{
  int mID;                      // slot ID (note that it can be re-used in multiplayer after someone leaves)
  double mDeltaTime;             // time since last update (seconds)
  double mElapsedTime;           // game session time
  int mLapNumber;               // current lap number
  double mLapStartET;            // time this lap was started
  char mVehicleName[64];         // current vehicle name
  char mTrackName[64];           // current track name

  // Position and derivatives
  rF2Vec3 mPos;               // world position in meters
  rF2Vec3 mLocalVel;          // velocity (meters/sec) in local vehicle coordinates
  rF2Vec3 mLocalAccel;        // acceleration (meters/sec^2) in local vehicle coordinates

  // Orientation and derivatives
  rF2Vec3 mOri[3];            // rows of orientation matrix (use TelemQuat conversions if desired), also converts local
                                 // vehicle vectors into world X, Y, or Z using dot product of rows 0, 1, or 2 respectively
  rF2Vec3 mLocalRot;          // rotation (radians/sec) in local vehicle coordinates
  rF2Vec3 mLocalRotAccel;     // rotational acceleration (radians/sec^2) in local vehicle coordinates

  // Vehicle status
  int mGear;                    // -1=reverse, 0=neutral, 1+=forward gears
  double mEngineRPM;             // engine RPM
  double mEngineWaterTemp;       // Celsius
  double mEngineOilTemp;         // Celsius
  double mClutchRPM;             // clutch RPM

  // Driver input
  double mUnfilteredThrottle;    // ranges  0.0-1.0
  double mUnfilteredBrake;       // ranges  0.0-1.0
  double mUnfilteredSteering;    // ranges -1.0-1.0 (left to right)
  double mUnfilteredClutch;      // ranges  0.0-1.0

  // Filtered input (various adjustments for rev or speed limiting, TC, ABS?, speed sensitive steering, clutch work for semi-automatic shifting, etc.)
  double mFilteredThrottle;      // ranges  0.0-1.0
  double mFilteredBrake;         // ranges  0.0-1.0
  double mFilteredSteering;      // ranges -1.0-1.0 (left to right)
  double mFilteredClutch;        // ranges  0.0-1.0

  // Misc
  double mSteeringShaftTorque;   // torque around steering shaft (used to be mSteeringArmForce, but that is not necessarily accurate for feedback purposes)
  double mFront3rdDeflection;    // deflection at front 3rd spring
  double mRear3rdDeflection;     // deflection at rear 3rd spring

  // Aerodynamics
  double mFrontWingHeight;       // front wing height
  double mFrontRideHeight;       // front ride height
  double mRearRideHeight;        // rear ride height
  double mDrag;                  // drag
  double mFrontDownforce;        // front downforce
  double mRearDownforce;         // rear downforce

  // State/damage info
  double mFuel;                  // amount of fuel (liters)
  double mEngineMaxRPM;          // rev limit
  unsigned char mScheduledStops; // number of scheduled pitstops
  bool  mOverheating;            // whether overheating icon is shown
  bool  mDetached;               // whether any parts (besides wheels) have been detached
  bool  mHeadlights;             // whether headlights are on
  unsigned char mDentSeverity[8];// dent severity at 8 locations around the car (0=none, 1=some, 2=more)
  double mLastImpactET;          // time of last impact
  double mLastImpactMagnitude;   // magnitude of last impact
  rF2Vec3 mLastImpactPos;     // location of last impact

  // Expanded
  double mEngineTorque;          // current engine torque (including additive torque) (used to be mEngineTq, but there's little reason to abbreviate it)
  int mCurrentSector;           // the current sector (zero-based) with the pitlane stored in the sign bit (example: entering pits from third sector gives 0x80000002)
  unsigned char mSpeedLimiter;   // whether speed limiter is on
  unsigned char mMaxGears;       // maximum forward gears
  unsigned char mFrontTireCompoundIndex;   // index within brand
  unsigned char mRearTireCompoundIndex;    // index within brand
  double mFuelCapacity;          // capacity in liters
  unsigned char mFrontFlapActivated;       // whether front flap is activated
  unsigned char mRearFlapActivated;        // whether rear flap is activated
  unsigned char mRearFlapLegalStatus;      // 0=disallowed, 1=criteria detected but not allowed quite yet, 2=allowed
  unsigned char mIgnitionStarter;          // 0=off 1=ignition 2=ignition+starter

  char mFrontTireCompoundName[18];         // name of front tire compound
  char mRearTireCompoundName[18];          // name of rear tire compound

  unsigned char mSpeedLimiterAvailable;    // whether speed limiter is available
  unsigned char mAntiStallActivated;       // whether (hard) anti-stall is activated
  unsigned char mUnused[2];                //
  float mVisualSteeringWheelRange;         // the *visual* steering wheel range

  double mRearBrakeBias;                   // fraction of brakes on rear
  double mTurboBoostPressure;              // current turbo boost pressure if available
  float mPhysicsToGraphicsOffset[3];       // offset from static CG to graphical center
  float mPhysicalSteeringWheelRange;       // the *physical* steering wheel range

  // Future use
  unsigned char mExpansion[152]; // for future use (note that the slot ID has been moved to mID above)

  // keeping this at the end of the structure to make it easier to replace in future versions
  TelemWheelV01 mWheel[4];       // wheel info (front left, front right, rear left, rear right)
} rF2VehicleTelemetry;


typedef struct //rF2ScoringInfo
{
  unsigned char mTrackName[64];
  int mSession;
  double mCurrentET;
  double mEndET;
  int mMaxLaps;
  double mLapDist;
  unsigned char pointer1[8];

  int mNumVehicles;

  // Game phases:
  // 0 Before session has begun
  // 1 Reconnaissance laps (race only)
  // 2 Grid walk-through (race only)
  // 3 Formation lap (race only)
  // 4 Starting-light countdown has begun (race only)
  // 5 Green flag
  // 6 Full course yellow / safety car
  // 7 Session stopped
  // 8 Session over
  char mGamePhase;

  // Yellow flag states (applies to full-course only)
  // -1 Invalid
  //  0 None
  //  1 Pending
  //  2 Pits closed
  //  3 Pit lead lap
  //  4 Pits open
  //  5 Last lap
  //  6 Resume
  //  7 Race halt (not currently used)
  char YellowFlagState;

  char mSectorFlag[3];      // whether there are any local yellows at the moment in each sector (not sure if sector 0 is first or last, so test)
  char mStartLight;       // start light frame (number depends on track)
  char mNumRedLights;     // number of red lights in start sequence
  char mInRealtime;                // in realtime as opposed to at the monitor

  char mPlayerName[32];            // player name (including possible multiplayer override)
  char mPlrFileName[64];           // may be encoded to be a legal filename

  // weather
  double mDarkCloud;               // cloud darkness? 0.0-1.0
  double mRaining;                 // raining severity 0.0-1.0
  double mAmbientTemp;             // temperature (Celsius)
  double mTrackTemp;               // temperature (Celsius)
  rF2Vec3 mWind;                // wind speed
  double mMinPathWetness;          // minimum wetness on main path 0.0-1.0
  double mMaxPathWetness;          // maximum wetness on main path 0.0-1.0

  //char mGameMode;                  // 1 = server, 2 = client, 3 = server and client
  //char mIsPasswordProtected;       // is the server password protected
  //char mServerPort;              // the port of the server (if on a server)
  //int mServerPublicIP;            // the public IP address of the server (if on a server)
  //int mMaxPlayers;                 // maximum number of vehicles that can be in the session
  //char mServerName[32];              // name of the server
  //float mStartET;                  // start time (seconds since midnight) of the event
  //double mAvgPathWetness;

  // Future use
  char mExpansion[256];

  // MM_NOT_USED
  // keeping this at the end of the structure to make it easier to replace in future versions
  // VehicleScoringInfoV01 *mVehicle; // array of vehicle scoring info's
  // MM_NEW
  char pointer2[8];
} rF2ScoringInfo;

typedef struct //rF2VehicleScoring
{
  int mID;                      // slot ID (note that it can be re-used in multiplayer after someone leaves)
  char mDriverName[32];          // driver name
  char mVehicleName[64];         // vehicle name
  short mTotalLaps;              // laps completed
  signed char mSector;           // 0=sector3, 1=sector1, 2=sector2 (don't ask why)
  signed char mFinishStatus;     // 0=none, 1=finished, 2=dnf, 3=dq
  double mLapDist;               // current distance around track
  double mPathLateral;           // lateral position with respect to *very approximate* "center" path
  double mTrackEdge;             // track edge (w.r.t. "center" path) on same side of track as vehicle

  double mBestSector1;           // best sector 1
  double mBestSector2;           // best sector 2 (plus sector 1)
  double mBestLapTime;           // best lap time
  double mLastSector1;           // last sector 1
  double mLastSector2;           // last sector 2 (plus sector 1)
  double mLastLapTime;           // last lap time
  double mCurSector1;            // current sector 1 if valid
  double mCurSector2;            // current sector 2 (plus sector 1) if valid
  // no current laptime because it instantly becomes "last"

  short mNumPitstops;            // number of pitstops made
  short mNumPenalties;           // number of outstanding penalties
  bool mIsPlayer;                // is this the player's vehicle

  signed char mControl;          // who's in control: -1=nobody (shouldn't get this), 0=local player, 1=local AI, 2=remote, 3=replay (shouldn't get this)
  bool mInPits;                  // between pit entrance and pit exit (not always accurate for remote vehicles)
  unsigned char mPlace;          // 1-based position
  char mVehicleClass[32];        // vehicle class

  // Dash Indicators
  double mTimeBehindNext;        // time behind vehicle in next higher place
  int mLapsBehindNext;          // laps behind vehicle in next higher place
  double mTimeBehindLeader;      // time behind leader
  int mLapsBehindLeader;        // laps behind leader
  double mLapStartET;            // time this lap was started

  // Position and derivatives
  rF2Vec3 mPos;               // world position in meters
  rF2Vec3 mLocalVel;          // velocity (meters/sec) in local vehicle coordinates
  rF2Vec3 mLocalAccel;        // acceleration (meters/sec^2) in local vehicle coordinates

  // Orientation and derivatives
  rF2Vec3 mOri[3];            // rows of orientation matrix (use TelemQuat conversions if desired), also converts local
                                 // vehicle vectors into world X, Y, or Z using dot product of rows 0, 1, or 2 respectively
  rF2Vec3 mLocalRot;          // rotation (radians/sec) in local vehicle coordinates
  rF2Vec3 mLocalRotAccel;     // rotational acceleration (radians/sec^2) in local vehicle coordinates

  // tag.2012.03.01 - stopped casting some of these so variables now have names and mExpansion has shrunk, overall size and old data locations should be same
  unsigned char mHeadlights;     // status of headlights
  unsigned char mPitState;       // 0=none, 1=request, 2=entering, 3=stopped, 4=exiting
  unsigned char mServerScored;   // whether this vehicle is being scored by server (could be off in qualifying or racing heats)
  unsigned char mIndividualPhase; // game phases (described below) plus 9=after formation, 10=under yellow, 11=under blue (not used)

  int mQualification;           // 1-based, can be -1 when invalid

  double mTimeIntoLap;           // estimated time into lap
  double mEstimatedLapTime;      // estimated laptime used for 'time behind' and 'time into lap' (note: this may changed based on vehicle and setup!?)

  char mPitGroup[24];            // pit group (same as team name unless pit is shared)
  unsigned char mFlag;           // primary flag being shown to vehicle (currently only 0=green or 6=blue)
  bool mUnderYellow;             // whether this car has taken a full-course caution flag at the start/finish line
  unsigned char mCountLapFlag;   // 0 = do not count lap or time, 1 = count lap but not time, 2 = count lap and time
  bool mInGarageStall;           // appears to be within the correct garage stall

  unsigned char mUpgradePack[16];  // Coded upgrades
  float mPitLapDist;             // location of pit in terms of lap distance

  float mBestLapSector1;         // sector 1 time from best lap (not necessarily the best sector 1 time)
  float mBestLapSector2;         // sector 2 time from best lap (not necessarily the best sector 2 time)

  // Future use
  // tag.2012.04.06 - SEE ABOVE!
  unsigned char mExpansion[48];  // for future use
} rF2VehicleScoring;

struct rF2Telemetry
{
  int mVersionUpdateBegin;
  int mVersionUpdateEnd;
  int mBytesUpdatedHint;

  int mNumVehicles;

  rF2VehicleTelemetry mVehicles[64];
};

struct rF2Scoring
{
  int mVersionUpdateBegin;
  int mVersionUpdateEnd;
  int mBytesUpdatedHint;

  rF2ScoringInfo mScoringInfo;

  rF2VehicleScoring mVehicles[64];
};

#pragma pack(pop)
#endif
