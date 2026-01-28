#pragma once
#pragma pack(push, 1)

#include <stdint.h>

typedef struct
{
    // = 1 when race is on. = 0 when in menus/race stopped …
    int32_t IsRaceOn;

    // Can overflow to 0 eventually
    uint32_t TimestampMS;

    float EngineMaxRpm;
    float EngineIdleRpm;
    float CurrentEngineRpm;

    // In the car's local space; X = right, Y = up, Z = forward
    float AccelerationX;
    float AccelerationY;
    float AccelerationZ;

    float VelocityX;
    float VelocityY;
    float VelocityZ;

    // In the car's local space; X = pitch, Y = yaw, Z = roll
    float AngularVelocityX;
    float AngularVelocityY;
    float AngularVelocityZ;

    float Yaw;
    float Pitch;
    float Roll;

    // Suspension travel normalized
    float NormalizedSuspensionTravelFrontLeft;
    float NormalizedSuspensionTravelFrontRight;
    float NormalizedSuspensionTravelRearLeft;
    float NormalizedSuspensionTravelRearRight;

    // Tire normalized slip ratio
    float TireSlipRatioFrontLeft;
    float TireSlipRatioFrontRight;
    float TireSlipRatioRearLeft;
    float TireSlipRatioRearRight;

    // Wheel rotation speed (rad/s)
    float WheelRotationSpeedFrontLeft;
    float WheelRotationSpeedFrontRight;
    float WheelRotationSpeedRearLeft;
    float WheelRotationSpeedRearRight;

    // = 1 when wheel is on rumble strip, = 0 when off
    int32_t WheelOnRumbleStripFrontLeft;
    int32_t WheelOnRumbleStripFrontRight;
    int32_t WheelOnRumbleStripRearLeft;
    int32_t WheelOnRumbleStripRearRight;

    // Puddle depth (0–1)
    float WheelInPuddleDepthFrontLeft;
    float WheelInPuddleDepthFrontRight;
    float WheelInPuddleDepthRearLeft;
    float WheelInPuddleDepthRearRight;

    // Surface rumble
    float SurfaceRumbleFrontLeft;
    float SurfaceRumbleFrontRight;
    float SurfaceRumbleRearLeft;
    float SurfaceRumbleRearRight;

    // Tire normalized slip angle
    float TireSlipAngleFrontLeft;
    float TireSlipAngleFrontRight;
    float TireSlipAngleRearLeft;
    float TireSlipAngleRearRight;

    // Tire normalized combined slip
    float TireCombinedSlipFrontLeft;
    float TireCombinedSlipFrontRight;
    float TireCombinedSlipRearLeft;
    float TireCombinedSlipRearRight;

    // Suspension travel in meters
    float SuspensionTravelMetersFrontLeft;
    float SuspensionTravelMetersFrontRight;
    float SuspensionTravelMetersRearLeft;
    float SuspensionTravelMetersRearRight;

    // Vehicle info
    int32_t CarOrdinal;
    int32_t CarClass;            // 0–7
    int32_t CarPerformanceIndex; // 100–999
    int32_t DrivetrainType;      // 0=FWD, 1=RWD, 2=AWD
    int32_t NumCylinders;

} Forza_Sled;

typedef struct
{
    int32_t IsRaceOn;
    uint32_t TimestampMS;

    float EngineMaxRpm;
    float EngineIdleRpm;
    float CurrentEngineRpm;

    float AccelerationX;
    float AccelerationY;
    float AccelerationZ;

    float VelocityX;
    float VelocityY;
    float VelocityZ;

    float AngularVelocityX;
    float AngularVelocityY;
    float AngularVelocityZ;

    float Yaw;
    float Pitch;
    float Roll;

    float NormalizedSuspensionTravelFrontLeft;
    float NormalizedSuspensionTravelFrontRight;
    float NormalizedSuspensionTravelRearLeft;
    float NormalizedSuspensionTravelRearRight;

    float TireSlipRatioFrontLeft;
    float TireSlipRatioFrontRight;
    float TireSlipRatioRearLeft;
    float TireSlipRatioRearRight;

    float WheelRotationSpeedFrontLeft;
    float WheelRotationSpeedFrontRight;
    float WheelRotationSpeedRearLeft;
    float WheelRotationSpeedRearRight;

    int32_t WheelOnRumbleStripFrontLeft;
    int32_t WheelOnRumbleStripFrontRight;
    int32_t WheelOnRumbleStripRearLeft;
    int32_t WheelOnRumbleStripRearRight;

    float WheelInPuddleDepthFrontLeft;
    float WheelInPuddleDepthFrontRight;
    float WheelInPuddleDepthRearLeft;
    float WheelInPuddleDepthRearRight;

    float SurfaceRumbleFrontLeft;
    float SurfaceRumbleFrontRight;
    float SurfaceRumbleRearLeft;
    float SurfaceRumbleRearRight;

    float TireSlipAngleFrontLeft;
    float TireSlipAngleFrontRight;
    float TireSlipAngleRearLeft;
    float TireSlipAngleRearRight;

    float TireCombinedSlipFrontLeft;
    float TireCombinedSlipFrontRight;
    float TireCombinedSlipRearLeft;
    float TireCombinedSlipRearRight;

    float SuspensionTravelMetersFrontLeft;
    float SuspensionTravelMetersFrontRight;
    float SuspensionTravelMetersRearLeft;
    float SuspensionTravelMetersRearRight;

    int32_t CarOrdinal;
    int32_t CarClass;
    int32_t CarPerformanceIndex;
    int32_t DrivetrainType;
    int32_t NumCylinders;

    // Dash-only fields
    float PositionX;
    float PositionY;
    float PositionZ;
    float Speed;
    float Power;
    float Torque;

    float TireTempFrontLeft;
    float TireTempFrontRight;
    float TireTempRearLeft;
    float TireTempRearRight;

    float Boost;
    float Fuel;
    float DistanceTraveled;

    float BestLap;
    float LastLap;
    float CurrentLap;
    float CurrentRaceTime;

    uint16_t LapNumber;
    uint8_t  RacePosition;

    uint8_t  Accel;
    uint8_t  Brake;
    uint8_t  Clutch;
    uint8_t  HandBrake;
    uint8_t  Gear;

    int8_t   Steer;
    int8_t   NormalizedDrivingLine;
    int8_t   NormalizedAIBrakeDifference;

    float TireWearFrontLeft;
    float TireWearFrontRight;
    float TireWearRearLeft;
    float TireWearRearRight;

    int32_t TrackOrdinal;

} Forza_Dash;

#pragma pack(pop)
