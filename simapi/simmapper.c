#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>

#include "simdata.h"
#include "simapi.h"
#include "simmapper.h"
#include "test.h"
#include "ac.h"
#include "rf2.h"
#include "pcars2.h"
#include "scs2.h"

#include <sys/stat.h>
#include <sys/types.h>
#if defined(OS_WIN)
    #include <windows.h>
#else
    #include <dirent.h> // for *Nix directory access
    #include <unistd.h>
#endif

#include "getpid.h"
#include "../include/acdata.h"
#include "../include/rf2data.h"
#include "../include/pcars2data.h"
#include "../include/scs2data.h"

struct _simmap
{
    void* addr;
    int fd;
    union
    {
        ACMap ac;
        RF2Map rf2;
        PCars2Map pcars2;
        SCS2Map scs2;
    } d;
};

SimMap* createSimMap() {
    SimMap* ptr = malloc(sizeof(SimMap));
    ptr->fd = -1;
    ptr->addr = 0;
    return ptr;
}

void* getSimMapPtr(SimMap* simmap)
{
    return simmap->addr;
}

int sstrcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

int simapi_strtogame(const char* game)
{
    int sim = 0;
    if (sstrcicmp(game, "ac") == 0)
    {
        sim = SIMULATOREXE_ASSETTO_CORSA;
    }
    else if (sstrcicmp(game, "acc") == 0)
    {
        sim = SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE;
    }
    else if (sstrcicmp(game, "ace") == 0)
    {
        sim = SIMULATOREXE_ASSETTO_CORSA_EVO;
    }
    else if (sstrcicmp(game, "rf2") == 0)
    {
        sim = SIMULATOREXE_RFACTOR2;
    }
    else if (sstrcicmp(game, "ams2") == 0)
    {
        sim = SIMULATOREXE_AUTOMOBILISTA2;
    }
    else if (sstrcicmp(game, "et2") == 0)
    {
        sim = SIMULATOREXE_EUROTRUCKS2;
    }
    else if (sstrcicmp(game, "at") == 0)
    {
        sim = SIMULATOREXE_EUROTRUCKS2;
    }
    else if (sstrcicmp(game, "lmu") == 0)
    {
        sim = SIMULATOREXE_LEMANS_ULTIMATE;
    }
    else
    {
        sim = 0;
    }
    return sim;
}

char* simapi_gametostr(SimulatorEXE sim)
{
    switch ( sim )
    {
        case SIMULATOREXE_ASSETTO_CORSA:
            return "ac";
        case SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE:
            return "acc";
        case SIMULATOREXE_ASSETTO_CORSA_EVO:
            return "ace";
        case SIMULATOREXE_RFACTOR2:
            return "rf2";
        case SIMULATOREXE_LEMANS_ULTIMATE:
            return "lmu";
        case SIMULATOREXE_AUTOMOBILISTA2:
            return "ams2";
        case SIMULATOREXE_AMERICANTRUCKS:
            return "at";
        case SIMULATOREXE_EUROTRUCKS2:
            return "et2";
        default:
            return "default";
    }
}


int droundint(double d)
{
    return trunc(nearbyint(d));
}

int froundint(float f)
{
    return trunc(nearbyint(f));
}

func_ptr_t loginfo = NULL;
func_ptr_t logdebug = NULL;
func_ptr_t logtrace = NULL;

void set_simapi_log_info(func_ptr_t func) {
    loginfo = func;
}
void set_simapi_log_debug(func_ptr_t func) {
    logdebug = func;
}
void set_simapi_log_trace(func_ptr_t func) {
    logtrace = func;
}

void simapi_log(SIMAPI_LOGLEVEL sll, char* message)
{
    if(message != NULL)
    {
        if(sll == SIMAPI_LOGLEVEL_INFO)
        {
            if(loginfo != NULL)
            {
                loginfo(message);
            }
        }
        if(sll == SIMAPI_LOGLEVEL_DEBUG)
        {
            if(logdebug != NULL)
            {
                logdebug(message);
            }
        }
        if(sll == SIMAPI_LOGLEVEL_TRACE)
        {
            if(logtrace != NULL)
            {
                logtrace(message);
            }
        }
    }
}

void SetProximityData(SimData* simdata, int cars)
{
    double carwidth = 1.8;
    double maxradius = 10.0;

    int proxcars = PROXCARS;
    if(cars < PROXCARS)
    {
        proxcars = cars;
    }


    for(int x = 0; x < proxcars; x++)
    {
        simdata->pd[x].radius = 0.0;
        simdata->pd[x].theta = 0.0;
    }
    for(int car = 1; car < cars; car++)
    {
        double rawXCoordinate = simdata->cars[car].xpos - simdata->worldposx;
        double rawYCoordinate = simdata->cars[car].ypos - simdata->worldposy;

        double ff = simdata->tyrecontact0[0];
        double lf = simdata->tyrecontact2[0];
        double fr = simdata->tyrecontact0[2];
        double lr = simdata->tyrecontact2[2];

        double f = ff - fr;
        double l = lf - lr;

        double mag1 = sqrt((f*f) + (l*l));

        double negx = -(f / mag1);
        double negy = -(l / mag1);

        double angle = atan2(-1, 0) - atan2(negy, negx);
        double angleD = angle * 360 / (2 * M_PI);
        double angleR = angleD * M_PI / 180;

        double cosTheta = cos(angleR);
        double sinTheta = sin(angleR);
        double xscore  = cosTheta * rawXCoordinate - sinTheta * rawYCoordinate;
        double yscore  = sinTheta * rawXCoordinate + cosTheta * rawYCoordinate;

        double rads = atan2(-yscore, -xscore);
        double degrees = (rads * (180 / M_PI)) + 90.0;

        double radius = sqrt((rawXCoordinate * rawXCoordinate) + (rawYCoordinate * rawYCoordinate));
        double theta = degrees;
        if( theta < 0 )
        {
            theta = 360 + degrees;
        }
        radius = radius - carwidth;

        //fprintf(stderr, "rawx: %f, rawy: %f degs: %f mag: %f\n", rawXCoordinate, rawYCoordinate, theta, radius);

        if(radius < maxradius)
        {
            int j = proxcars - 1;
            if((simdata->pd[j].radius == 0) || (radius < simdata->pd[j].radius))
            {
                simdata->pd[j].radius = radius;
                simdata->pd[j].theta = theta;


                j--;
                while(j >= 0)
                {
                    double tempradius = simdata->pd[j+1].radius;
                    double temptheta = simdata->pd[j+1].theta;


                    if((simdata->pd[j].radius == 0) || (tempradius < simdata->pd[j].radius))
                    {
                        simdata->pd[j+1].radius = simdata->pd[j].radius;
                        simdata->pd[j+1].theta = simdata->pd[j].theta;

                        simdata->pd[j].radius = tempradius;
                        simdata->pd[j].theta = temptheta;
                    }
                    j--;
                }
            }
        }
    }
}


// probably going to move functions like this to ac.h
LapTime ac_convert_to_simdata_laptime(int ac_laptime)
{
    LapTime l;
    l.minutes = ac_laptime/60000;
    l.seconds = ac_laptime/1000-(l.minutes*60);
    l.fraction = ac_laptime-(l.minutes*60000)-(l.seconds*1000);
    return l;
}

LapTime rf2_convert_to_simdata_laptime(double rf2_laptime)
{
    if(rf2_laptime <= 0)
    {
        return (LapTime){0, 0, 0};
    }
    LapTime l;
    l.hours = rf2_laptime/60/60;
    l.minutes = rf2_laptime/60-(l.hours*60);
    l.seconds = rf2_laptime-(l.minutes*60);
    l.fraction = (rf2_laptime*1000)-(l.minutes*60000)-(l.seconds*1000);
    return l;
}

LapTime pcars2_convert_to_simdata_laptime(float pcars2_laptime)
{
    if(pcars2_laptime <= 0)
    {
        return (LapTime){0, 0, 0};
    }
    LapTime l;
    l.seconds = floor(pcars2_laptime);
    float a = pcars2_laptime-l.seconds;
    l.fraction = a * 1000;
    l.hours = l.seconds/60/60;
    l.minutes = l.seconds/60-(l.hours*60);
    l.seconds = l.seconds-(l.minutes*60);
    return l;
}

int rf2_phase_to_simdata_flag(int rf2_flag)
{

    int courseflag = 0;
    if(rf2_flag < 5 || rf2_flag > 8)
    {
        courseflag = 0;
    }
    else
    {
        courseflag = rf2_flag - 5;
    }

    return courseflag;
}

int rf2_flag_to_simdata_flag(int rf2_flag)
{

    int playerflag = 0;
    if(rf2_flag == 6)
    {
        playerflag = 4;
    }

    return playerflag;
}


int pcars2_session_to_simdata_session(int pcars2_session_state)
{
    switch ( pcars2_session_state )
    {
        case SESSION_PRACTICE:
            return AC_PRACTICE;
        case SESSION_QUALIFY:
            return AC_QUALIFY;
        case SESSION_RACE:
            return AC_RACE;
        case SESSION_TIME_ATTACK:
            return AC_TIME_ATTACK;
        default:
          return AC_UNKNOWN;
    }
}

int pcars2_gamestate_to_simdata_simstatus(int pcars2_gamestate) {
    switch ( pcars2_gamestate ) {
        case GAME_EXITED:
            return AC_OFF;
        case GAME_INGAME_PAUSED:
            return AC_PAUSE;
        case GAME_INGAME_REPLAY:
            return AC_REPLAY;
        case GAME_FRONT_END_REPLAY:
            return AC_REPLAY;
        default:
            return AC_LIVE;
    }
}

int pcars2_flag_to_simdata_flag(int pcars2_flag)
{
    switch ( pcars2_flag )
    {
        case FLAG_COLOUR_NONE:
            return SIMAPI_FLAG_GREEN;
        case FLAG_COLOUR_GREEN:
            return SIMAPI_FLAG_GREEN;
        case FLAG_COLOUR_BLUE:
            return SIMAPI_FLAG_BLUE;
        case FLAG_COLOUR_WHITE_SLOW_CAR:
            return SIMAPI_FLAG_WHITE;
        case FLAG_COLOUR_WHITE_FINAL_LAP:
            return SIMAPI_FLAG_WHITE;
        case FLAG_COLOUR_RED:
            return SIMAPI_FLAG_RED;
        case FLAG_COLOUR_YELLOW:
            return SIMAPI_FLAG_YELLOW;
        case FLAG_COLOUR_DOUBLE_YELLOW:
            return SIMAPI_FLAG_YELLOW;
        case FLAG_COLOUR_BLACK_AND_WHITE:
            return SIMAPI_FLAG_BLACK_WHITE;
        case FLAG_COLOUR_BLACK_ORANGE_CIRCLE:
            return SIMAPI_FLAG_BLACK_ORANGE;
        case FLAG_COLOUR_BLACK:
            return SIMAPI_FLAG_BLACK;
        case FLAG_COLOUR_CHEQUERED:
            return SIMAPI_FLAG_CHEQUERED;
        default:
            return SIMAPI_FLAG_GREEN;
    }
}

int pcars2_state_to_simdata_flag(int pcars2_state)
{
    if(pcars2_state == 8)
    {
        return SIMAPI_FLAG_RED;
    }
    if(pcars2_state > 0)
    {
        return SIMAPI_FLAG_YELLOW;
    }
    return SIMAPI_FLAG_GREEN;
}


int acc_flag_to_simdata_flag(int ac_flag)
{

    switch ( ac_flag )
    {
        case ACC_NO_FLAG:
            return SIMAPI_FLAG_GREEN;
        case ACC_GREEN_FLAG:
            return SIMAPI_FLAG_GREEN;
        case ACC_BLUE_FLAG:
            return SIMAPI_FLAG_BLUE;
        case ACC_WHITE_FLAG:
            return SIMAPI_FLAG_WHITE;
        case ACC_YELLOW_FLAG:
            return SIMAPI_FLAG_YELLOW;
        case ACC_PENALTY_FLAG:
            return SIMAPI_FLAG_BLACK_WHITE;
        case ACC_BLACK_FLAG:
            return SIMAPI_FLAG_BLACK;
        case ACC_CHECKERED_FLAG:
            return SIMAPI_FLAG_CHEQUERED;
        case ACC_ORANGE_FLAG:
            return SIMAPI_FLAG_ORANGE;
        default:
            return SIMAPI_FLAG_GREEN;
    }
}

int acc_get_global_flag(int yellow, int white, int chequered, int green, int red)
{
    if(chequered > 0)
        return SIMAPI_FLAG_CHEQUERED;
    if(red > 0)
        return SIMAPI_FLAG_RED;
    if(yellow > 0)
        return SIMAPI_FLAG_YELLOW;
    if(white > 0)
        return SIMAPI_FLAG_WHITE;

    return SIMAPI_FLAG_GREEN;
}

bool does_sim_file_exist(const char* file)
{
    if (file == NULL) { return false; }
    #if defined(OS_WIN)
        #if defined(WIN_API)
            // if you want the WinAPI, versus CRT
            if (strnlen(file, MAX_PATH+1) > MAX_PATH) {
                // ... throw error here or ...
                return false;
            }
            DWORD res = GetFileAttributesA(file);
            return (res != INVALID_FILE_ATTRIBUTES &&
                !(res & FILE_ATTRIBUTE_DIRECTORY));
        #else
            // Use Win CRT
            struct stat fi;
            if (_stat(file, &fi) == 0) {
                #if defined(S_ISSOCK)
                    // sockets come back as a 'file' on some systems
                    // so make sure it's not a socket or directory
                    // (in other words, make sure it's an actual file)
                    return !(S_ISDIR(fi.st_mode)) &&
                        !(S_ISSOCK(fi.st_mode));
                #else
                    return !(S_ISDIR(fi.st_mode));
                #endif
            }
            return false;
        #endif
    #else
        struct stat fi;
        if (stat(file, &fi) == 0) {
            #if defined(S_ISSOCK)
                return !(S_ISDIR(fi.st_mode)) &&
                    !(S_ISSOCK(fi.st_mode));
            #else
                return !(S_ISDIR(fi.st_mode));
            #endif
        }
        return false;
    #endif
}


// if this becomes more necessary i will move it into it's own file
float spLineLengthToDistanceRoundTrack(float trackLength, float spLine)
{
    if (spLine < 0.0)
    {
        spLine -= 1;
    }
    return spLine * trackLength;
}

int setSimInfo(SimInfo* si)
{
    switch ( si->simulatorapi )
    {

        case SIMULATORAPI_ASSETTO_CORSA :
            si->SimUsesUDP = false;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = true;
            si->SimSupportsAdvancedUI = true;
            break;
        case SIMULATORAPI_RFACTOR2 :
            si->SimUsesUDP = false;
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = false;
            si->SimSupportsAdvancedUI = true;
        case SIMULATORAPI_PROJECTCARS2 :
            si->SimSupportsBasicTelemetry = true;
            si->SimSupportsTyreEffects = true;
            si->SimSupportsRealtimeTelemetry = false;
            si->SimSupportsAdvancedUI = true;
        case SIMULATORAPI_SCSTRUCKSIM2 :
            si->SimUsesUDP = false;
            si->SimSupportsBasicTelemetry = true;
        default:
            si->SimSupportsBasicTelemetry = true;
    }

    return 0;
}

void hexDump(char *desc, void *addr, int len)
{
  int i;
  unsigned char buff[17];
  unsigned char *pc = (unsigned char*)addr;

  // Output description if given.
  if (desc != NULL)
    printf ("%s:\n", desc);

  // Process every byte in the data.
  for (i = 0; i < len; i++) {
    // Multiple of 16 means new line (with line offset).
    if ((i % 16) == 0) {
      // Just don't print ASCII for the zeroth line.
      if (i != 0)
        printf ("  %s\n", buff);

      // Output the offset.
      printf ("  %04x ", i);
    }

    // Now the hex code for the specific character.
    printf (" %02x", pc[i]);

    // And store a printable ASCII character for later.
    if ((pc[i] < 0x20) || (pc[i] > 0x7e))
      buff[i % 16] = '.';
    else
      buff[i % 16] = pc[i];
    buff[(i % 16) + 1] = '\0';
  }

  // Pad out last line if not exactly 16 characters.
  while ((i % 16) != 0) {
    printf ("   ");
    i++;
  }

  // And print the final ASCII bit.
  printf ("  %s\n", buff);
}

SimulatorEXE getSimExe()
{
    if(IsProcessRunning(AC_EXE)==true)
    {
        return SIMULATOREXE_ASSETTO_CORSA;
    }
    if(IsProcessRunning(ACC_EXE)==true)
    {
        return SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE;
    }
    if(IsProcessRunning(ACE_EXE)==true)
    {
        return SIMULATOREXE_ASSETTO_CORSA_EVO;
    }
    if(IsProcessRunning(RFACTOR2_EXE)==true)
    {
        return SIMULATOREXE_RFACTOR2;
    }
    if(IsProcessRunning(AMS2_EXE)==true)
    {
        return SIMULATOREXE_AUTOMOBILISTA2;
    }
    if(IsProcessRunning(EUROTRUCKS2_EXE)==true)
    {
        return SIMULATOREXE_EUROTRUCKS2;
    }
    if(IsProcessRunning(AMERICANTRUCKS_EXE)==true)
    {
        return SIMULATOREXE_AMERICANTRUCKS;
    }
    if(IsProcessRunning(LEMANS_ULTIMATE_EXE)==true)
    {
        return SIMULATOREXE_LEMANS_ULTIMATE;
    }
    return SIMULATOREXE_SIMAPI_TEST_NONE;
}


SimInfo getSim(SimData* simdata, SimMap* simmap, bool force_udp, int (*setup_udp)(int), bool simd)
{

    SimInfo si;
    si.simulatorapi = -1;
    si.mapapi = -1;
    si.simulatorexe = 0;
    si.isSimOn = false;
    si.SimUsesUDP = false;
    si.SimSupportsBasicTelemetry = false;
    si.SimSupportsTyreEffects = false;
    si.SimSupportsRealtimeTelemetry = false;
    si.SimSupportsAdvancedUI = false;

    simapi_log(SIMAPI_LOGLEVEL_TRACE, "looking for running simulators");

    if(simd == false)
    {
        if (does_sim_file_exist("/dev/shm/SIMAPI.DAT"))
        {
            int e = siminit(simdata, simmap, SIMULATORAPI_SIMAPI_TEST);
            simdatamap(simdata, simmap, NULL, SIMULATORAPI_SIMAPI_TEST, false, NULL);
            char* temp;
            asprintf(&temp, "found running simapi daemon simint error %i", e);
            simapi_log(SIMAPI_LOGLEVEL_INFO, temp);
            free(temp);
            //simdatamap(simdata, simmap, NULL, SIMULATORAPI_SIMAPI_TEST, false, NULL);
            if(simdata->simapiversion == SIMAPI_VERSION)
            {
                if (simdata->simon == 1)
                {
                    simapi_log(SIMAPI_LOGLEVEL_TRACE, "status okay");
                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.simulatorexe = simdata->simexe;
                    si.mapapi = SIMULATORAPI_SIMAPI_TEST;
                    si.simulatorexe = simdata->simexe;
                    setSimInfo(&si);
                }
                return si;
            }
            else
            {
                char* temp;
                asprintf(&temp, "skipping sim api daemon due to version mismatch. Daemon Version: %i. App Version: %i", simdata->simapiversion, SIMAPI_VERSION);
                simapi_log(SIMAPI_LOGLEVEL_INFO, temp);
                free(temp);
            }
        }
    }

    SimulatorEXE simexe = getSimExe();

    switch ( simexe )
    {
        case SIMULATOREXE_ASSETTO_CORSA:
        case SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE:
        case SIMULATOREXE_ASSETTO_CORSA_EVO:
            simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Found running process for Assetto Corsa");
            if (does_sim_file_exist("/dev/shm/acpmf_physics"))
            {
                if (does_sim_file_exist("/dev/shm/acpmf_static"))
                {
                    simapi_log(SIMAPI_LOGLEVEL_DEBUG, "static and physics files found");
                    si.simulatorapi = SIMULATORAPI_ASSETTO_CORSA;
                    int error = siminit(simdata, simmap, SIMULATORAPI_ASSETTO_CORSA);
                    simdatamap(simdata, simmap, NULL, SIMULATORAPI_ASSETTO_CORSA, false, NULL);
                    if (error == 0 && simdata->simstatus > 1)
                    {
                        simapi_log(SIMAPI_LOGLEVEL_DEBUG, "AC Shared memory looks good");
                        simdata->simon = true;
                        simdata->simapi = SIMULATORAPI_ASSETTO_CORSA;
                        simdata->simexe = simexe;

                        si.isSimOn = true;
                        si.simulatorapi = simdata->simapi;
                        si.mapapi = si.simulatorapi;
                        si.simulatorexe = simdata->simexe;
                        setSimInfo(&si);
                        if(simexe == SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE)
                        {
                            // support will have to be revisited for this sim
                            si.SimSupportsRealtimeTelemetry = false;
                        }

                        return si;
                    }
                }
                else
                {
                    simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Could not find static shared memory file");
                }
            }
            else
            {
                simapi_log(SIMAPI_LOGLEVEL_DEBUG, "Could not find physics shared memory file");
            }
            break;


        case SIMULATOREXE_RFACTOR2:
        case SIMULATOREXE_LEMANS_ULTIMATE:
            if (does_sim_file_exist("/dev/shm/$rFactor2SMMP_Telemetry$"))
            {
                simapi_log(SIMAPI_LOGLEVEL_DEBUG, "RFactor2 telemetry file found");
                si.simulatorapi = SIMULATORAPI_RFACTOR2;
                int error = siminit(simdata, simmap, SIMULATORAPI_RFACTOR2);
                simdatamap(simdata, simmap, NULL, SIMULATORAPI_RFACTOR2, false, NULL);
                if (error == 0)
                {
                    //slogi("found Assetto Corsa, starting application...");

                    simdata->simon = true;
                    simdata->simapi = SIMULATORAPI_RFACTOR2;
                    simdata->simexe = simexe;

                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.mapapi = si.simulatorapi;
                    si.simulatorexe = simdata->simexe;
                    setSimInfo(&si);

                    return si;
                }
            }
            break;

        case SIMULATOREXE_AUTOMOBILISTA2:
            if (force_udp == false)
            {
                if (does_sim_file_exist("/dev/shm/$pcars2$"))
                {
                    si.simulatorapi = SIMULATORAPI_PROJECTCARS2;
                    int error = siminit(simdata, simmap, SIMULATORAPI_PROJECTCARS2);
                    simdatamap(simdata, simmap, NULL, SIMULATORAPI_PROJECTCARS2, false, NULL);
                    if (error == 0 && simdata->simstatus > 1)
                    {
                        simdata->simon = true;
                        simdata->simapi = SIMULATORAPI_PROJECTCARS2;

                        si.isSimOn = true;
                        si.simulatorapi = simdata->simapi;
                        si.mapapi = si.simulatorapi;
                        si.simulatorexe = simdata->simexe;
                        setSimInfo(&si);

                        return si;
                    }
                }
            }
            else
            {
                int error = (*setup_udp)(5606);
                error = siminitudp(simdata, simmap, SIMULATORAPI_PROJECTCARS2);
                if (error == 0)
                {
                    si.simulatorapi = SIMULATORAPI_PROJECTCARS2;
                    si.SimUsesUDP = true;
                    simdatamap(simdata, simmap, NULL, SIMULATORAPI_PROJECTCARS2, true, NULL);
                }
                if (error == 0 && simdata->simstatus > 1)
                {
                    simdata->simon = true;
                    simdata->simapi = SIMULATORAPI_PROJECTCARS2;
                    simdata->simexe = simexe;

                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.mapapi = si.simulatorapi;
                    si.simulatorexe = simdata->simexe;
                    setSimInfo(&si);

                    return si;
                }
            }
            break;
        case SIMULATOREXE_EUROTRUCKS2:
        case SIMULATOREXE_AMERICANTRUCKS:
            if (does_sim_file_exist("/dev/shm/SCS/SCSTelemetry"))
            {
                si.simulatorapi = SIMULATORAPI_SCSTRUCKSIM2;
                int error = siminit(simdata, simmap, SIMULATORAPI_SCSTRUCKSIM2);
                simdatamap(simdata, simmap, NULL, SIMULATORAPI_SCSTRUCKSIM2, false, NULL);
                if (error == 0)
                {
                    simdata->simon = true;
                    simdata->simapi = SIMULATORAPI_SCSTRUCKSIM2;
                    simdata->simexe = simexe;

                    si.isSimOn = true;
                    si.simulatorapi = simdata->simapi;
                    si.mapapi = si.simulatorapi;
                    si.simulatorexe = simdata->simexe;
                    setSimInfo(&si);

                    return si;
                }
            }
            break;
    }
    return si;
}

int simdatamap(SimData* simdata, SimMap* simmap, SimMap* simmap2, SimulatorAPI simulator, bool udp, char* base)
{
    char* a;
    char* b;
    char* c;
    char* d;

    switch ( simulator )
    {
        case SIMULATORAPI_SIMAPI_TEST :
            double tyre0 = simdata->tyrediameter[0];
            double tyre1 = simdata->tyrediameter[1];
            double tyre2 = simdata->tyrediameter[2];
            double tyre3 = simdata->tyrediameter[3];
            memcpy(simdata, simmap->addr, sizeof(SimData));
            simdata->tyrediameter[0] = tyre0;
            simdata->tyrediameter[1] = tyre1;
            simdata->tyrediameter[2] = tyre2;
            simdata->tyrediameter[3] = tyre3;
            return 0;
        case SIMULATORAPI_ASSETTO_CORSA :

            a = simmap->d.ac.physics_map_addr;

            // basic telemetry
            simdata->rpms = *(uint32_t*) (char*) (a + offsetof(struct SPageFilePhysics, rpms));
            simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct SPageFilePhysics, gear));
            simdata->velocity = droundint( *(float*) (char*) (a + offsetof(struct SPageFilePhysics, speedKmh)));

            simdata->gas = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, gas));
            simdata->clutch = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, clutch));
            simdata->steer = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, steerAngle));
            simdata->brake = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brake));
            simdata->heading = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, heading) + sizeof(float));
            simdata->brakebias = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeBias));
            simdata->handbrake = 0;
            simdata->fuel = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, fuel));
            simdata->gearc[0] = simdata->gear + 47;
            if (simdata->gear == 0)
            {
                simdata->gearc[0] = 82;
            }
            if (simdata->gear == 1)
            {
                simdata->gearc[0] = 78;
            }
            simdata->gearc[1] = 0;


            // tyre effects
            simdata->abs = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, abs));
            simdata->tyreRPS[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelAngularSpeed) + (sizeof(float) * 0));
            simdata->tyreRPS[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelAngularSpeed) + (sizeof(float) * 1));
            simdata->tyreRPS[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelAngularSpeed) + (sizeof(float) * 2));
            simdata->tyreRPS[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelAngularSpeed) + (sizeof(float) * 3));

            simdata->Xvelocity = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, localVelocity) + (sizeof(float) * 0 ));
            simdata->Zvelocity = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, localVelocity) + (sizeof(float) * 1 ));
            simdata->Yvelocity = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, localVelocity) + (sizeof(float) * 2 ));

            simdata->suspension[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, suspensionTravel) + (sizeof(float) * 0));
            simdata->suspension[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, suspensionTravel) + (sizeof(float) * 1));
            simdata->suspension[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, suspensionTravel) + (sizeof(float) * 2));
            simdata->suspension[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, suspensionTravel) + (sizeof(float) * 3));


            //advanced ui
            simdata->tyrewear[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreWear) + (sizeof(float) * 0));
            simdata->tyrewear[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreWear) + (sizeof(float) * 1));
            simdata->tyrewear[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreWear) + (sizeof(float) * 2));
            simdata->tyrewear[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreWear) + (sizeof(float) * 3));

            simdata->tyretemp[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreCoreTemperature) + (sizeof(float) * 0));
            simdata->tyretemp[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreCoreTemperature) + (sizeof(float) * 1));
            simdata->tyretemp[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreCoreTemperature) + (sizeof(float) * 2));
            simdata->tyretemp[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreCoreTemperature) + (sizeof(float) * 3));

            //simdata->tyrecontact0[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 0));
            //simdata->tyrecontact0[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 1));
            //simdata->tyrecontact0[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 2));
            //simdata->tyrecontact0[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 3));

            //simdata->tyrecontact1[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 4) + (sizeof(float) * 0));
            //simdata->tyrecontact1[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 4) + (sizeof(float) * 1));
            //simdata->tyrecontact1[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 4) + (sizeof(float) * 2));
            //simdata->tyrecontact1[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 4) + (sizeof(float) * 3));

            //simdata->tyrecontact2[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 8) + (sizeof(float) * 0));
            //simdata->tyrecontact2[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 8) + (sizeof(float) * 1));
            //simdata->tyrecontact2[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 8) + (sizeof(float) * 2));
            //simdata->tyrecontact2[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 8) + (sizeof(float) * 3));

            simdata->tyrecontact0[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 0));
            simdata->tyrecontact1[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 1));
            simdata->tyrecontact2[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 2));

            simdata->tyrecontact1[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 3) + (sizeof(float) * 0));
            simdata->tyrecontact1[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 3) + (sizeof(float) * 1));
            simdata->tyrecontact2[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 3) + (sizeof(float) * 2));

            simdata->tyrecontact0[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 6) + (sizeof(float) * 0));
            simdata->tyrecontact1[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 6) + (sizeof(float) * 1));
            simdata->tyrecontact2[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 6) + (sizeof(float) * 2));

            simdata->tyrecontact0[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 9) + (sizeof(float) * 0));
            simdata->tyrecontact1[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 9) + (sizeof(float) * 1));
            simdata->tyrecontact2[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, tyreContactPoint) + (sizeof(float) * 9) + (sizeof(float) * 2));

            simdata->braketemp[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeTemp) + (sizeof(float) * 0));
            simdata->braketemp[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeTemp) + (sizeof(float) * 1));
            simdata->braketemp[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeTemp) + (sizeof(float) * 2));
            simdata->braketemp[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, brakeTemp) + (sizeof(float) * 3));

            simdata->tyrepressure[0] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelsPressure) + (sizeof(float) * 0));
            simdata->tyrepressure[1] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelsPressure) + (sizeof(float) * 1));
            simdata->tyrepressure[2] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelsPressure) + (sizeof(float) * 2));
            simdata->tyrepressure[3] = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, wheelsPressure) + (sizeof(float) * 3));

            simdata->airdensity = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, airDensity));
            simdata->airtemp = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, airTemp));
            simdata->tracktemp = *(float*) (char*) (a + offsetof(struct SPageFilePhysics, roadTemp));


            if ( simmap->d.ac.has_graphic == true )
            {
                c = simmap->d.ac.graphic_map_addr;

                simdata->simstatus = *(int*) (char*) (c + offsetof(struct SPageFileGraphic, status));
                simdata->lap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, completedLaps));
                simdata->position = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, position));
                uint32_t lastlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iLastTime));
                simdata->lastlap = ac_convert_to_simdata_laptime(lastlap);
                uint32_t bestlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iBestTime));
                simdata->bestlap = ac_convert_to_simdata_laptime(bestlap);
                uint32_t currentlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iCurrentTime));
                simdata->currentlap = ac_convert_to_simdata_laptime(currentlap);
                //simdata->time = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iCurrentTime));
                simdata->numlaps = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, numberOfLaps));
                simdata->session = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, session));
                simdata->sectorindex = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, currentSectorIndex));
                simdata->lastsectorinms = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, lastSectorTime));
                simdata->playerflag = acc_flag_to_simdata_flag(*(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, Flag)));

                int yellow = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, GlobalYellow));
                int white = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, GlobalWhite));
                int chequered = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, GlobalChequered));
                int green = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, GlobalGreen));
                int red = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, GlobalRed));
                simdata->courseflag = acc_get_global_flag( yellow, white, chequered, green, red);

                int strsize = 32;
                for(int i=0; i<strsize; i++)
                {
                    simdata->tyrecompound[i] = *(char*) (char*) ((c + offsetof(struct SPageFileGraphic, tyreCompound)) + (sizeof(char16_t) * i));
                }
                //simdata->tyrecompound = simmap->d.ac.compound;

                //float timeleft = *(float*) (char*) (c + offsetof(struct SPageFileGraphic, sessionTimeLeft));
                //if (timeleft < 0)
                //    simdata->timeleft = 0;
                //else
                //    simdata->timeleft = droundint(timeleft);
            }

            if (simmap->d.ac.has_static == true )
            {
                b = simmap->d.ac.static_map_addr;
                simdata->maxrpm = *(uint32_t*) (char*) (b + offsetof(struct SPageFileStatic, maxRpm));

                int strsize = 32;
                for(int i=0; i<strsize; i++)
                {
                    simdata->car[i] = *(char*) (char*) ((b + offsetof(struct SPageFileStatic, carModel)) + (sizeof(char16_t) * i));
                    simdata->track[i] = *(char*) (char*) ((b + offsetof(struct SPageFileStatic, track)) + (sizeof(char16_t) * i));
                    simdata->driver[i] = *(char*) (char*) ((b + offsetof(struct SPageFileStatic, playerName)) + (sizeof(char16_t) * i));
                }

            }

            // realtime telemetry
            if (simmap->d.ac.has_crewchief == true && simdata->simexe != SIMULATOREXE_ASSETTO_CORSA_COMPETIZIONE )
            {
                d = simmap->d.ac.crewchief_map_addr;

                simdata->worldposx = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, x)));
                simdata->worldposz = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, y)));
                simdata->worldposy = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, z)));
                double heading = simdata->heading;
                double player_rotation = heading;


                float player_spline = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, spLineLength)));
                float track_spline = *(float*) (char*) (b + offsetof(struct SPageFileStatic, TrackSPlineLength));
                simdata->playerspline = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, spLineLength)));
                simdata->trackspline = *(float*) (char*) (b + offsetof(struct SPageFileStatic, TrackSPlineLength));
                simdata->trackdistancearound = spLineLengthToDistanceRoundTrack(track_spline, player_spline);
                int track_samples = track_spline * 4;
                simdata->tracksamples = track_samples;
                simdata->playertrackpos = (int) track_samples * player_spline;

                simdata->numcars = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, numVehicles));
                int numcars = simdata->numcars;
                if (numcars > MAXCARS)
                {
                    numcars = MAXCARS;
                }

                int strsize = 32;
                for(int i=0; i<numcars; i++)
                {
                    simdata->cars[i].lap = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, lapCount)));
                    simdata->cars[i].pos = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, carLeaderboardPosition)));
                    uint32_t lastlap = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, lastLapTimeMS)));
                    uint32_t bestlap = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, bestLapMS)));
                    simdata->cars[i].bestlap = ac_convert_to_simdata_laptime(bestlap);
                    simdata->cars[i].lastlap = ac_convert_to_simdata_laptime(lastlap);
                    //uint32_t currentlap = *(uint32_t*) (char*) (c + offsetof(struct SPageFileGraphic, iCurrentTime));
                    //simdata->currentlap = ac_convert_to_simdata_laptime(currentlap);
                    simdata->cars[i].inpitlane = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, isCarInPitline)));
                    simdata->cars[i].inpit = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, isCarInPit)));

                    int strsize = 32;
                    for(int k=0; k<strsize; k++)
                    {
                        simdata->cars[i].driver[k] = *(char*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, driverName)) + (sizeof(char) * k));
                        simdata->cars[i].car[k] = *(char*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, carModel)) + (sizeof(char) * k));
                    }
                    simdata->cars[i].speed = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, speedMS)));

                    //simdata->cars[i].carspline = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, spLineLength)));
                    float spline = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, spLineLength)));
                    simdata->cars[i].trackpos = (int) track_samples * spline;
                    simdata->cars[i].xpos = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, x)));
                    simdata->cars[i].zpos = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, y)));
                    simdata->cars[i].ypos = *(float*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * i) + offsetof(acsVehicleInfo, worldPosition) + offsetof(acsVec3, z)));
                }
                SetProximityData(simdata, numcars);

                simdata->playerlaps = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, lapCount)));
                simdata->lapisvalid = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, currentLapInvalid)));
                simdata->lapisvalid = !simdata->lapisvalid;

                int currentlapinticks = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, currentLapTimeMS)));
                int lastlapinticks = *(uint32_t*) (char*) (d + offsetof(struct SPageFileCrewChief, vehicle) + ((sizeof(acsVehicleInfo) * 0) + offsetof(acsVehicleInfo, lastLapTimeMS)));
                simdata->currentlapinseconds = currentlapinticks * 100000;
                simdata->lastlapinseconds = lastlapinticks * 100000;
            }

            simdata->altitude = 1;
            break;

        case SIMULATORAPI_RFACTOR2 :

            a = simmap->d.rf2.telemetry_map_addr;

            // basic telemetry
            simdata->simstatus = 2;
            simdata->velocity = abs(droundint(3.6 * (*(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + ((sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mLocalVel)) + (sizeof(double) * 2)))));
            simdata->rpms = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + ((sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mEngineRPM)));
            simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + ((sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mGear)));
            simdata->maxrpm = droundint( *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + ((sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mEngineMaxRPM))));
            simdata->gas = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mUnfilteredThrottle));
            simdata->brake = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mUnfilteredBrake));
            simdata->clutch = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mUnfilteredClutch));
            simdata->steer = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mUnfilteredSteering));
            simdata->fuel = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mFuel));
            simdata->brakebias = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * 0) + offsetof(rF2VehicleTelemetry, mRearBrakeBias));
            simdata->handbrake = 0;
            simdata->altitude = 1;

            simdata->gearc[0] = simdata->gear + 48;
            if (simdata->gear < 0)
            {
                simdata->gearc[0] = 82;
            }
            if (simdata->gear == 0)
            {
                simdata->gearc[0] = 78;
            }
            simdata->gearc[1] = 0;
            simdata->gear += 1;

            // tyre effects
            //simdata->abs = *(float*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles));
            simdata->tyreRPS[0] = -1 * *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mRotation) + (sizeof(TelemWheelV01) * 0)));
            simdata->tyreRPS[1] = -1 * *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mRotation) + (sizeof(TelemWheelV01) * 1)));
            simdata->tyreRPS[2] = -1 * *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mRotation) + (sizeof(TelemWheelV01) * 2)));
            simdata->tyreRPS[3] = -1 * *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mRotation) + (sizeof(TelemWheelV01) * 3)));

            simdata->Xvelocity = -1 * *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mLocalVel) + (sizeof(double) * 0 ));
            simdata->Zvelocity = -1 * *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mLocalVel) + (sizeof(double) * 1 ));
            simdata->Yvelocity = -1 * *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mLocalVel) + (sizeof(double) * 2 ));

            //advanced ui
            if (simmap->d.rf2.has_scoring == true )
            {
                b = simmap->d.rf2.scoring_map_addr;

                simdata->tyrewear[0] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mWear) + (sizeof(TelemWheelV01) * 0)));
                simdata->tyrewear[1] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mWear) + (sizeof(TelemWheelV01) * 1)));
                simdata->tyrewear[2] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mWear) + (sizeof(TelemWheelV01) * 2)));
                simdata->tyrewear[3] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mWear) + (sizeof(TelemWheelV01) * 3)));

                simdata->tyretemp[0] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mTireCarcassTemperature) + (sizeof(TelemWheelV01) * 0)));
                simdata->tyretemp[1] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mTireCarcassTemperature) + (sizeof(TelemWheelV01) * 1)));
                simdata->tyretemp[2] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mTireCarcassTemperature) + (sizeof(TelemWheelV01) * 2)));
                simdata->tyretemp[3] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mTireCarcassTemperature) + (sizeof(TelemWheelV01) * 3)));

                for(int k = 0; k<4; k++)
                {
                    simdata->tyretemp[k] = simdata->tyretemp[k] - 273.15;
                }

                simdata->braketemp[0] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mBrakeTemp) + (sizeof(TelemWheelV01) * 0)));
                simdata->braketemp[1] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mBrakeTemp) + (sizeof(TelemWheelV01) * 1)));
                simdata->braketemp[2] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mBrakeTemp) + (sizeof(TelemWheelV01) * 2)));
                simdata->braketemp[3] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mBrakeTemp) + (sizeof(TelemWheelV01) * 3)));

                simdata->tyrepressure[0] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mPressure) + (sizeof(TelemWheelV01) * 0)));
                simdata->tyrepressure[1] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mPressure) + (sizeof(TelemWheelV01) * 1)));
                simdata->tyrepressure[2] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mPressure) + (sizeof(TelemWheelV01) * 2)));
                simdata->tyrepressure[3] = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mWheel) + (offsetof(TelemWheelV01, mPressure) + (sizeof(TelemWheelV01) * 3)));

                simdata->airtemp = *(double*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mAmbientTemp));
                simdata->tracktemp = *(double*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mTrackTemp));

                double trackdist = *(double*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mLapDist));
                double pos = *(double*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mLapDist));
                if(pos < 0)
                {
                    pos = (-1 * pos) + .5;
                }
                simdata->tracksamples = ceil(trackdist * 4);
                simdata->playerspline = (pos/trackdist);

                simdata->lap = *(uint32_t*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mLapNumber));
                simdata->lap++;
                simdata->position = *(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mPlace));

                simdata->lastlap = rf2_convert_to_simdata_laptime(*(double*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mLastLapTime)));
                simdata->bestlap = rf2_convert_to_simdata_laptime(*(double*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mBestLapTime)));
                simdata->currentlap = rf2_convert_to_simdata_laptime(*(double*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mTimeIntoLap)));


                simdata->numlaps = *(uint32_t*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mMaxLaps));
                if(simdata->numlaps == INT_MAX)
                {
                    simdata->numlaps = 0;
                }
                //simdata->session
                simdata->sectorindex = *(uint32_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mSector));
                //simdata->lastsectorinms
                simdata->playerflag = rf2_flag_to_simdata_flag(*(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mFlag)));
                simdata->courseflag = rf2_phase_to_simdata_flag(*(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2ScoringInfo, mGamePhase)));
                double z = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mElapsedTime));
                simdata->sessiontime = rf2_convert_to_simdata_laptime(*(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mElapsedTime)));

                // Car and Track
                size_t actsize = 0;
                size_t actsize2 = 0;
                int strsize = 64;
                for(int i=0; i<strsize; i++)
                {
                    simdata->car[i] = *(char*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mVehicleName) + (sizeof(char)*i));
                    //simdata->track[i] = *(char*) (char*) (b + offsetof(struct rF2Scoring, mScoringInfo) + offsetof(rF2ScoringInfo, mTrackName) + (sizeof(char)*i));
                    simdata->track[i] = *(char*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mTrackName) + (sizeof(char)*i));
                    if(simdata->car[i] != '\0')
                    {
                        actsize++;
                    }
                    if(simdata->track[i] != '\0')
                    {
                        actsize2++;
                    }
                }

                // Driver
                strsize = 32;
                actsize = 0;
                for(int i=0; i<strsize; i++)
                {
                    simdata->driver[i] = *(char*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mDriverName) + (sizeof(char)*i));
                    if(simdata->driver[i] != '\0')
                    {
                        actsize++;
                    }
                }
                simdata->driver[32] = '\0';

                //Tyre Compound
                strsize = 18;
                actsize = 0;
                for(int i=0; i<strsize; i++)
                {
                    simdata->tyrecompound[i] = *(char*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mRearTireCompoundName) + (sizeof(char)*i));
                    if(simdata->tyrecompound[i] != '\0')
                    {
                        actsize++;
                    }
                }

                simdata->numcars = *(uint32_t*) (char*) (a + offsetof(struct rF2Telemetry, mNumVehicles));
                int numcars = simdata->numcars;
                if (numcars > MAXCARS)
                {
                    numcars = MAXCARS;
                }
                for(int i=0; i<numcars; i++)
                {

                    simdata->cars[i].lap = *(uint32_t*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + (sizeof(rF2VehicleTelemetry) * i) + offsetof(rF2VehicleTelemetry, mLapNumber));
                    simdata->cars[i].pos = *(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * i) + offsetof(rF2VehicleScoring, mPlace));
                    uint8_t pitstate = *(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * i) + offsetof(rF2VehicleScoring, mPitState));
                    uint8_t garagestall = *(uint8_t*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * i) + offsetof(rF2VehicleScoring, mInGarageStall));
                    simdata->cars[i].ingarage = false;
                    simdata->cars[i].inpitstopped = false;
                    simdata->cars[i].inpitentrance = false;
                    simdata->cars[i].inpitexit = false;
                    if(pitstate == 2)
                    {
                        simdata->cars[i].inpitentrance = true;
                    }
                    if(pitstate == 3)
                    {
                        simdata->cars[i].inpitstopped = true;
                    }
                    if(pitstate == 4)
                    {
                        simdata->cars[i].inpitexit = true;
                    }
                    if(pitstate >= 2)
                    {
                        simdata->cars[i].inpit = true;
                    }
                    if(pitstate == 2 || pitstate == 4)
                    {
                        simdata->cars[i].inpitlane = true;
                    }
                    if(garagestall > 0)
                    {
                        simdata->cars[i].ingarage = true;
                    }
                    if(simdata->cars[i].ingarage == true)
                    {
                        simdata->cars[i].inpit = true;
                    }

                    simdata->cars[i].lastlap = rf2_convert_to_simdata_laptime(*(double*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * i) + offsetof(rF2VehicleScoring, mLastLapTime)));
                    simdata->cars[i].bestlap = rf2_convert_to_simdata_laptime(*(double*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + (sizeof(rF2VehicleScoring) * i) + offsetof(rF2VehicleScoring, mBestLapTime)));


                    strsize = 32;
                    actsize = 0;
                    for(int k=0; k<strsize; k++)
                    {
                        simdata->cars[i].driver[k] = *(char*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mDriverName) + (sizeof(rF2VehicleScoring) * i) + (sizeof(char)*k));
                        if(simdata->cars[i].driver[k] != '\0')
                        {
                            actsize++;
                        }
                    }

                    actsize = 0;
                    strsize = 64;
                    for(int k=0; k<strsize; k++)
                    {
                        simdata->cars[i].car[k] = *(char*) (char*) (b + offsetof(struct rF2Scoring, mVehicles) + offsetof(rF2VehicleScoring, mVehicleName) + (sizeof(rF2VehicleScoring) * i) + (sizeof(char)*k));
                        if(simdata->cars[i].driver[k] != '\0')
                        {
                            actsize++;
                        }
                    }
                }


                simdata->worldposx = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mPos) + (sizeof(double) * 0 ));
                simdata->worldposz = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mPos) + (sizeof(double) * 1 ));
                simdata->worldposy = *(double*) (char*) (a + offsetof(struct rF2Telemetry, mVehicles) + offsetof(rF2VehicleTelemetry, mPos) + (sizeof(double) * 2 ));
            }

            break;

        case SIMULATORAPI_PROJECTCARS2 :



            if(udp == false)
            {

                a = simmap->d.pcars2.telemetry_map_addr;
                // basic telemetry
                unsigned int gamestate = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mGameState));
                simdata->simstatus = pcars2_gamestate_to_simdata_simstatus(gamestate);
                simdata->velocity = droundint(3.6 * (*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mSpeed))));
                simdata->rpms = droundint(*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mRpm)));
                simdata->maxrpm = droundint(*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mMaxRPM)));
                simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mGear));
                simdata->fuel = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mFuelLevel));
                simdata->gas = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mThrottle));
                simdata->brake = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mBrake));
                simdata->steer = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mSteering));
                // bool
                simdata->abs = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mAntiLockActive));
                simdata->altitude = 1;

                simdata->gearc[0] = simdata->gear + 48;
                if (simdata->gear < 0)
                {
                    simdata->gearc[0] = 82;
                }
                if (simdata->gear == 0)
                {
                    simdata->gearc[0] = 78;
                }
                simdata->gearc[1] = 0;


                // tyre effects
                // m/s
                simdata->Xvelocity = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mLocalVelocity) + (sizeof(float) * 0 ));
                simdata->Zvelocity = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mLocalVelocity) + (sizeof(float) * 1 ));
                simdata->Yvelocity = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mLocalVelocity) + (sizeof(float) * 2 ));


                simdata->tyreRPS[0] = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreRPS) + (sizeof(float) * 0));
                simdata->tyreRPS[1] = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreRPS) + (sizeof(float) * 1));
                simdata->tyreRPS[2] = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreRPS) + (sizeof(float) * 2));
                simdata->tyreRPS[3] = -1 * *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreRPS) + (sizeof(float) * 3));

                simdata->tyrewear[0] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreWear) + (sizeof(float) * 0));
                simdata->tyrewear[1] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreWear) + (sizeof(float) * 1));
                simdata->tyrewear[2] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreWear) + (sizeof(float) * 2));
                simdata->tyrewear[3] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreWear) + (sizeof(float) * 3));

                simdata->tyretemp[0] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreInternalAirTemp) + (sizeof(float) * 0));
                simdata->tyretemp[1] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreInternalAirTemp) + (sizeof(float) * 1));
                simdata->tyretemp[2] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreInternalAirTemp) + (sizeof(float) * 2));
                simdata->tyretemp[3] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreInternalAirTemp) + (sizeof(float) * 3));

                simdata->braketemp[0] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mBrakeTempCelsius) + (sizeof(float) * 0));
                simdata->braketemp[1] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mBrakeTempCelsius) + (sizeof(float) * 1));
                simdata->braketemp[2] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mBrakeTempCelsius) + (sizeof(float) * 2));
                simdata->braketemp[3] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mBrakeTempCelsius) + (sizeof(float) * 3));

                simdata->tyrepressure[0] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mAirPressure) + (sizeof(float) * 0));
                simdata->tyrepressure[1] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mAirPressure) + (sizeof(float) * 1));
                simdata->tyrepressure[2] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mAirPressure) + (sizeof(float) * 2));
                simdata->tyrepressure[3] = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mAirPressure) + (sizeof(float) * 3));


                // advanced ui
                //simdata->airdensity = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, airDensity));
                simdata->airtemp = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mAmbientTemperature));
                simdata->tracktemp = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTrackTemperature));

                simdata->lap = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mCurrentLap));
                simdata->position = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mRacePosition));
                float lastlap = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mLastLapTime));
                float bestlap = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mBestLapTime));
                float currentlap = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mCurrentTime));
                simdata->lastlap = pcars2_convert_to_simdata_laptime(lastlap);
                simdata->bestlap = pcars2_convert_to_simdata_laptime(bestlap);
                simdata->currentlap = pcars2_convert_to_simdata_laptime(currentlap);

                int session = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mSessionState));
                simdata->session = pcars2_session_to_simdata_session(session);

                simdata->sectorindex = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mCurrentSector));
                if (simdata->sectorindex != 0)
                {
                    simdata->sectorindex -= 1;
                    float sectorinsecs = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mCurrentSector1Time) + (simdata->sectorindex * sizeof(float)));
                    simdata->lastsectorinms = (uint32_t) (sectorinsecs * 1000.0f);
                }
                else
                {
                    simdata->lastsectorinms = 0;
                }

                simdata->numlaps = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mLapsInEvent));
                simdata->lapisvalid = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mLapInvalidated));
                simdata->lapisvalid = !simdata->lapisvalid;
                simdata->courseflag = pcars2_state_to_simdata_flag(*(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mYellowFlagState)));
                simdata->playerflag = pcars2_flag_to_simdata_flag(*(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mHighestFlagColour)));

                int actsize = 0;
                int actsize2 = 0;
                int actsize3 = 0;
                int strsize = 64;
                for(int i=0; i<strsize; i++)
                {
                    simdata->car[i] = *(char*) (char*) (a + offsetof(struct pcars2APIStruct, mCarName) + (sizeof(char)*i));
                    simdata->track[i] = *(char*) (char*) (a + offsetof(struct pcars2APIStruct, mTrackLocation) + (sizeof(char)*i));
                    simdata->driver[i] = *(char*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mName) + (sizeof(char)*i));
                    if(simdata->car[i] != '\0')
                    {
                        actsize++;
                    }
                    if(simdata->track[i] != '\0')
                    {
                        actsize2++;
                    }
                    if(simdata->driver[i] != '\0')
                    {
                        actsize3++;
                    }
                }
                actsize = 0;
                strsize = 40;
                for(int i=0; i<strsize; i++)
                {
                    simdata->tyrecompound[i] = *(char*) (char*) (a + offsetof(struct pcars2APIStruct, mTyreCompound) + (sizeof(char)*i));
                    if(simdata->tyrecompound[i] != '\0')
                    {
                        actsize++;
                    }
                }

                simdata->numcars = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mNumParticipants));
                int numcars = simdata->numcars;
                if (numcars > MAXCARS)
                {
                    numcars = MAXCARS;
                }
                for(int i=0; i<numcars; i++)
                {
                    int actsize = 0;
                    int actsize2 = 0;
                    int strsize = 64;
                    for(int k=0; k<strsize; k++)
                    {
                        simdata->cars[i].driver[k] = *(char*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mName) + (sizeof(ParticipantInfo)*i) + (sizeof(char)*k));
                        simdata->cars[i].car[k] = *(char*) (char*) (a + offsetof(struct pcars2APIStruct, mCarNames) + STRING_LENGTH_MAX + ((sizeof(char) * k)));
                        if(simdata->cars[i].driver[k] != '\0')
                        {
                            actsize++;
                        }
                    }
                    simdata->cars[i].lap = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mCurrentLap) + (sizeof(ParticipantInfo)*i));
                    simdata->cars[i].pos = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mRacePosition) + (sizeof(ParticipantInfo)*i));
                    simdata->cars[i].lastlap = pcars2_convert_to_simdata_laptime(*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mLastLapTimes) + ((sizeof(float) * i))));
                    simdata->cars[i].bestlap = pcars2_convert_to_simdata_laptime(*(float*) (char*) (a + offsetof(struct pcars2APIStruct, mFastestLapTimes) + ((sizeof(float) * i))));

                    // TODO move to it's own function
                    simdata->cars[i].inpitlane = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mPitModes) + ((sizeof(float) * i)));
                    if(simdata->cars[i].inpitlane == 1 || simdata->cars[i].inpitlane == 3)
                    {
                        simdata->cars[i].inpitlane = 1;
                    }
                    else
                    {
                        simdata->cars[i].inpitlane = 0;
                    }
                    simdata->cars[i].inpit = *(uint32_t*) (char*) (a + offsetof(struct pcars2APIStruct, mPitModes) + ((sizeof(float) * i)));
                    if(simdata->cars[i].inpit == 2 || simdata->cars[i].inpit > 3)
                    {
                        simdata->cars[i].inpit = 1;
                    }
                    else
                    {
                        simdata->cars[i].inpit = 0;
                    }
                }

                // realtime telemetry
                simdata->worldposx = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + (sizeof(bool)) + (sizeof(char[STRING_LENGTH_MAX])) + (sizeof(float) * 0));
                simdata->worldposy = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + (sizeof(bool)) + (sizeof(char[STRING_LENGTH_MAX])) + (sizeof(float) * 1));
                simdata->worldposz = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + (sizeof(bool)) + (sizeof(char[STRING_LENGTH_MAX])) + (sizeof(float) * 2));

                float track_spline = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mTrackLength));
                if (track_spline > 0.0f) {
                    float current_lap_distance = *(float*) (char*) (a + offsetof(struct pcars2APIStruct, mParticipantInfo) + offsetof(ParticipantInfo, mCurrentLapDistance));
                    float current_lap_spline = current_lap_distance/track_spline;
                    float player_spline= current_lap_spline+simdata->lap;
                    if (current_lap_distance > 0.0f &&
                        current_lap_spline > 0.0f &&
                        player_spline >0.0f) {
                        simdata->trackdistancearound = current_lap_distance + (track_spline * simdata->lap);
                        simdata->trackspline= track_spline;
                        simdata->playerspline= player_spline;

                        int track_samples = track_spline * 4;
                        simdata->tracksamples = track_samples;
                        simdata->playertrackpos = (int) track_samples * player_spline;
                    }
                }

                break;
            }
            else
            {

                //eCarPhysics = 0,
                //eRaceDefinition = 1,
                //eParticipants = 2,
                //eTimings = 3,
                //eGameState = 4,
                //eWeatherState = 5, // not sent at the moment, information can be found in the game state packet
                //eVehicleNames = 6, //not sent at the moment
                //eTimeStats = 7,
                //eParticipantVehicleNames = 8
                a = base;
                simdata->simstatus = 2;
                if(base != NULL)
                {

                    int packet_type = *(uint8_t*) (char*) (a + offsetof(struct ams2UDPData, mPacketType));
                    char* msg;
                    asprintf(&msg, "project cars 2 packet type %i", packet_type);
                    simapi_log(SIMAPI_LOGLEVEL_TRACE, msg);
                    free(msg);
                    switch ( packet_type )
                    {
                        case 0:
                            simdata->car[0] ='d';
                            simdata->car[1]='e';
                            simdata->car[2]='f';
                            simdata->car[3]='a';
                            simdata->car[4]='u';
                            simdata->car[5]='l';
                            simdata->car[6]='t';
                            simdata->car[7]='\0';

                            simdata->fuel = *(float*) (char*) (a + offsetof(struct ams2UDPData, sFuelLevel));
                            simdata->velocity = droundint(3.6 * (*(float*) (char*) (a + offsetof(struct ams2UDPData, sSpeed))));
                            simdata->rpms = *(uint16_t*) (char*) (a + offsetof(struct ams2UDPData, sRpm));
                            simdata->maxrpm = *(uint16_t*) (char*) (a + offsetof(struct ams2UDPData, sMaxRpm));
                            simdata->gear = *(uint8_t*) (char*) (a + offsetof(struct ams2UDPData, sGearNumGears));
                            // couldn't find this documented anywhere outside of the crewchief source code
                            simdata->gear = simdata->gear & 15;
                            simdata->brake = *(uint8_t*) (char*) (a + offsetof(struct ams2UDPData, sBrake));
                            simdata->gas = *(uint8_t*) (char*) (a + offsetof(struct ams2UDPData, sThrottle));
                            simdata->altitude = 1;

                            simdata->gearc[0] = simdata->gear + 48;
                            if (simdata->gear < 0)
                            {
                                simdata->gearc[0] = 82;
                            }
                            if (simdata->gear == 0)
                            {
                                simdata->gearc[0] = 78;
                            }
                            simdata->gearc[1] = 0;
                    }

                }
                break;
            }

        case SIMULATORAPI_SCSTRUCKSIM2 :

            a = simmap->d.scs2.telemetry_map_addr;

            simdata->simstatus = 2;
            simdata->car[0] ='d';
            simdata->car[1]='e';
            simdata->car[2]='f';
            simdata->car[3]='a';
            simdata->car[4]='u';
            simdata->car[5]='l';
            simdata->car[6]='t';
            simdata->car[7]='\0';


            simdata->velocity = droundint(3.6 * (*(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.speed))));
            simdata->rpms = droundint(*(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.engineRpm)));
            simdata->brake = droundint(*(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.userBrake)));
            simdata->gas = droundint(*(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.userThrottle)));
            simdata->gear = *(uint32_t*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_i.gear));
            simdata->tyreRPS[0] = *(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.truck_wheelVelocity));
            simdata->tyreRPS[1] = *(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.truck_wheelVelocity) + (sizeof(float) * 1));
            simdata->tyreRPS[2] = *(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.truck_wheelVelocity) + (sizeof(float) * 2));
            simdata->tyreRPS[3] = *(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, truck_f.truck_wheelVelocity) + (sizeof(float) * 3));
            simdata->maxrpm = droundint( *(float*) (char*) (a + offsetof(struct scs2TelemetryMap_s, config_f.engineRpmMax)));

            if(simdata->gear>0)
            {
                simdata->gearc[0] = (simdata->gear/2) + 48;
                if(simdata->gear % 2 == 0)
                {
                    simdata->gearc[1] = 72;
                }
                else
                {
                    simdata->gearc[1] = 76;
                    simdata->gearc[0]++;
                }
                simdata->gearc[2] = 0;
            }
            else
            {
                if (simdata->gear < 0)
                {
                    simdata->gearc[0] = 82;
                }
                if (simdata->gear == 0)
                {
                    simdata->gearc[0] = 78;
                }
                simdata->gearc[1] = 0;
            }

            simdata->altitude = 1;
            break;
    }

    if (simmap2 != NULL && simmap2->addr != NULL)
    {
        simdmap(simmap2, simdata);
    }

}

int simdmap(SimMap* simmap, SimData* simdata)
{
    memcpy(simmap->addr, simdata, sizeof(SimData));
}

int siminitudp(SimData* simdata, SimMap* simmap, SimulatorAPI simulator)
{
    int error = SIMAPI_ERROR_NONE;

    void* a;
    switch ( simulator )
    {
        case SIMULATORAPI_PROJECTCARS2 :

            simmap->d.pcars2.has_telemetry=false;

            simmap->d.pcars2.telemetry_map_addr = malloc( AMS2_MAX_UDP_PACKET_SIZE );
            simmap->d.pcars2.has_telemetry=true;
            break;
    }

    return error;
}


int siminit(SimData* simdata, SimMap* simmap, SimulatorAPI simulator)
{
    //slogi("searching for simulator data...");
    int error = SIMAPI_ERROR_NONE;

    void* a;
    switch ( simulator )
    {
        case SIMULATORAPI_SIMAPI_TEST :
            simmap->fd = shm_open(SIMAPI_MEM_FILE, O_RDONLY, S_IRUSR|S_IWUSR);
            if (simmap->fd == -1)
            {
                return 10;
            }

            simmap->addr = mmap(NULL, sizeof(SimData), PROT_READ, MAP_SHARED, simmap->fd, 0);
            if (simmap->addr == MAP_FAILED)
            {
                return 30;
            }
            //slogi("found data for monocoque test...");
            break;

        case SIMULATORAPI_ASSETTO_CORSA :

            simmap->d.ac.has_physics=false;
            simmap->d.ac.has_static=false;
            simmap->d.ac.fd_physics = shm_open(AC_PHYSICS_FILE, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
            if (simmap->d.ac.fd_physics == -1)
            {
                //slogd("could not open Assetto Corsa physics engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->d.ac.physics_map_addr = mmap(NULL, sizeof(simmap->d.ac.ac_physics), PROT_READ, MAP_SHARED, simmap->d.ac.fd_physics, 0);
            if (simmap->d.ac.physics_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa physics data");
                return 30;
            }
            simmap->d.ac.has_physics=true;

            simmap->d.ac.fd_static = shm_open(AC_STATIC_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->d.ac.fd_static == -1)
            {
                //slogd("could not open Assetto Corsa static data");
                return 10;
            }
            simmap->d.ac.static_map_addr = mmap(NULL, sizeof(simmap->d.ac.ac_static), PROT_READ, MAP_SHARED, simmap->d.ac.fd_static, 0);
            if (simmap->d.ac.static_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->d.ac.has_static=true;

            simmap->d.ac.fd_graphic = shm_open(AC_GRAPHIC_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->d.ac.fd_graphic == -1)
            {
                //slogd("could not open Assetto Corsa graphic data");
                return 10;
            }
            simmap->d.ac.graphic_map_addr = mmap(NULL, sizeof(simmap->d.ac.ac_graphic), PROT_READ, MAP_SHARED, simmap->d.ac.fd_graphic, 0);
            if (simmap->d.ac.graphic_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->d.ac.has_graphic=true;
            //slogi("found data for Assetto Corsa...");
            simmap->d.ac.fd_crewchief = shm_open(AC_CREWCHIEF_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->d.ac.fd_crewchief == -1)
            {
                //slogd("could not open Assetto Corsa graphic data");
                return 10;
            }
            simmap->d.ac.crewchief_map_addr = mmap(NULL, sizeof(simmap->d.ac.ac_crewchief), PROT_READ, MAP_SHARED, simmap->d.ac.fd_crewchief, 0);
            if (simmap->d.ac.crewchief_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa static data");
                return 30;
            }
            simmap->d.ac.has_crewchief=true;

            break;

        case SIMULATORAPI_PROJECTCARS2 :

            simmap->d.pcars2.has_telemetry=false;
            simmap->d.pcars2.fd_telemetry = shm_open(PCARS2_FILE, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
            if (simmap->d.pcars2.fd_telemetry == -1)
            {
                //slogd("could not open Assetto Corsa physics engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->d.pcars2.telemetry_map_addr = mmap(NULL, sizeof(simmap->d.pcars2.pcars2_telemetry), PROT_READ, MAP_SHARED, simmap->d.pcars2.fd_telemetry, 0);
            if (simmap->d.pcars2.telemetry_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve Assetto Corsa physics data");
                return 30;
            }
            simmap->d.pcars2.has_telemetry=true;

            break;

        case SIMULATORAPI_RFACTOR2 :

            simmap->d.rf2.has_telemetry=false;
            simmap->d.rf2.has_scoring=false;
            simmap->d.rf2.fd_telemetry = shm_open(RF2_TELEMETRY_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->d.rf2.fd_telemetry == -1)
            {
                //slogd("could not open RFactor2 Telemetry engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->d.rf2.telemetry_map_addr = mmap(NULL, sizeof(simmap->d.rf2.rf2_telemetry), PROT_READ, MAP_SHARED, simmap->d.rf2.fd_telemetry, 0);
            if (simmap->d.rf2.telemetry_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve RFactor2 telemetry data");
                return 30;
            }
            simmap->d.rf2.has_telemetry=true;
            simmap->d.rf2.fd_scoring = shm_open(RF2_SCORING_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->d.rf2.fd_scoring == -1)
            {
                //slogd("could not open RFactor2 Telemetry engine");
                return SIMAPI_ERROR_NODATA;
            }
            simmap->d.rf2.scoring_map_addr = mmap(NULL, sizeof(simmap->d.rf2.rf2_scoring), PROT_READ, MAP_SHARED, simmap->d.rf2.fd_scoring, 0);
            if (simmap->d.rf2.scoring_map_addr == MAP_FAILED)
            {
                //slogd("could not retrieve RFactor2 telemetry data");
                return 30;
            }
            simmap->d.rf2.has_scoring=true;


            //slogi("found data for RFactor2...");
            break;

        case SIMULATORAPI_SCSTRUCKSIM2 :

            simmap->d.scs2.has_telemetry=false;
            simmap->d.scs2.fd_telemetry = open("/dev/shm/SCS/SCSTelemetry", O_CREAT|O_RDWR, 0777);
            //simmap->d.scs2.fd_telemetry = shm_open(SCS2_TELEMETRY_FILE, O_RDONLY, S_IRUSR | S_IWUSR);
            if (simmap->d.scs2.fd_telemetry == -1)
            {
                return SIMAPI_ERROR_NODATA;
            }
            simmap->d.scs2.telemetry_map_addr = mmap(NULL, sizeof(simmap->d.scs2.scs2_telemetry), PROT_READ, MAP_SHARED, simmap->d.scs2.fd_telemetry, 0);
            if (simmap->d.scs2.telemetry_map_addr == MAP_FAILED)
            {
                return 30;
            }
            simmap->d.scs2.has_telemetry=true;

            break;
    }

    return error;
}


int simfree(SimData* simdata, SimMap* simmap, SimulatorAPI simulator)
{
    int error = SIMAPI_ERROR_NONE;
    if(simulator == -1)
    {
        return error;
    }

    switch ( simulator )
    {
        case SIMULATORAPI_SIMAPI_TEST :
            if (munmap(simmap->addr, sizeof(SimData)) == -1)
            {
                return 100;
            }

            if (close(simmap->fd) == -1)
            {
                return 200;
            }
            break;

        case SIMULATORAPI_ASSETTO_CORSA :
            if(simmap->d.ac.has_physics==true)
            {
                if (munmap(simmap->d.ac.physics_map_addr, sizeof(simmap->d.ac.ac_physics)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.ac.fd_physics) == -1)
                {
                    return 200;
                }

                simmap->d.ac.has_physics = false;
            }
            if(simmap->d.ac.has_static==true)
            {
                if (munmap(simmap->d.ac.static_map_addr, sizeof(simmap->d.ac.ac_static)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.ac.fd_static) == -1)
                {
                    return 200;
                }

                simmap->d.ac.has_static = false;
            }

            if(simmap->d.ac.has_graphic==true)
            {
                if (munmap(simmap->d.ac.graphic_map_addr, sizeof(simmap->d.ac.ac_graphic)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.ac.fd_graphic) == -1)
                {
                    return 200;
                }

                simmap->d.ac.has_graphic = false;
            }

            if(simmap->d.ac.has_crewchief==true)
            {
                if (munmap(simmap->d.ac.crewchief_map_addr, sizeof(simmap->d.ac.ac_crewchief)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.ac.fd_crewchief) == -1)
                {
                    return 200;
                }

                simmap->d.ac.has_crewchief = false;
            }
            break;

        case SIMULATORAPI_PROJECTCARS2 :
            if(simmap->d.pcars2.has_telemetry==true)
            {
                if (munmap(simmap->d.pcars2.telemetry_map_addr, sizeof(simmap->d.pcars2.pcars2_telemetry)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.pcars2.fd_telemetry) == -1)
                {
                    return 200;
                }

                simmap->d.pcars2.has_telemetry = false;
            }
            break;

        case SIMULATORAPI_RFACTOR2 :
            if(simmap->d.rf2.has_telemetry==true)
            {
                if (munmap(simmap->d.rf2.telemetry_map_addr, sizeof(simmap->d.rf2.rf2_telemetry)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.rf2.fd_telemetry) == -1)
                {
                    return 200;
                }

                simmap->d.rf2.has_telemetry = false;
            }
            if(simmap->d.rf2.has_scoring==true)
            {
                if (munmap(simmap->d.rf2.scoring_map_addr, sizeof(simmap->d.rf2.rf2_scoring)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.rf2.fd_scoring) == -1)
                {
                    return 200;
                }

                simmap->d.rf2.has_scoring = false;
            }
            break;
        case SIMULATORAPI_SCSTRUCKSIM2 :
            if(simmap->d.scs2.has_telemetry==true)
            {
                if (munmap(simmap->d.scs2.telemetry_map_addr, sizeof(simmap->d.scs2.scs2_telemetry)) == -1)
                {
                    return 100;
                }

                if (close(simmap->d.scs2.fd_telemetry) == -1)
                {
                    return 200;
                }

                simmap->d.scs2.has_telemetry = false;
            }
            break;
    }

    bzero(simdata, sizeof(SimData));
    return error;
}

int freesimmap(SimMap* simmap, bool issimd)
{
    simapi_log(SIMAPI_LOGLEVEL_INFO, "Freeing universal shared memory");

    if(simmap->fd == -1)
    {
        free(simmap);
        return 0;
    }

    if (munmap(simmap->addr, sizeof(SimData)) == -1)
    {
        return 100;
    }
    if(issimd == true)
    {
        shm_unlink(SIMAPI_MEM_FILE);
    }

    if (close(simmap->fd) == -1)
    {
        return 200;
    }

    free(simmap);
    return 0;
}

int opensimmap(SimMap* simmap)
{
    simmap->fd = shm_open(SIMAPI_MEM_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (simmap->fd == -1)
    {
        printf("open");
        return 10;
    }
    int res = ftruncate(simmap->fd, sizeof(SimData));
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    void* addr = mmap(NULL, sizeof(SimData), PROT_WRITE, MAP_SHARED, simmap->fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    simmap->addr = addr;
    return 0;
}


int opensimcompatmap(SimCompatMap* compatmap)
{
    compatmap->pcars2_fd = shm_open(PCARS2_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->pcars2_fd == -1)
    {
        printf("open");
        return 10;
    }
    int res = ftruncate(compatmap->pcars2_fd, sizeof(struct pcars2APIStruct));
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    void* addr = mmap(NULL, sizeof(struct pcars2APIStruct), PROT_WRITE, MAP_SHARED, compatmap->pcars2_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->pcars2_addr = addr;

    compatmap->acphysics_fd = shm_open(AC_PHYSICS_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acphysics_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = ftruncate(compatmap->acphysics_fd, sizeof(struct SPageFilePhysics));
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = mmap(NULL, sizeof(struct SPageFilePhysics), PROT_WRITE, MAP_SHARED, compatmap->acphysics_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->acphysics_addr = addr;

    compatmap->acgraphics_fd = shm_open(AC_GRAPHIC_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acgraphics_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = ftruncate(compatmap->acgraphics_fd, sizeof(struct SPageFileGraphic));
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = mmap(NULL, sizeof(struct SPageFileGraphic), PROT_WRITE, MAP_SHARED, compatmap->acgraphics_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->acgraphics_addr = addr;

    compatmap->acstatic_fd = shm_open(AC_STATIC_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->acstatic_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = ftruncate(compatmap->acstatic_fd, sizeof(struct SPageFileStatic));
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = mmap(NULL, sizeof(struct SPageFileStatic), PROT_WRITE, MAP_SHARED, compatmap->acstatic_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->acstatic_addr = addr;

    compatmap->accrew_fd = shm_open(AC_CREWCHIEF_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (compatmap->accrew_fd == -1)
    {
        printf("open");
        return 10;
    }
    res = ftruncate(compatmap->accrew_fd, sizeof(struct SPageFileCrewChief));
    if (res == -1)
    {
        printf("ftruncate");
        return 20;
    }

    addr = mmap(NULL, sizeof(struct SPageFileCrewChief), PROT_WRITE, MAP_SHARED, compatmap->accrew_fd, 0);
    if (addr == MAP_FAILED)
    {
        printf("mmap");
        return 30;
    }
    compatmap->accrew_addr = addr;
    return 0;
}

int freesimcompatmap(SimCompatMap* compatmap)
{
    if (munmap(compatmap->acphysics_addr, sizeof(struct SPageFilePhysics)) == -1)
    {
        return 100;
    }
    shm_unlink(AC_PHYSICS_FILE);

    if (close(compatmap->acphysics_fd) == -1)
    {
        return 200;
    }

    if (munmap(compatmap->acstatic_addr, sizeof(struct SPageFileStatic)) == -1)
    {
        return 100;
    }
    shm_unlink(AC_STATIC_FILE);

    if (close(compatmap->acstatic_fd) == -1)
    {
        return 200;
    }

    if (munmap(compatmap->acgraphics_addr, sizeof(struct SPageFileGraphic)) == -1)
    {
        return 100;
    }
    shm_unlink(AC_GRAPHIC_FILE);

    if (close(compatmap->acgraphics_fd) == -1)
    {
        return 200;
    }

    if (munmap(compatmap->accrew_addr, sizeof(struct SPageFileCrewChief)) == -1)
    {
        return 100;
    }
    shm_unlink(AC_CREWCHIEF_FILE);

    if (close(compatmap->accrew_fd) == -1)
    {
        return 200;
    }

    if (munmap(compatmap->pcars2_addr, sizeof(struct pcars2APIStruct)) == -1)
    {
        return 100;
    }
    shm_unlink(PCARS2_FILE_LINUX);

    if (close(compatmap->pcars2_fd) == -1)
    {
        return 200;
    }

    return 0;
}
