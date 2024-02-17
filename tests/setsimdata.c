#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "../simmap/mapacdata.h"
#include "../simmap/maprf2data.h"

#include "../simapi/ac.h"
#include "../include/acdata.h"

#include "../simapi/rf2.h"
#include "../include/rf2data.h"

const char* mem_file;
const char* variable_name;
const char* data_type;
const char* valuetoset;
const char* action;
const char* save_file;

/* Parse command line arguments */
void parse_args(int argc, char* argv[])
{
    int i;

    int help = (argc < 2);

    for (i=1; i<argc && !help; i++)
    {
        if (!strcmp(argv[i],"-f"))
        {
            if (i<argc-1)
            {
                mem_file = argv[++i];
            }
            else
            {
                help = 1;
            }
        }
        else
            if (!strcmp(argv[i],"-n"))
            {
                if (i<argc-1)
                {
                    variable_name = argv[++i];
                }
                else
                {
                    help = 1;
                }
            }
            else
                if (!strcmp(argv[i],"-t"))
                {
                    if (i<argc-1)
                    {
                        //motion_frequency=atof(argv[++i]);
                        data_type = argv[++i];
                    }
                    else
                    {
                        help = 1;
                    }
                }
                else
                    if (!strcmp(argv[i],"-v"))
                    {
                        if (i<argc-1)
                        {
                            //motion_amplitude=atof(argv[++i]);
                            valuetoset = argv[++i];
                        }
                        else
                        {
                            help = 1;
                        }
                    }
                    else
                        if (!strcmp(argv[i],"-a"))
                        {
                           if (i<argc-1)
                            {
                                action = argv[++i];
                                //spring_strength =atof(argv[++i]);
                            }
                            else
                            {
                                help = 1;
                            }
                        }
                        else
                            if (!strcmp(argv[i],"-s"))
                            {
                                if (i<argc-1)
                                {
                                    save_file = argv[++i];
                                    //spring_strength =atof(argv[++i]);
                                }
                                else
                                {
                                    help = 1;
                                }
                                }
                            else
                                if (!strcmp(argv[i],"-o"))
                                {
                                    ;
                                }
                                else
                                    if (!strcmp(argv[i],"-A"))
                                    {
                                        //autocenter_off = 1;
                                    }
                                    else
                                    {
                                        help = 1;
                                    }
    }


    if (help)
    {
        printf("-------- Manually Create and Edit Simulator Memory Mapped Files for Testing --------\n");
        printf("Description:\n");
        printf("  This program is for stress testing constant non-enveloped forces on\n");
        printf("  a force feedback device via the event interface. It simulates a\n");
        printf("  moving spring force by a frequently updated constant force effect.\n");
        printf("Usage:\n");
        printf("  %s <option> [<option>...]\n",argv[0]);
        printf("Options:\n");
        //printf("  -d <string>  device name (default: %s)\n",DEFAULT_DEVICE_NAME);
        //printf("  -u <double>  update rate in Hz (default: %.2f)\n",DEFAULT_UPDATE_RATE);
        //printf("  -f <double>  spring center motion frequency in Hz (default: %.2f)\n",DEFAULT_MOTION_FREQUENCY);
        //printf("  -a <double>  spring center motion amplitude 0.0..1.0 (default: %.2f)\n",DEFAULT_MOTION_AMPLITUDE);
        //printf("  -s <double>  spring strength factor (default: %.2f)\n",DEFAULT_SPRING_STRENGTH);
        printf("  -o           dummy option (useful because at least one option is needed)\n");
        exit(1);
    }
}

int rf2_init(struct Map* map, RF2Map* rf2map)
{
    CreateRF2Map(map, rf2map, 1);
    return 0;
}

int ac_init(struct Map* map, ACMap* acmap)
{
    CreateACMap(map, acmap, 1);
    return 0;
}

int main(int argc, char* argv[])
{
    /* Parse command line arguments */
    parse_args(argc,argv);

    printf(variable_name);

#ifdef ASSETTOCORSA
#define MAPSIZE ACMAP_SIZE
    struct Map* map = (struct Map*) malloc((ACMAP_SIZE) * sizeof(struct Map));
    void* struct1;
    int datasize1;
    struct SPageFilePhysics* spfp = malloc(sizeof(struct SPageFilePhysics));
    struct SPageFileGraphic* spfg = malloc(sizeof(struct SPageFileGraphic));
    struct SPageFileStatic* spfs = malloc(sizeof(struct SPageFileStatic));
    struct SPageFileCrewChief* spfc = malloc(sizeof(struct SPageFileCrewChief));


    if (strcmp(mem_file, AC_PHYSICS_FILE) == 0)
    {
        struct1 = spfp;
        datasize1 = sizeof(struct SPageFilePhysics);
    }
    else if (strcmp(mem_file, AC_GRAPHIC_FILE) == 0)
    {
        struct1 = spfg;
        datasize1 = sizeof(struct SPageFileGraphic);
    }
    else if (strcmp(mem_file, AC_STATIC_FILE) == 0)
    {
        struct1 = spfs;
        datasize1 = sizeof(struct SPageFileStatic);
    }
    else if (strcmp(mem_file, AC_CREWCHIEF_FILE) == 0)
    {
        struct1 = spfc;
        datasize1 = sizeof(struct SPageFileCrewChief);
    }
    else
    {
        printf("Unknown memory mapped file name");
    }


    ACMap* acmap = malloc(sizeof(ACMap));
    acmap->ac_physics = *spfp;
    acmap->ac_graphic = *spfg;
    acmap->ac_static = *spfs;
    acmap->ac_crewchief = *spfc;
    acmap->physics_map_addr = spfp;
    acmap->graphic_map_addr = spfg;
    acmap->static_map_addr = spfs;
    acmap->crewchief_map_addr = spfc;

    ac_init(map, acmap);
#endif
#ifdef RFACTOR2
#define MAPSIZE RF2MAP_SIZE
    struct Map* map = (struct Map*) malloc((RF2MAP_SIZE) * sizeof(struct Map));
    void* struct1;
    int datasize1;
    struct rF2Telemetry* rf2t = malloc(sizeof(struct rF2Telemetry));
    struct rF2Scoring* rf2s = malloc(sizeof(struct rF2Scoring));

    if (strcmp(mem_file, RF2_TELEMETRY_FILE) == 0)
    {
        struct1 = rf2t;
        datasize1 = sizeof(struct rF2Telemetry);
    }
    else if (strcmp(mem_file, RF2_SCORING_FILE) == 0)
    {
        struct1 = rf2s;
        datasize1 = sizeof(struct rF2Scoring);
    }
    else
    {
        printf("Unknown memory mapped file name");
    }

    RF2Map* rf2map = malloc(sizeof(RF2Map));
    rf2map->rf2_telemetry = *rf2t;
    rf2map->rf2_scoring = *rf2s;
    rf2map->telemetry_map_addr = rf2t;
    rf2map->scoring_map_addr = rf2s;

    rf2_init(map, rf2map);
#endif

    int fd = shm_open(mem_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        printf("open");
        return 10;
    }
    int res = ftruncate(fd, datasize1);
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    void* addr = mmap(NULL, datasize1, PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }

    memcpy(struct1, addr, datasize1);

    if (strcmp(action,"loadfile") == 0)
    {

        FILE* fp;
        fp=fopen(save_file, "r");
        fread(addr, datasize1, 1, fp);
        fclose(fp);
        goto cleanup;
    }

    if (strcmp(action,"dumpfile") == 0)
    {
        FILE* fp;
        fp=fopen(save_file, "w+");
        fwrite(struct1, datasize1, 1, fp);
        fclose(fp);
        goto cleanup;
    }

    void* addr2;
    for (int k = 0; k < MAPSIZE; k++)
    {
        if ( map[k].name == NULL )
        {
            continue;
        }
        if (strcmp(map[k].name, variable_name) == 0)
        {
            addr2 = map[k].value;
            break;
        }
    }

    if (strcmp( data_type, "float" ) == 0 )
    {
        float b;
        b = *(float*) (char*) addr2;
        float new = strtof( valuetoset, NULL );
        printf("current value as float %f\n", b);
        memcpy(addr2, &new, sizeof(new));
    }
    else if(strcmp( data_type, "double" ) == 0 )
    {
        double b;
        b = *(double*) (char*) addr2;
        double new = strtod( valuetoset, NULL );
        printf("current value as double %f\n", b);
        memcpy(addr2, &new, sizeof(new));
    }
    else if(strcmp( data_type, "integer" ) == 0 )
    {
        int b;
        b = *(int*) (char*) addr2;
        int new = atoi( valuetoset );
        printf("current value as integer %i\n", b);
        memcpy(addr2, &new, sizeof(new));
    }
    else
    {
        printf("Unknown data type");
    }

    memcpy(addr, struct1, datasize1);
    //printf("set to value %f\n", *(float*) (char*) addr2);
cleanup:
    free(map);
    free(struct1);
    return 0;
}
