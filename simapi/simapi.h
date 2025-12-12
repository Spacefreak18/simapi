#ifndef _SIMMAPI_H
#define _SIMMAPI_H

#define SIMAPI_VERSION 1

typedef void (*func_ptr_t)(char* message);
//func_ptr_t logfunc;

void set_simapi_log_info(func_ptr_t logfunc);
void set_simapi_log_debug(func_ptr_t logfunc);
void set_simapi_log_trace(func_ptr_t logfunc);


typedef enum
{
    SIMAPI_LOGLEVEL_INFO          = 0,
    SIMAPI_LOGLEVEL_DEBUG         = 1,
    SIMAPI_LOGLEVEL_TRACE         = 2,
}
SIMAPI_LOGLEVEL;

typedef enum
{
    SIMULATORAPI_SIMAPI_TEST       = 0,
    SIMULATORAPI_ASSETTO_CORSA     = 1,
    SIMULATORAPI_RFACTOR2          = 2,
    SIMULATORAPI_PROJECTCARS2      = 3,
    SIMULATORAPI_SCSTRUCKSIM2      = 4,
    SIMULATORAPI_OUTSIMOUTGAUGE    = 5,
}
SimulatorAPI;

typedef enum
{
    SIMULATOREXE_SIMAPI_TEST_NONE             = 0,
    SIMULATOREXE_ASSETTO_CORSA                = 244210, //ac
    SIMULATOREXE_RFACTOR2                     = 365960, //rf2
    SIMULATOREXE_AUTOMOBILISTA2               = 1066890, //ams2
    SIMULATOREXE_EUROTRUCKS2                  = 227300, //et2
    SIMULATOREXE_AMERICANTRUCKS               = 270880, //at
    SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE   = 805550, //acc
    SIMULATOREXE_ASSETTO_CORSA_EVO            = 3058630, //ace
    SIMULATOREXE_LEMANS_ULTIMATE              = 2399420, //lmu
    SIMULATOREXE_BEAMNG                       = 284160, //beamng
    SIMULATOREXE_LIVE_FOR_SPEED               = 0000001, //lfs
}
SimulatorEXE;

typedef enum
{
    SIMAPI_ERROR_NONE          = 0,
    SIMAPI_ERROR_UNKNOWN       = 1,
    SIMAPI_ERROR_INVALID_SIM   = 2,
    SIMAPI_ERROR_NODATA        = 3,
}
SimAPIError;

typedef enum
{
    INTEGER       = 0,
    DOUBLE        = 1,
    FLOAT         = 2,
    CHAR          = 3,
    BOOLEAN       = 4,
    UINT8         = 5,
    UINT32        = 6,
    UINT64        = 7,
}
SimDataType;

#endif
