#ifndef _PARAMETERS_H
#define _PARAMETERS_H

#include <stdbool.h>

typedef struct
{
    int verbosity_count;
    bool daemon;
    bool memmap;
}
Parameters;


typedef enum
{
    E_SUCCESS_AND_EXIT = 0,
    E_SUCCESS_AND_DO   = 1,
    E_SOMETHING_BAD    = 2
}
ConfigError;

int freeparams(Parameters* p);
ConfigError getParameters(int argc, char** argv, Parameters* p);


#endif
