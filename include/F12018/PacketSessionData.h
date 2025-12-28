#ifndef __PacketSessionData_h__
#define __PacketSessionData_h__

typedef struct //MarshalZone
{
    float  m_zoneStart;   // Fraction (0..1) of way through the lap the marshal zone starts
    int8_t m_zoneFlag;    // -1 = invalid/unknown, 0 = none, 1 = green, 2 = blue, 3 = yellow, 4 = red

} MarshalZone;

enum Weather: uint8_t
{
    WEATHER_CLEAR = 0,
    WEATHER_LIGHT_CLOUD = 1,
    WEATHER_OVERCAST = 2,
    WEATHER_LIGHT_RAIN = 3,
    WEATHER_HEAVY_RAIN = 4,
    WEATHER_STORM = 5
};

static const char* const Weather_name[] =
{
    "CLEAR", "LIGHT_CLOUD", "OVERCAST", "LIGHT_RAIN", "HEAVY_RAIN", "STORM",
};

enum SessionType: uint8_t
{
    SESSION_TYPE_UNKNOWN = 0,
    SESSION_TYPE_P1 = 1,
    SESSION_TYPE_P2 = 2,
    SESSION_TYPE_P3 = 3,
    SESSION_TYPE_SHORT_P = 4,
    SESSION_TYPE_Q1 = 5,
    SESSION_TYPE_Q2 = 6,
    SESSION_TYPE_Q3 = 7,
    SESSION_TYPE_SHORT_Q = 8,
    SESSION_TYPE_OSQ = 9,
    SESSION_TYPE_R = 10,
    SESSION_TYPE_R2 = 11,
    SESSION_TYPE_TIME_TRIAL = 12
};

static const char* const SessionType_name[] =
{
    "UNKNOWN", "P1", "P2", "P3", "SHORT_P", "Q1", "Q2", "Q3", "SHORT_Q", "OSQ",
    "R", "R2", "TIME_TRIAL",
};

enum TrackID: uint8_t
{
    TRACK_ID_MELBOURNE = 0,
    TRACK_ID_PAUL_RICARD = 1,
    TRACK_ID_SHANGHAI = 2,
    TRACK_ID_SAKHIR = 3,
    TRACK_ID_CATALUNYA = 4,
    TRACK_ID_MONACO = 5,
    TRACK_ID_MONTREAL = 6,
    TRACK_ID_SILVERSTONE = 7,
    TRACK_ID_HOCKENHEIM = 8,
    TRACK_ID_HUNGARORING = 9,
    TRACK_ID_SPA = 10,
    TRACK_ID_MONZA = 11,
    TRACK_ID_SINGAPORE = 12,
    TRACK_ID_SUZUKA = 13,
    TRACK_ID_ABU_DHABI = 14,
    TRACK_ID_TEXAS = 15,
    TRACK_ID_BRAZIL = 16,
    TRACK_ID_AUSTRIA = 17,
    TRACK_ID_SOCHI = 18,
    TRACK_ID_MEXICO = 19,
    TRACK_ID_BAKU = 20,
    TRACK_ID_SAKHIR_SHORT = 21,
    TRACK_ID_SILVERSTONE_SHORT = 22,
    TRACK_ID_TEXAS_SHORT = 23,
    TRACK_ID_SUZUKA_SHORT = 24
};

static const char* const TrackID_name[] =
{
    "MELBOURNE", "PAUL_RICARD", "SHANGHAI", "SAKHIR", "CATALUNYA", "MONACO", "MONTREAL",
    "SILVERSTONE", "HOCKENHEIM", "HUNGARORING", "SPA", "MONZA", "SINGAPORE", "SUZUKA",
    "ABU_DHABI", "TEXAS", "BRAZIL", "AUSTRIA", "SOCHI", "MEXICO", "BAKU", "SAKHIR_SHORT",
    "SILVERSTONE_SHORT", "TEXAS_SHORT", "SUZUKA_SHORT"
};

enum Era: uint8_t
{
    ERA_MODERN = 0, ERA_CLASSIC = 1,
};

static const char* const Era_name[] =
{
    "MODERN", "CLASSIC",
};

enum SLIProNativeSupport: uint8_t
{
    SLIP_PRO_NATIVE_SUPPORT_INACTIVE = 0, SLIP_PRO_NATIVE_SUPPORT_ACTIVE = 1,
};

static const char* const SLIProNativeSupport_name[] =
{
    "INACTIVE", "ACTIVE",
};

enum SafetyCarStatus: uint8_t
{
    NO_SAFETY_CAR = 0, FULL_SAFETY_CAR = 1, VIRTUAL_SAFETY_CAR = 2,
};

static const char* const SafetyCarStatus_name[] =
{
    "NO_SAFETY_CAR", "FULL_SAFETY_CAR", "VIRTUAL_SAFETY_CAR"
};

enum NetworkGame: uint8_t
{
    NETWORK_GAME_OFFLINE = 0, NETWORK_GAME_ONLINE = 1,
};

static const char* const NetworkGame_name[] =
{
    "OFFLINE", "ONLINE",
};

struct PacketSessionData
{
    uint8_t         m_weather;              // Weather - 0 = clear, 1 = light cloud, 2 = overcast
    // 3 = light rain, 4 = heavy rain, 5 = storm
    int8_t          m_trackTemperature;     // Track temp. in degrees celsius
    int8_t          m_airTemperature;       // Air temp. in degrees celsius
    uint8_t         m_totalLaps;            // Total number of laps in this race
    uint16_t        m_trackLength;          // Track length in metres
    uint8_t         m_sessionType;          // 0 = unknown, 1 = P1, 2 = P2, 3 = P3, 4 = Short P
    // 5 = Q1, 6 = Q2, 7 = Q3, 8 = Short Q, 9 = OSQ
    // 10 = R, 11 = R2, 12 = Time Trial
    int8_t          m_trackId;              // -1 for unknown, 0-21 for tracks, see appendix
    uint8_t         m_era;                  // Era, 0 = modern, 1 = classic
    uint16_t        m_sessionTimeLeft;      // Time left in session in seconds
    uint16_t        m_sessionDuration;      // Session duration in seconds
    uint8_t         m_pitSpeedLimit;        // Pit speed limit in kilometres per hour
    uint8_t         m_gamePaused;           // Whether the game is paused
    uint8_t         m_isSpectating;         // Whether the player is spectating
    uint8_t         m_spectatorCarIndex;    // Index of the car being spectated
    uint8_t         m_sliProNativeSupport;  // SLI Pro support, 0 = inactive, 1 = active
    uint8_t         m_numMarshalZones;      // Number of marshal zones to follow
    MarshalZone     m_marshalZones[21];     // List of marshal zones – max 21
    uint8_t         m_safetyCarStatus;      // 0 = no safety car, 1 = full safety car
    // 2 = virtual safety car
    uint8_t        m_networkGame;           // 0 = offline, 1 = online

};

#endif
