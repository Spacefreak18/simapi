#pragma once

#include <stdint.h>


#define R3E_SHARED_MEMORY_NAME "$R3E"

enum
{
    // Major version number to test against
    R3E_VERSION_MAJOR = 3
};

enum
{
    // Minor version number to test against
    R3E_VERSION_MINOR = 4
};

enum
{
    R3E_NUM_DRIVERS_MAX = 128
};

typedef enum
{
	R3E_GAMEMODE_UNAVAILABLE = -1,
	R3E_GAMEMODE_TRACKTEST = 0,
	R3E_GAMEMODE_LEADERBOARDCHALLENGE = 1,
	R3E_GAMEMODE_COMPETITION = 2,
	R3E_GAMEMODE_SINGLERACE = 3,
	R3E_GAMEMODE_CHAMPIONSHIP = 4,
	R3E_GAMEMODE_MULTIPLAYER = 5,
	R3E_GAMEMODE_MULTIPLAYERRANKED = 6, // not impl currently
	R3E_GAMEMODE_TRYBEFOREYOUBUY = 7,
} r3e_gamemode;

typedef enum
{
    R3E_SESSION_UNAVAILABLE = -1,
    R3E_SESSION_PRACTICE = 0,
    R3E_SESSION_QUALIFY = 1,
    R3E_SESSION_RACE = 2,
	R3E_SESSION_WARMUP = 3,
} r3e_session;

typedef enum
{
    R3E_SESSION_PHASE_UNAVAILABLE = -1,

    // Currently in garage
    R3E_SESSION_PHASE_GARAGE = 1,

    // Gridwalk or track walkthrough
    R3E_SESSION_PHASE_GRIDWALK = 2,

    // Formation lap, rolling start etc.
    R3E_SESSION_PHASE_FORMATION = 3,

    // Countdown to race is ongoing
    R3E_SESSION_PHASE_COUNTDOWN = 4,

    // Race is ongoing
    R3E_SESSION_PHASE_GREEN = 5,

    // End of session
    R3E_SESSION_PHASE_CHECKERED = 6,
} r3e_session_phase;

typedef enum
{
    R3E_CONTROL_UNAVAILABLE = -1,

    // Controlled by the actual player
    R3E_CONTROL_PLAYER = 0,

    // Controlled by AI
    R3E_CONTROL_AI = 1,

    // Controlled by a network entity of some sort
    R3E_CONTROL_REMOTE = 2,

    // Controlled by a replay or ghost
    R3E_CONTROL_REPLAY = 3,
} r3e_control;

typedef enum
{
    R3E_PIT_WINDOW_UNAVAILABLE = -1,

    // Pit stops are not enabled for this session
    R3E_PIT_WINDOW_DISABLED = 0,

    // Pit stops are enabled, but you're not allowed to perform one right now
    R3E_PIT_WINDOW_CLOSED = 1,

    // Allowed to perform a pit stop now
    R3E_PIT_WINDOW_OPEN = 2,

    // Currently performing the pit stop changes (changing driver, etc.)
    R3E_PIT_WINDOW_STOPPED = 3,

    // After the current mandatory pitstop have been completed
    R3E_PIT_WINDOW_COMPLETED = 4,
} r3e_pit_window;

typedef enum
{	
    // Pit menu unavailable
    R3E_PIT_MENU_UNAVAILABLE = -1,

    // Pit menu preset
    R3E_PIT_MENU_PRESET = 0,

    // Pit menu actions
    R3E_PIT_MENU_PENALTY = 1,
    R3E_PIT_MENU_DRIVERCHANGE = 2,
    R3E_PIT_MENU_FUEL = 3,
    R3E_PIT_MENU_FRONTTIRES = 4,
    R3E_PIT_MENU_REARTIRES = 5,
    R3E_PIT_MENU_BODY = 6,
    R3E_PIT_MENU_FRONTWING = 7,
    R3E_PIT_MENU_REARWING = 8,
    R3E_PIT_MENU_SUSPENSION = 9,
	
    // Pit menu buttons
    R3E_PIT_MENU_BUTTON_TOP = 10,
    R3E_PIT_MENU_BUTTON_BOTTOM = 11,
	
    // Pit menu nothing selected
    R3E_PIT_MENU_MAX = 12,
} r3e_pit_menu_selection;

typedef enum
{
    R3E_TIRE_TYPE_UNAVAILABLE = -1,
    R3E_TIRE_TYPE_OPTION = 0,
    R3E_TIRE_TYPE_PRIME = 1,
} r3e_tire_type;

typedef enum
{
	R3E_TIRE_SUBTYPE_UNAVAILABLE = -1,
	R3E_TIRE_SUBTYPE_PRIMARY = 0,
	R3E_TIRE_SUBTYPE_ALTERNATE = 1,
	R3E_TIRE_SUBTYPE_SOFT = 2,
	R3E_TIRE_SUBTYPE_MEDIUM = 3,
	R3E_TIRE_SUBTYPE_HARD = 4,
} r3e_tire_subtype;

typedef enum
{
	R3E_MTRL_TYPE_UNAVAILABLE = -1,
	R3E_MTRL_TYPE_NONE = 0,
	R3E_MTRL_TYPE_TARMAC = 1,
	R3E_MTRL_TYPE_GRASS = 2,
	R3E_MTRL_TYPE_DIRT = 3,
	R3E_MTRL_TYPE_GRAVEL = 4,
	R3E_MTRL_TYPE_RUMBLE = 5,
} r3e_mtrl_type;

typedef enum
{
    // No mandatory pitstops
    R3E_PITSTOP_STATUS_UNAVAILABLE = -1,

    // Mandatory pitstop for two tyres not served yet
    R3E_PITSTOP_STATUS_TWO_TYRES_UNSERVED = 0,

    // Mandatory pitstop for four tyres not served yet
	R3E_PITSTOP_STATUS_FOUR_TYRES_UNSERVED = 1,

    // Mandatory pitstop served
    R3E_PITSTOP_STATUS_SERVED = 2,
} r3e_pitstop_status;

typedef enum
{
    R3E_TIRE_FRONT_LEFT = 0,
    R3E_TIRE_FRONT_RIGHT = 1,
    R3E_TIRE_REAR_LEFT = 2,
    R3E_TIRE_REAR_RIGHT = 3,
    R3E_TIRE_INDEX_MAX = 4,
} r3e_tire_index_enum;

typedef enum
{
    R3E_TIRE_TEMP_LEFT = 0,
    R3E_TIRE_TEMP_CENTER = 1,
    R3E_TIRE_TEMP_RIGHT = 2,
    R3E_TIRE_TEMP_INDEX_MAX = 3,
} r3e_tire_temp_enum;

typedef enum
{
    R3E_ENGINE_TYPE_COMBUSTION = 0,
    R3E_ENGINE_TYPE_ELECTRIC = 1,
    R3E_ENGINE_TYPE_HYBRID = 2,
    R3E_ENGINE_TYPE_INDEX_MAX = 3,
} r3e_engine_type_enum;

typedef enum
{
    // N/A
    R3E_FINISH_STATUS_UNAVAILABLE = -1,

    // Still on track, not finished
    R3E_FINISH_STATUS_NONE = 0,

    // Finished session normally
    R3E_FINISH_STATUS_FINISHED = 1,

    // Did not finish
    R3E_FINISH_STATUS_DNF = 2,

    // Did not qualify
    R3E_FINISH_STATUS_DNQ = 3,

    // Did not start
    R3E_FINISH_STATUS_DNS = 4,

    // Disqualified
    R3E_FINISH_STATUS_DQ = 5,
} r3e_finish_status;

typedef enum
{
    // N/A
    R3E_SESSION_LENGTH_UNAVAILABLE = -1,

    R3E_SESSION_LENGTH_TIME_BASED = 0,

    R3E_SESSION_LENGTH_LAP_BASED = 1,

    // Time and lap based session means there will be an extra lap after the time has run out
    R3E_SESSION_LENGTH_TIME_AND_LAP_BASED = 2,
} r3e_session_length_format;

// Make sure everything is tightly packed, to prevent the compiler from adding any hidden padding
#pragma pack(push, 1)

typedef struct
{
    float x;
    float y;
    float z;
} r3e_vec3_f32;

typedef struct
{
    double x;
    double y;
    double z;
} r3e_vec3_f64;

typedef struct
{
    float pitch;
    float yaw;
    float roll;
} r3e_ori_f32;

typedef struct
{
    float sector1;
    float sector2;
    float sector3;
} r3e_sectorStarts;

// High precision data for player's vehicle only
typedef struct
{
	// Player user id
	int32_t user_id;

    // Virtual physics time
    // Unit: Ticks (1 tick = 1/400th of a second)
    int32_t game_simulation_ticks;

    // Virtual physics time
    // Unit: Seconds
    double game_simulation_time;

    // Car world-space position
    r3e_vec3_f64 position;

    // Car world-space velocity
    // Unit: Meter per second (m/s)
    r3e_vec3_f64 velocity;

    // Car local-space velocity
    // Unit: Meter per second (m/s)
    r3e_vec3_f64 local_velocity;

    // Car world-space acceleration
    // Unit: Meter per second squared (m/s^2)
    r3e_vec3_f64 acceleration;

    // Car local-space acceleration
    // Unit: Meter per second squared (m/s^2)
    r3e_vec3_f64 local_acceleration;

    // Car body orientation
    // Unit: Euler angles
    r3e_vec3_f64 orientation;

    // Car body rotation
    r3e_vec3_f64 rotation;

    // Car body angular acceleration (torque divided by inertia)
    r3e_vec3_f64 angular_acceleration;

    // Car world-space angular velocity
    // Unit: Radians per second
    r3e_vec3_f64 angular_velocity;

    // Car local-space angular velocity
    // Unit: Radians per second
    r3e_vec3_f64 local_angular_velocity;

    // Driver g-force local to car
    r3e_vec3_f64 local_g_force;

    // Total steering force coming through steering bars
    double steering_force;
    double steering_force_percentage;
	
    // Current engine torque
    double engine_torque;

    // Current downforce
    // Unit: Newtons (N)
    double current_downforce;
	
    // Currently unused
    double voltage;
    double ers_level;
    double power_mgu_h;
    double power_mgu_k;
    double torque_mgu_k;

    // Car setup (radians, meters, meters per second)
    double suspension_deflection[R3E_TIRE_INDEX_MAX];
    double suspension_velocity[R3E_TIRE_INDEX_MAX];
    double camber[R3E_TIRE_INDEX_MAX];
    double ride_height[R3E_TIRE_INDEX_MAX];
    double front_wing_height;
    double front_roll_angle;
    double rear_roll_angle;
    double third_spring_suspension_deflection_front;
    double third_spring_suspension_velocity_front;
    double third_spring_suspension_deflection_rear;
    double third_spring_suspension_velocity_rear;

    // Reserved data
	double unused1;
	double unused2;
	double unused3;
} r3e_playerdata;

typedef struct
{
    // Whether yellow flag is currently active
    // -1 = no data
    //  0 = not active
    //  1 = active
    int32_t yellow;

    // Whether yellow flag was caused by current slot
    // -1 = no data
    //  0 = didn't cause it
    //  1 = caused it
    int32_t yellowCausedIt;

    // Whether overtake of car in front by current slot is allowed under yellow flag
    // -1 = no data
    //  0 = not allowed
    //  1 = allowed
    int32_t yellowOvertake;

    // Whether you have gained positions illegaly under yellow flag to give back
    // -1 = no data
    //  0 = no positions gained
    //  n = number of positions gained
    int32_t yellowPositionsGained;
	
    // Yellow flag for each sector; -1 = no data, 0 = not active, 1 = active
    int32_t sector_yellow[3];

    // Distance into track for closest yellow, -1.0 if no yellow flag exists
    // Unit: Meters (m)
    float closest_yellow_distance_into_track;

    // Whether blue flag is currently active
    // -1 = no data
    //  0 = not active
    //  1 = active
    int32_t blue;

    // Whether black flag is currently active
    // -1 = no data
    //  0 = not active
    //  1 = active
    int32_t black;

    // Whether green flag is currently active
    // -1 = no data
    //  0 = not active
    //  1 = active
    int32_t green;

    // Whether checkered flag is currently active
    // -1 = no data
    //  0 = not active
    //  1 = active
    int32_t checkered;

    // Whether white flag is currently active
    // -1 = no data
    //  0 = not active
    //  1 = active
    int32_t white;

    // Whether black and white flag is currently active and reason
    // -1 = no data
    //  0 = not active
    //  1 = blue flag 1st warning
    //  2 = blue flag 2nd warning
    //  3 = wrong way
    //  4 = cutting track
    int32_t black_and_white;
} r3e_flags;

typedef struct
{
    // Range: 0.0 - 1.0
    // Note: -1.0 = N/A
    float engine;

    // Range: 0.0 - 1.0
    // Note: -1.0 = N/A
    float transmission;

    // Range: 0.0 - 1.0
    // Note: A bit arbitrary at the moment. 0.0 doesn't necessarily mean completely destroyed.
    // Note: -1.0 = N/A
    float aerodynamics;

    // Range: 0.0 - 1.0
    // Note: -1.0 = N/A
    float suspension;

    // Reserved data
    float unused1;
    float unused2;
} r3e_car_damage;

typedef struct
{
	// -1.0 = none pending, otherwise penalty time dep on penalty type (drive-through active = 0.0, stop-and-go = time to stay, slow-down = time left to give back etc))
	float drive_through;
	float stop_and_go;
	float pit_stop;
	float time_deduction;
	float slow_down;
} r3e_cut_track_penalties;

typedef struct
{
    // If DRS is equipped and allowed
    // 0 = No, 1 = Yes, -1 = N/A
    int32_t equipped;
    // Got DRS activation left
    // 0 = No, 1 = Yes, -1 = N/A
    int32_t available;
    // Number of DRS activations left this lap
    // Note: In sessions with 'endless' amount of drs activations per lap this value starts at int32::max
    // -1 = N/A
    int32_t numActivationsLeft;
    // DRS engaged
    // 0 = No, 1 = Yes, -1 = N/A
    int32_t engaged;
} r3e_drs;

typedef struct
{
    int32_t available;
    int32_t engaged;
    int32_t amount_left;
    float engaged_time_left;
    float wait_time_left;
} r3e_push_to_pass;

typedef struct
{
    float current_temp[R3E_TIRE_TEMP_INDEX_MAX];
    float optimal_temp;
    float cold_temp;
    float hot_temp;
} r3e_tire_temp;

typedef struct
{
    float current_temp;
    float optimal_temp;
    float cold_temp;
    float hot_temp;
} r3e_brake_temp;

typedef struct
{
    // ABS; -1 = N/A, 0 = off, 1 = on, 5 = currently active
    int32_t abs;
    // TC; -1 = N/A, 0 = off, 1 = on, 5 = currently active
    int32_t tc;
    // ESP; -1 = N/A, 0 = off, 1 = on low, 2 = on medium, 3 = on high, 5 = currently active
    int32_t esp;
    // Countersteer; -1 = N/A, 0 = off, 1 = on, 5 = currently active
    int32_t countersteer;
    // Cornering; -1 = N/A, 0 = off, 1 = on, 5 = currently active
    int32_t cornering;
} r3e_aid_settings;

typedef struct
{
    uint8_t name[64];
    int32_t car_number;
    int32_t class_id;
    int32_t model_id;
    int32_t team_id;
    int32_t livery_id;
    int32_t manufacturer_id;
    int32_t user_id;
    int32_t slot_id;
    int32_t class_performance_index;
    int32_t engine_type;
    float car_width;
	float car_length;
	float rating;
	float reputation;

	// Reserved data
	float unused1;
	float unused2;
} r3e_driver_info;

typedef struct
{
    r3e_driver_info driver_info;
    r3e_finish_status finish_status;
    int32_t place;
    int32_t place_class;
	float lap_distance;
	float lap_distance_fraction;
    r3e_vec3_f32 position;
    int32_t track_sector;
    int32_t completed_laps;
    int32_t current_lap_valid;
    float lap_time_current_self;
    float sector_time_current_self[3];
    float sector_time_previous_self[3];
    float sector_time_best_self[3];
    float time_delta_front;
    float time_delta_behind;
    r3e_pitstop_status pitstop_status;
    int32_t in_pitlane;
    int32_t num_pitstops;
    r3e_cut_track_penalties penalties;
    float car_speed;
    int32_t tire_type_front;
	int32_t tire_type_rear;
	int32_t tire_subtype_front;
	int32_t tire_subtype_rear;
    float base_penalty_weight;
    float aid_penalty_weight;

    // -1 unavailable, 0 = not engaged, 1 = engaged
    int32_t drs_state;
    int32_t ptp_state;

    // -1.0 unavailable, 0.0 - 1.0 tank factor
	float virtual_energy;

    // -1 unavailable, DriveThrough = 0, StopAndGo = 1, Pitstop = 2, Time = 3, Slowdown = 4, Disqualify = 5,
    int32_t penaltyType;

	// Based on the PenaltyType you can assume the reason is:

    // DriveThroughPenaltyInvalid = 0,
    // DriveThroughPenaltyCutTrack = 1,
    // DriveThroughPenaltyPitSpeeding = 2,
    // DriveThroughPenaltyFalseStart = 3,
    // DriveThroughPenaltyIgnoredBlue = 4,
    // DriveThroughPenaltyDrivingTooSlow = 5,
    // DriveThroughPenaltyIllegallyPassedBeforeGreen = 6,
    // DriveThroughPenaltyIllegallyPassedBeforeFinish = 7,
    // DriveThroughPenaltyIllegallyPassedBeforePitEntrance = 8,
    // DriveThroughPenaltyIgnoredSlowDown = 9,
    // DriveThroughPenaltyMax = 10

    // StopAndGoPenaltyInvalid = 0,
    // StopAndGoPenaltyCutTrack1st = 1,
    // StopAndGoPenaltyCutTrackMult = 2,
    // StopAndGoPenaltyYellowFlagOvertake = 3,
    // StopAndGoPenaltyVirtualEnergy = 4,
    // StopAndGoPenaltyMax = 5

    // PitstopPenaltyInvalid = 0,
    // PitstopPenaltyIgnoredPitstopWindow = 1,
    // PitstopPenaltyMax = 2

    // ServableTimePenaltyInvalid = 0,
    // ServableTimePenaltyServedMandatoryPitstopLate = 1,
    // ServableTimePenaltyIgnoredMinimumPitstopDuration = 2,
    // ServableTimePenaltyMax = 3

    // SlowDownPenaltyInvalid = 0,
    // SlowDownPenaltyCutTrack1st = 1,
    // SlowDownPenaltyCutTrackMult = 2,
    // SlowDownPenaltyMax = 3

    // DisqualifyPenaltyInvalid = -1,
    // DisqualifyPenaltyFalseStart = 0,
    // DisqualifyPenaltyPitlaneSpeeding = 1,
    // DisqualifyPenaltyWrongWay = 2,
    // DisqualifyPenaltyEnteringPitsUnderRed = 3,
    // DisqualifyPenaltyExitingPitsUnderRed = 4,
    // DisqualifyPenaltyFailedDriverChange = 5,
    // DisqualifyPenaltyThreeDriveThroughsInLap = 6,
    // DisqualifyPenaltyLappedFieldMultipleTimes = 7,
    // DisqualifyPenaltyIgnoredDriveThroughPenalty = 8,
    // DisqualifyPenaltyIgnoredStopAndGoPenalty = 9,
    // DisqualifyPenaltyIgnoredPitStopPenalty = 10,
    // DisqualifyPenaltyIgnoredTimePenalty = 11,
    // DisqualifyPenaltyExcessiveCutting = 12,
    // DisqualifyPenaltyIgnoredBlueFlag = 13,
    // DisqualifyPenaltyMax = 14
    int32_t penaltyReason;
	
    // -1 unavailable, 0 = ignition off, 1 = ignition on but not running, 2 = ignition on and starter running, 3 = ignition on and running
    int32_t engineState;

    // Orientation in Euler coordinates
    r3e_vec3_f32 orientation;

	// Reserved data
	float unused1;
	float unused2;
	float unused3;
} r3e_driver_data;

struct r3e_share
{
    //////////////////////////////////////////////////////////////////////////
    // Version
    //////////////////////////////////////////////////////////////////////////

    int32_t version_major;
    int32_t version_minor;
    int32_t all_drivers_offset; // Offset to num_cars
    int32_t driver_data_size; // size of the driver data struct

    //////////////////////////////////////////////////////////////////////////
    // Game State
    //////////////////////////////////////////////////////////////////////////

    int32_t game_mode;
    int32_t game_paused;
    int32_t game_in_menus;
    int32_t game_in_replay;
    int32_t game_using_vr;

    // Reserved data
    int32_t game_unused1;

    //////////////////////////////////////////////////////////////////////////
    // High detail
    //////////////////////////////////////////////////////////////////////////

    // High detail player vehicle data
    r3e_playerdata player;

    //////////////////////////////////////////////////////////////////////////
    // Event and session
    //////////////////////////////////////////////////////////////////////////

    uint8_t track_name[64];
    uint8_t layout_name[64];

    int32_t track_id;
    int32_t layout_id;
    float layout_length;
	r3e_sectorStarts sector_start_factors;
	
    // Race session durations
    // Note: Index 0-2 = race 1-3
    // Note: Value -1 = N/A
    // Note: If both laps and minutes are more than 0, race session starts with minutes then adds laps
    int32_t race_session_laps[3];
    int32_t race_session_minutes[3];

    // The current race event index, for championships with multiple events
    // Note: 0-indexed, -1 = N/A
    int32_t event_index;
    // Which session the player is in (practice, qualifying, race, etc.)
    // Note: See the r3e_session enum
    int32_t session_type;
    // The current iteration of the current type of session
    // Note: 1 = first, 2 = second etc, -1 = N/A
    int32_t session_iteration;	
    // If the session is time based, lap based or time based with an extra lap at the end
    r3e_session_length_format session_length_format;
    // Unit: Meter per second (m/s)
    float session_pit_speed_limit;

    // Which phase the current session is in (gridwalk, countdown, green flag, etc.)
    // Note: See the r3e_session_phase enum
    int32_t session_phase;

    // Which phase start lights are in; -1 = unavailable, 0 = off, 1-5 = redlight on and counting down, 6 = greenlight on
    // Note: See the r3e_session_phase enum
    int32_t start_lights;

    // If tire wear is active (-1 = N/A, 0 = Off, 1 = 1x, 2 = 2x, 3 = 3x, 4 = 4x)
    int32_t tire_wear_active;
    // If fuel usage is active (-1 = N/A, 0 = Off, 1 = 1x, 2 = 2x, 3 = 3x, 4 = 4x)
    int32_t fuel_use_active;

    // Total number of laps in the race, or -1 if player is not in race mode (practice, test mode, etc.)
    int32_t number_of_laps;

    // Amount of time and time remaining for the current session
    // Note: Only available in time-based sessions, -1.0 = N/A
    // Units: Seconds
    float session_time_duration;
    float session_time_remaining;

    // Server max incident points, -1 = N/A
    int32_t max_incident_points;

    // Reserved data
	float event_unused1;
    float event_unused2;

    //////////////////////////////////////////////////////////////////////////
    // Pit
    //////////////////////////////////////////////////////////////////////////

    // Current status of the pit stop
    // Note: See the r3e_pit_window enum
    int32_t pit_window_status;

    // The minute/lap from which you're obligated to pit (-1 = N/A)
    // Unit: Minutes in time-based sessions, otherwise lap
    int32_t pit_window_start;

    // The minute/lap into which you need to have pitted (-1 = N/A)
    // Unit: Minutes in time-based sessions, otherwise lap
    int32_t pit_window_end;

    // If current vehicle is in pitlane (-1 = N/A)
    int32_t in_pitlane;

    // What is currently selected in pit menu, and array of states (preset/buttons: -1 = not selectable, 1 = selectable) (actions: -1 = N/A, 0 = unmarked for fix, 1 = marked for fix)
    r3e_pit_menu_selection pit_menu_selection;
    int32_t pit_menu_state[R3E_PIT_MENU_MAX];

    // Current vehicle pit state (-1 = N/A, 0 = None, 1 = Requested stop, 2 = Entered pitlane heading for pitspot, 3 = Stopped at pitspot, 4 = Exiting pitspot heading for pit exit)
    int32_t pit_state;
	
	// Current vehicle pitstop actions duration
    float pit_total_duration;
    float pit_elapsed_time;

	// Current vehicle pit action (-1 = N/A, 0 = None, 1 = Preparing, (combination of 2 = Penalty serve, 4 = Driver change, 8 = Refueling, 16 = Front tires, 32 = Rear tires, 64 = Body, 128 = Front wing, 256 = Rear wing, 512 = Suspension))
	int32_t pit_action;

    // Number of pitstops the current vehicle has performed (-1 = N/A)
    int32_t num_pitstops;

    // Pitstop with min duration (-1.0 = N/A, else seconds)
    float pit_min_duration_total;
    float pit_min_duration_left;

    //////////////////////////////////////////////////////////////////////////
    // Scoring & Timings
    //////////////////////////////////////////////////////////////////////////

    // The current state of each type of flag
    r3e_flags flags;

    // Current position (1 = first place)
    int32_t position;
    int32_t position_class;

    r3e_finish_status finish_status;

    // Total number of cut track warnings (-1 = N/A)
    int32_t cut_track_warnings;
    // The number of penalties the car currently has pending of each type (-1 = N/A)
    r3e_cut_track_penalties penalties;
    // Total number of penalties pending for the car
    // Note: See the 'penalties' field
    int32_t num_penalties;

    // How many laps the car has completed. If this value is 6, the car is on it's 7th lap. -1 = n/a
    int32_t completed_laps;
    int32_t current_lap_valid;
    int32_t track_sector;
    float lap_distance;

    // fraction of lap completed, 0.0-1.0, -1.0 = N/A
    float lap_distance_fraction;

    // The current best lap time for the leader of the session
    // Unit: Seconds (-1.0 = N/A)
    float lap_time_best_leader;
    // The current best lap time for the leader of the current/viewed vehicle's class in the current session
    // Unit: Seconds (-1.0 = N/A)
    float lap_time_best_leader_class;
    // Sector times of fastest lap by anyone in session
    // Unit: Seconds (-1.0 = N/A)
    float session_best_lap_sector_times[3];
    // Best lap time
    // Unit: Seconds (-1.0 = N/A)
    float lap_time_best_self;
    float sector_time_best_self[3];
    // Previous lap
    // Unit: Seconds (-1.0 = N/A)
    float lap_time_previous_self;
    float sector_time_previous_self[3];
    // Current lap time
    // Unit: Seconds (-1.0 = N/A)
    float lap_time_current_self;
    float sector_time_current_self[3];
    // The time delta between this car's time and the leader
    // Unit: Seconds (-1.0 = N/A)
    float lap_time_delta_leader;
    // The time delta between this car's time and the leader of the car's class
    // Unit: Seconds (-1.0 = N/A)
    float lap_time_delta_leader_class;
    // Time delta between this car and the car placed in front
    // Unit: Seconds (-1.0 = N/A)
    float time_delta_front;
    // Time delta between this car and the car placed behind
    // Unit: Seconds (-1.0 = N/A)
    float time_delta_behind;
    // Time delta between this car's current laptime and this car's best laptime
    // Unit: Seconds (-1000.0 = N/A)
    float time_delta_best_self;
    // Best time for each individual sector no matter lap
    // Unit: Seconds (-1.0 = N/A)
    float best_individual_sector_time_self[3];
    float best_individual_sector_time_leader[3];
    float best_individual_sector_time_leader_class[3];
    // Incident points (-1 = N/A)
    int32_t incident_points;
	
    // -1 = N/A, 0 = this and next lap valid, 1 = this lap invalid, 2 = this and next lap invalid
	int32_t lap_valid_state;
    // -1 = N/A, 0 = invalid, 1 = valid
	int32_t prev_lap_valid;

	// Reserved data
	float unused1;
	float unused2;
	float unused3;

    //////////////////////////////////////////////////////////////////////////
    // Vehicle information
    //////////////////////////////////////////////////////////////////////////

    r3e_driver_info vehicle_info;
    uint8_t player_name[64];

    //////////////////////////////////////////////////////////////////////////
    // Vehicle state
    //////////////////////////////////////////////////////////////////////////

    // Which controller is currently controlling the vehicle (AI, player, remote, etc.)
    // Note: See the r3e_control enum
    int32_t control_type;

    // Unit: Meter per second (m/s)
    float car_speed;

    // Unit: Radians per second (rad/s)
    float engine_rps;
    float max_engine_rps;
    float upshift_rps;

    // -2 = N/A, -1 = reverse, 0 = neutral, 1 = first gear, ... (for electric cars, gear is set to 2 if regenerative braking is enabled)
    int32_t gear;
    // -1 = N/A
    int32_t num_gears;

    // Physical location of car's center of gravity in world space (X, Y, Z) (Y = up)
    r3e_vec3_f32 car_cg_location;
    // Pitch, yaw, roll
    // Unit: Radians (rad)
    r3e_ori_f32 car_orientation;
    // Acceleration in three axes (X, Y, Z) of car body in local-space.
    // From car center, +X=left, +Y=up, +Z=back.
    // Unit: Meter per second squared (m/s^2)
    r3e_vec3_f32 local_acceleration;
	
    // Unit: Kilograms (kg)
    // Note: Car + penalty weight + fuel
    float total_mass;
    // Unit: Liters (l)
    // Note: Fuel per lap show estimation when not enough data, then max recorded fuel per lap
    // Note: Not valid for remote players
    float fuel_left;
    float fuel_capacity;
	float fuel_per_lap;
	// Unit: Mega-Joule (MJ)
	// Note: -1.0f when not enough data, then max recorded virtual energy per lap
	// Note: Not valid for remote players
	float virtual_energy_left;
	float virtual_energy_capacity;
	float virtual_energy_per_lap;
    // Unit: Celsius (C)
    // Note: Not valid for AI or remote players
    float engine_temp;
    float engine_oil_temp;
    // Unit: Kilopascals (KPa)
    // Note: Not valid for AI or remote players
    float fuel_pressure;
    float engine_oil_pressure;
    // Unit: (Bar)
    // Note: Not valid for AI or remote players (-1.0 = N/A)
    float turbo_pressure;

    // How pressed the throttle pedal is
    // Range: 0.0 - 1.0 (-1.0 = N/A)
    // Note: Not valid for AI or remote players
    float throttle;
    float throttle_raw;
    // How pressed the brake pedal is
    // Range: 0.0 - 1.0 (-1.0 = N/A)
    // Note: Not valid for AI or remote players
    float brake;
    float brake_raw;
    // How pressed the clutch pedal is
    // Range: 0.0 - 1.0 (-1.0 = N/A)
    // Note: Not valid for AI or remote players
    float clutch;
    float clutch_raw;
    // How much the steering wheel is turned
    // Range: -1.0 - 1.0
    // Note: Not valid for AI or remote players
    float steer_input_raw;
    // How many degrees in steer lock (center to full lock)
    // Note: Not valid for AI or remote players
    int32_t steer_lock_degrees;
    // How many degrees in wheel range (degrees full left to rull right)
    // Note: Not valid for AI or remote players
    int32_t steer_wheel_range_degrees;

	// Aid settings
	r3e_aid_settings aid_settings;

    // DRS data
    r3e_drs drs;

    // Pit limiter (-1 = N/A, 0 = inactive, 1 = active)
    int32_t pit_limiter;

    // Push to pass data
    r3e_push_to_pass push_to_pass;

    // How much the vehicle's brakes are biased towards the back wheels (0.3 = 30%, etc.) (-1.0 = N/A)
    // Note: Not valid for AI or remote players
    float brake_bias;

    // DRS activations available in total (-1 = N/A or endless), placed outside of drs struct to keep backwards compatibility
    int32_t drs_numActivationsTotal;

    // PTP activations available in total (-1 = N/A, or there's no restriction per lap, or endless), placed outside of ptp struct to keep backwards compatibility
    int32_t ptp_numActivationsTotal;

    // Battery state of charge
    // Range: 0.0 - 100.0 (-1.0 = N/A)
    float battery_soc;

    // Brake water tank (-1.0 = N/A)
    // Unit: Liters (l)
    float water_left;
	
    // -1 = N/A
    int32_t abs_setting;
	
    // -1 = N/A or dont exist on car, 0 = ignition off or headlights off, 1 = on, 2 = strobing
    int32_t headlights;

    // Reserved data
    float vehicle_unused1;

    //////////////////////////////////////////////////////////////////////////
    // Tires
    //////////////////////////////////////////////////////////////////////////

    // Which type of tires the car has (option, prime, etc.)
    // Note: See the r3e_tire_type enum, deprecated - use the values further down instead
    int32_t tire_type;
    // Rotation speed
    // Uint: Radians per second
    float tire_rps[R3E_TIRE_INDEX_MAX];
    // Wheel speed
    // Uint: Meters per second
    float tire_speed[R3E_TIRE_INDEX_MAX];
    // Range: 0.0 - 1.0 (-1.0 = N/A)
    float tire_grip[R3E_TIRE_INDEX_MAX];
    // Range: 0.0 - 1.0 (-1.0 = N/A)
    float tire_wear[R3E_TIRE_INDEX_MAX];
    // (-1 = N/A, 0 = false, 1 = true)
    int32_t tire_flatspot[R3E_TIRE_INDEX_MAX];
    // Unit: Kilopascals (KPa) (-1.0 = N/A)
    // Note: Not valid for AI or remote players
    float tire_pressure[R3E_TIRE_INDEX_MAX];
    // Percentage of dirt on tire (-1.0 = N/A)
    // Range: 0.0 - 1.0
    float tire_dirt[R3E_TIRE_INDEX_MAX];
    // Current temperature of three points across the tread of the tire (-1.0 = N/A)
    // Optimum temperature
    // Cold temperature
    // Hot temperature
    // Unit: Celsius (C)
    // Note: Not valid for AI or remote players
    r3e_tire_temp tire_temp[R3E_TIRE_INDEX_MAX];
	// Which type of tires the car has (option, prime, etc.)
	// Note: See the r3e_tire_type enum
	int32_t tire_type_front;
	int32_t tire_type_rear;
	// Which subtype of tires the car has
	// Note: See the r3e_tire_subtype enum
	int32_t tire_subtype_front;
	int32_t tire_subtype_rear;
    // Current brake temperature (-1.0 = N/A)
    // Optimum temperature
    // Cold temperature
    // Hot temperature
    // Unit: Celsius (C)
    // Note: Not valid for AI or remote players
    r3e_brake_temp brake_temp[R3E_TIRE_INDEX_MAX];
    // Brake pressure (-1.0 = N/A)
    // Unit: Kilo Newtons (kN)
    // Note: Not valid for AI or remote players
    float brake_pressure[R3E_TIRE_INDEX_MAX];

    //////////////////////////////////////////////////////////////////////////
    // Electronics
    //////////////////////////////////////////////////////////////////////////
	
    // -1 = N/A
    int32_t traction_control_setting;
    int32_t engine_map_setting;
    int32_t engine_brake_setting;
	
    // -1.0 = N/A, 0.0 -> 100.0 percent
    float traction_control_percent;
	
    // Material under player car tires, see the r3e_mtrl_type enum
    int32_t tire_on_mtrl[R3E_TIRE_INDEX_MAX];

    // Tire load (N)
    // -1.0 = N/A
    float tire_load[R3E_TIRE_INDEX_MAX];

    //////////////////////////////////////////////////////////////////////////
    // Damage
    //////////////////////////////////////////////////////////////////////////

    // The current state of various parts of the car
    // Note: Not valid for AI or remote players
    r3e_car_damage car_damage;

    //////////////////////////////////////////////////////////////////////////
    // Driver info
    //////////////////////////////////////////////////////////////////////////

    // Number of cars (including the player) in the race
    int32_t num_cars;

    // Contains name and basic vehicle info for all drivers in place order
    r3e_driver_data all_drivers_data_1[R3E_NUM_DRIVERS_MAX];
};

#pragma pack(pop)
