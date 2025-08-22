#ifndef _CONFIGHELPER_H
#define _CONFIGHELPER_H

#include <stdbool.h>
#include <stdint.h>

#include <libconfig.h>

#include "loopdata.h"

int loadconfig(SimdSettings simds, int compat_info_size, GameCompatInfo* game_compat_info);

int getNumberOfConfigs(const char* config_file_str);

#endif
