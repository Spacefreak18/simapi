#ifndef _SIMMAPI_H
#define _SIMMAPI_H

typedef enum
{
    SIMULATOR_SIMAPI_TEST       = 0,
    SIMULATOR_ASSETTO_CORSA     = 1,
    SIMULATOR_RFACTOR2          = 2,
    SIMULATOR_PROJECTCARS2      = 3,
}
Simulator;

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
}
SimDataType;

#endif
