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
    SIMULATORAPI_DIRT_RALLY_2      = 6,
    SIMULATORAPI_F1_2018           = 7,
    SIMULATORAPI_RACE_ROOM         = 8,
    SIMULATORAPI_FORZA             = 9,
    SIMULATORAPI_LMU               = 10,
    SIMULATORAPI_WRECKFEST2        = 11,
    SIMULATORAPI_RICHARD_BURNS_RALLY = 12,
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
    SIMULATOREXE_ASSETTO_CORSA_RALLY          = 3917090, //acr
    SIMULATOREXE_LEMANS_ULTIMATE              = 2399420, //lmu
    SIMULATOREXE_BEAMNG                       = 284160, //beamng
    SIMULATOREXE_LIVE_FOR_SPEED               = 0000001, //lfs
    SIMULATOREXE_DIRT_RALLY_2                 = 690790, //dr2
    SIMULATOREXE_F1_2022                      = 1692250, //f122
    SIMULATOREXE_RACE_ROOM                    = 211500, //r3e
    SIMULATOREXE_FORZA_HORIZON_5              = 1551360, //fh5
    SIMULATOREXE_WRECKFEST2                   = 1203190, //wf2
    SIMULATOREXE_RICHARD_BURNS_RALLY          = 0000002, //rbr
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
