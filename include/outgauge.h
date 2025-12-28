
typedef struct //lfsVec3
{
    float x;
    float y;
    float z;
} lfsVec3;

typedef struct //lfsVecInt
{
    int x;
    int y;
    int z;
} lfsVecInt;

struct outgauge
{
    unsigned       time;            // time in milliseconds (to check order) // N/A, hardcoded to 0
    char           car[4];          // Car name // N/A, fixed value of "beam"
    unsigned short flags;           // Info (see OG_x below)
    char           gear;            // Reverse:0, Neutral:1, First:2...
    char           plid;            // Unique ID of viewed player (0 = none) // N/A, hardcoded to 0
    float          speed;           // M/S
    float          rpm;             // RPM
    float          turbo;           // BAR
    float          engTemp;         // C
    float          fuel;            // 0 to 1
    float          oilPressure;     // BAR // N/A, hardcoded to 0
    float          oilTemp;         // C
    unsigned       dashLights;      // Dash lights available (see DL_x below)
    unsigned       showLights;      // Dash lights currently switched on
    float          throttle;        // 0 to 1
    float          brake;           // 0 to 1
    float          clutch;          // 0 to 1
    char           display1[16];    // Usually Fuel // N/A, hardcoded to ""
    char           display2[16];    // Usually Settings // N/A, hardcoded to ""
    int            id;              // optional - only if OutGauge ID is specified
};



typedef struct //OutSimMain
{
    lfsVec3     AngVel;     // 3 floats, angular velocity vector
    float       Heading;    // anticlockwise from above (Z)
    float       Pitch;      // anticlockwise from right (X)
    float       Roll;       // anticlockwise from front (Y)
    lfsVec3     Accel;      // 3 floats X, Y, Z
    lfsVec3     Vel;        // 3 floats X, Y, Z
    lfsVecInt           Pos;        // 3 ints   X, Y, Z (1m = 65536)
} OutSimMain;

typedef struct //OutSimInputs
{
    float       Throttle;       // 0 to 1
    float       Brake;          // 0 to 1
    float       InputSteer;     // radians
    float       Clutch;         // 0 to 1
    float       Handbrake;      // 0 to 1
} OutSimInputs;

typedef struct //OutSimWheel // 10 ints
{
    float       SuspDeflect;        // compression from unloaded
    float       Steer;              // including Ackermann and toe
    float       XForce;             // force right
    float       YForce;             // force forward
    float       VerticalLoad;       // perpendicular to surface
    float       AngVel;             // radians/s
    float       LeanRelToRoad;      // radians a-c viewed from rear

    uint8_t     AirTemp;            // degrees C
    uint8_t     SlipFraction;       // (0 to 255 - see below)
    uint8_t     Touching;           // touching ground
    uint8_t     Sp3;

    float       SlipRatio;          // slip ratio
    float       TanSlipAngle;       // tangent of slip angle
} OutSimWheel;

struct outsim // size depends on OutSim Opts
{
    // if (OSOpts & OSO_HEADER)

    char L;
    char F;
    char S;
    char T;

    // if (OSOpts & OSO_ID)

    int           ID;                 // OutSim ID from cfg.txt

    // if (OSOpts & OSO_TIME)

    unsigned  Time;               // time in milliseconds (to check order)

    // if (OSOpts & OSO_MAIN)

    OutSimMain        OSMain;         // struct - see above

    // if (OSOpts & OSO_INPUTS)

    OutSimInputs  OSInputs;       // struct - see above

    // if (OSOpts & OSO_DRIVE)

    uint8_t       Gear;               // 0=R, 1=N, 2=first gear
    uint8_t       Sp1;                // spare
    uint8_t       Sp2;
    uint8_t       Sp3;

    float     EngineAngVel;       // radians/s
    float     MaxTorqueAtVel;     // Nm : output torque for throttle 1.0

    // if (OSOpts & OSO_DISTANCE)

    float     CurrentLapDist;     // m - travelled by car
    float     IndexedDistance;    // m - track ruler measurement

    // if (OSOpts & OSO_WHEELS)

    OutSimWheel       OSWheels[4];    // array of structs - see above
};
