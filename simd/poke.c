#include "poke.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <poll.h>
#include <termios.h>
#include <uv.h>
#include <yder.h>
#include <sys/types.h>
#include <pwd.h>
#include <libconfig.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <simapi.h>
#include <simdata.h>
#include <simmapper.h>
#include "loopdata.h"
#include "../simapi/test.h"
#include "../simmap/basicmap.h"

int poke(SimdSettings simds)
{
    int fd = shm_open(SIMAPI_MEM_FILE, O_RDWR, S_IRUSR | S_IWUSR);
    int res = ftruncate(fd, sizeof(SimData));
    
    SimData* s = mmap(NULL, sizeof(SimData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    struct Map* map = (struct Map*) malloc((1171 + 1) * sizeof(struct Map));
    int size = CreateSimDataMap(map, s, 1);
    
    void* addr;
    SimDataType sdt;
    for (int k = 0; k <= 1171; k++)
    {
        if ( map[k].name == NULL )
        {
            continue;
        }
        if (strcmp(map[k].name, simds.pokesetting) == 0)
        {
            addr = map[k].value;
            sdt = map[k].dtype;
            break;
        }
    }
    
    if(addr != NULL)
    {
        switch(sdt)
        {
            case UINT8:
                uint8_t new_uint8_value = strtoul(simds.targetvalue, NULL, 0);
                memcpy(addr, &new_uint8_value, sizeof(new_uint8_value));
                break;
            case UINT32:
                uint32_t new_uint32_value = strtoul(simds.targetvalue, NULL, 0);
                memcpy(addr, &new_uint32_value, sizeof(new_uint32_value));
                break;
            case DOUBLE:
                double new_double_value = strtod(simds.targetvalue, NULL);
                memcpy(addr, &new_double_value, sizeof(new_double_value));
            case BOOLEAN:
                bool val = false;
                if(strcmp(simds.targetvalue, "1") == 0)
                {
                    val = true;
                }
                memcpy(addr, &val, sizeof(val));
        }
    }
}
