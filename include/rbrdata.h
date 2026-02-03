// vim: set ts=4 :
///////////////////////////////////////////////////////////
//  rbr.telemetry.data.TelemetryData.h
//  Implementation of the Class TireSegment
//  Created on:      28-Dez-2019 07:49:08
//  Original author: Guenter Schlupf
///////////////////////////////////////////////////////////
#ifndef _RBRDATA_H
#define _RBRDATA_H

#include <stdint.h>

#pragma pack(push, 1)

typedef struct
{
    float temperature_;
    float wear_;
} RBR_TireSegment;

typedef struct
{
    float pressure_;
    float temperature_;
    float carcassTemperature_;
    float treadTemperature_;
    unsigned int currentSegment_;
    RBR_TireSegment segment1_;
    RBR_TireSegment segment2_;
    RBR_TireSegment segment3_;
    RBR_TireSegment segment4_;
    RBR_TireSegment segment5_;
    RBR_TireSegment segment6_;
    RBR_TireSegment segment7_;
    RBR_TireSegment segment8_;
} RBR_Tire;

typedef struct
{
    float layerTemperature_;
    float temperature_;
    float wear_;
} RBR_BrakeDisk;

typedef struct
{
    RBR_BrakeDisk brakeDisk_;
    RBR_Tire tire_;
} RBR_Wheel;

typedef struct
{
    float damage_;
    float pistonVelocity_;
} RBR_Damper;

typedef struct
{
    float springDeflection_;
    float rollbarForce_;
    float springForce_;
    float damperForce_;
    float strutForce_;
    int helperSpringIsActive_;
    RBR_Damper damper_;
    RBR_Wheel wheel_;
} RBR_Suspension;

typedef struct
{
    float rpm_;
    float radiatorCoolantTemperature_;
    float engineCoolantTemperature_;
    float engineTemperature_;
} RBR_Engine;

typedef struct
{
    /// Forward/backward.
    float surge_;
    /// Left/right.
    float sway_;
    /// Up/down.
    float heave_;
    /// Rotation about longitudinal axis.
    float roll_;
    /// Rotation about transverse axis.
    float pitch_;
    /// Rotation about normal axis.
    float yaw_;
} RBR_Motion;

typedef struct
{
    int index_;
    /// Speed of the car in kph or mph.
    float speed_;
    float positionX_;
    float positionY_;
    float positionZ_;
    float roll_;
    float pitch_;
    float yaw_;
    RBR_Motion velocities_;
    RBR_Motion accelerations_;
    RBR_Engine engine_;
    /// Suspension data: LF, RF, LB, RB.
    RBR_Suspension suspensionLF_;
    /// Suspension data: LF, RF, LB, RB.
    RBR_Suspension suspensionRF_;
    /// Suspension data: LF, RF, LB, RB.
    RBR_Suspension suspensionLB_;
    /// Suspension data: LF, RF, LB, RB.
    RBR_Suspension suspensionRB_;
} RBR_Car;

typedef struct
{
    float steering_;
    float throttle_;
    float brake_;
    float handbrake_;
    float clutch_;
    int gear_;
    float footbrakePressure_;
    float handbrakePressure_;
} RBR_Control;

typedef struct
{
    int index_;
    /// The position on the driveline.
    float progress_;
    /// The total race time.
    float raceTime_;
    float driveLineLocation_;
    float distanceToEnd_;
} RBR_Stage;

typedef struct
{
    unsigned int totalSteps_;
    RBR_Stage stage_;
    RBR_Control control_;
    RBR_Car car_;
} RBR_TelemetryData;

#pragma pack(pop)

#endif // _RBRDATA_H
