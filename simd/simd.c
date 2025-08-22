#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <poll.h>
#include <termios.h>
#include <signal.h>
#include <uv.h>
#include <yder.h>
#include <sys/types.h>
#include <pwd.h>
#include <libconfig.h>

#include <simapi.h>
#include <simdata.h>
#include <simmapper.h>

#include "../simapi/getpid.h"
#include "loopdata.h"
#include "parameters.h"
#include "dirhelper.h"
#include "confighelper.h"

bool compatmemmap;

Parameters* p;
LoopData* baton;
SimData* simdata;
SimMap* simmap;
SimMap* simmap2;
SimCompatMap* compatmap;
SimdSettings simds;

uv_poll_t pollt;
uv_timer_t gamefindtimer;
uv_timer_t datachecktimer;
uv_timer_t datamaptimer;
uv_udp_t recv_socket;


bool doui = false;
int appstate = 0;

void shmdatamapcallback(uv_timer_t* handle);
void datacheckcallback(uv_timer_t* handle);
void gamefindcallback(uv_timer_t* handle);

void simapilib_loginfo(char* message)
{
    y_log_message(Y_LOG_LEVEL_INFO, message);
}

void simapilib_logdebug(char* message)
{
    y_log_message(Y_LOG_LEVEL_DEBUG, message);
}

void simapilib_logtrace(char* message)
{
    y_log_message(Y_LOG_LEVEL_DEBUG, message);
}

int set_settings(Parameters* p, SimdSettings* simds)
{
    // future config file read goes here, which can be overriden by cli options

    simds->force_udp = false;
    if(p->udp_count > 0)
    {
        simds->force_udp = p->udp;
    }

    simds->daemon = true;
    if(p->daemon_count > 0)
    {
        simds->daemon = p->daemon;
    }

    simds->auto_memmap = true;
    if(p->memmap_count > 0)
    {
        simds->auto_memmap = p->memmap;
    }

    simds->auto_bridge = true;
    if(p->bridge_count > 0)
    {
        simds->auto_bridge = p->bridge;
    }
}

static void close_walk_cb(uv_handle_t* handle, void* arg) {
  if (!uv_is_closing(handle))
    uv_close(handle, NULL);
}
#define ASSERT(expr) expr
void release()
{
    uv_timer_stop(&gamefindtimer);
    uv_timer_stop(&datamaptimer);
    uv_timer_stop(&datachecktimer);
    uv_walk(uv_default_loop(), close_walk_cb, NULL);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    ASSERT(0 == uv_loop_close(uv_default_loop()));
    uv_library_shutdown();

    if(compatmemmap == true)
    {
        freesimcompatmap(compatmap);
        free(compatmap);
    }

    freesimmap(simmap, true);
    freesimmap(simmap2, true);

    free(baton);
    free(simdata);

    free(p);

    free(simds.home_dir);
    free(simds.configfile);

    y_close_logs();
}

// Signal handler for SIGTERM
void handle_sigterm(int sig) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "SIGTERM received. Exiting gracefully...");

    release();
    exit(0);
}

void shmdatamapcallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    LoopData* f = (LoopData*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;
    SimMap* simmap2 = f->simmap2;
    //appstate = 2;
    if (appstate == 2)
    {
        simdatamap(simdata, simmap, simmap2, f->sim, false, NULL);
        doui = false;
    }

    if (f->simstate == false || simdata->simstatus <= 1 || appstate <= 1)
    {
        if(f->releasing == false)
        {
            f->releasing = true;
            uv_timer_stop(handle);
            y_log_message(Y_LOG_LEVEL_INFO, "releasing devices, please wait");
            f->uion = false;

            // help things spin down
            simdata->rpms = 0;
            simdata->velocity = 0;
            int r = simfree(simdata, simmap, f->sim);
            y_log_message(Y_LOG_LEVEL_DEBUG, "simfree returned %i", r);
            y_log_message(Y_LOG_LEVEL_INFO, "stopped mapping data, press q again to quit");
            //stopui(ms->ui_type, f);
            // free loop data

            if(appstate > 0)
            {
                // not sure what to do here yet, but this needs to be different if you have auto bridge enabled
                // or the data check algorithm needs to check and see if any process is even running
                // if there isn't kill the bridge if you can and return to the gamefind if autobridge is enabled
                uv_timer_start(&datachecktimer, datacheckcallback, 3000, 1000);
            }
            f->releasing = false;
            if(appstate > 1)
            {
                appstate = 1;
            }
        }
    }
}

void on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
    bzero(buf->base, suggested_size);
}

static void on_udp_recv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* rcvbuf, const struct sockaddr* addr, unsigned flags) {

    char* a;
    a = rcvbuf->base;

    void* b = uv_handle_get_data((uv_handle_t*) handle);
    LoopData* f = (LoopData*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;

    if (appstate == 2)
    {
        simdatamap(simdata, simmap, NULL, f->sim, true, a);
    }

    if (f->simstate == false || simdata->simstatus <= 1 || appstate <= 1)
    {
        if(f->releasing == false)
        {
            f->releasing = true;
            uv_udp_recv_stop(handle);
            y_log_message(Y_LOG_LEVEL_CURRENT, "releasing devices, please wait");
            f->uion = false;

            int r = simfree(simdata, simmap, f->sim);
            y_log_message(Y_LOG_LEVEL_DEBUG, "simfree returned %i", r);
            y_log_message(Y_LOG_LEVEL_INFO, "stopped mapping data, press q again to quit");

            if(appstate > 0)
            {
                uv_timer_start(&datachecktimer, datacheckcallback, 3000, 1000);
            }
            f->releasing = false;
            if(appstate > 1)
            {
                appstate = 1;
            }
        }
    }

    free(rcvbuf->base);
}

int startudp(int port)
{
    uv_udp_init(uv_default_loop(), &recv_socket);
    struct sockaddr_in recv_addr;
    uv_ip4_addr("0.0.0.0", port, &recv_addr);
    int err = uv_udp_bind(&recv_socket, (const struct sockaddr *) &recv_addr, UV_UDP_REUSEADDR);

    return err;
}

void gamefindcallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    LoopData* f = (LoopData*) b;
    SimdSettings simds = f->simds;
    GameCompatInfo* game_compat_info = f->game_compat_info;

    int i = 0;
    int gamepid = -1;
    int sim = 0;
    for(int i = 0; i < f->compat_info_size; i++)
    {
        if(gamepid > 0)
        {
            continue;
        }
        char* tmp_launch_exe = game_compat_info[i].LaunchExe;
        sim = game_compat_info[i].GameId;

        gamepid = IsProcessRunning(tmp_launch_exe);
        if(gamepid > 0)
        {
            fprintf(stderr, "found a specified launch process running!!!\n");
            continue;
        }
    }
    if(gamepid <= 0 && sim <= 0)
    {
        i = -1;
        sim = getSimExe();
    }


    int err = 0;
    if(sim > 0)
    {
        y_log_message(Y_LOG_LEVEL_INFO, "Detected simulator id %i, starting appropriate bridge if necessary.", sim);

        if(does_sim_need_bridge(sim) == true && err == 0 && i > -1)
        {

            pid_t pid = gamepid;
            const char* env_var1 = "STEAM_COMPAT_TOOL_PATHS"; // this will need to be interpreted
            const char* env_var2 = "STEAM_COMPAT_DATA_PATH"; // this will be exactly what we need except append /pfx
            const char* env_var3 = "SIMD_BRIDGE_EXE_PATH"; // this will be exactly what we need
            char* env1 = getEnvValueForPid(pid, env_var1);
            char* env2 = getEnvValueForPid(pid, env_var2);
            char* env3 = getEnvValueForPid(pid, env_var3);
            y_log_message(Y_LOG_LEVEL_DEBUG, "Retrieved env vars %s and %s and %s", env1, env2, env3);

            char* token = strtok(env1, ":");

            char* wineprefix = NULL;
            if(env2 != NULL)
            {
                asprintf(&wineprefix, "WINE_PREFIX=%s/pfx", env2);
            }
            free(env2);

            char* wineexe = NULL;
            if(token != NULL)
            {
                char* pathcheck1 = NULL;
                asprintf(&pathcheck1, "%s/dist/bin/wine", token);
                if(does_file_exist(pathcheck1) == true)
                {
                    wineexe = strdup(pathcheck1);
                }
                if(pathcheck1 != NULL)
                {
                    free(pathcheck1);
                }
                pathcheck1 = NULL;
                if(wineexe == NULL)
                {
                    asprintf(&pathcheck1, "%s/files/bin/wine", token);

                    if(does_file_exist(pathcheck1) == true)
                    {

                        wineexe = strdup(pathcheck1);
                    }
                }
                if(pathcheck1 != NULL)
                {
                    free(pathcheck1);
                }
                pathcheck1 = NULL;
            }

            if(wineexe != NULL)
            {
                y_log_message(Y_LOG_LEVEL_DEBUG, "Determined wine executable path %s", wineexe);
            }

            static char* newargv[]= {"/usr/bin/wine", "/home/user/git/simshmbridge/assets/acbridge.exe", NULL};
            static char* newenviron[]= {"WINE_PREFIX=/home/user/.local/share/Steam/steamapps/compatdata/244210", "WINEFSYNC=1", NULL};
            newargv[1] = env3;
            newenviron[0] = wineprefix;

            uint8_t ret = 0;
            pid_t process;
            process = fork();
            if (process == 0)
            {
                if (setsid() == -1)
                {
                }

                int devnull = open("/dev/null", O_RDONLY);
                if (devnull == -1)
                {
                }

                dup2(devnull, STDIN_FILENO);
                dup2(devnull, STDOUT_FILENO);
                dup2(devnull, STDERR_FILENO);
                if (devnull > 2)
                {
                    close(devnull);
                }

                close(devnull);
                ret = execve(wineexe, newargv, newenviron);
            }
            free(env1);
            free(wineprefix);
            free(env3);

            if(process > 0)
            {
                y_log_message(Y_LOG_LEVEL_DEBUG, "Fork was successful looking for data next");
                //double check that process is running
                uv_timer_start(&datachecktimer, datacheckcallback, 5, 1000);
                uv_timer_stop(handle);
            }
            if(process == -1)
            {
                y_log_message(Y_LOG_LEVEL_DEBUG, "Could not fork a bridge process");
            }
        }
        else
        {
            y_log_message(Y_LOG_LEVEL_DEBUG, "sim %i does not require a compatibility exe, will continue to mapping data", sim);
            uv_timer_start(&datachecktimer, datacheckcallback, 0, 1000);
            uv_timer_stop(handle);
        }

    }
}

void datacheckcallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    LoopData* f = (LoopData*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;
    SimMap* simmap2 = f->simmap2;

    if ( appstate == 1 )
    {
        SimInfo si = getSim(simdata, simmap, false, startudp, true);
        //TODO: move all this to a siminfo struct in loop_data
        f->simstate = si.isSimOn;
        f->sim = si.simulatorapi;
        f->use_udp = si.SimUsesUDP;

    }
    if (f->simstate == true && simdata->simstatus >= 2)
    {
        fprintf(stderr, "the timer will stop");
        if ( appstate == 1 )
        {
            appstate++;
            doui = true;
            //simdata->tyrediameter[0] = -1;
            //simdata->tyrediameter[1] = -1;
            //simdata->tyrediameter[2] = -1;
            //simdata->tyrediameter[3] = -1;

            if(f->use_udp == true)
            {
                //udpstart(f, simdata, simmap);
                uv_udp_recv_start(&recv_socket, on_alloc, on_udp_recv);
            }
            else
            {
                uv_timer_start(&datamaptimer, shmdatamapcallback, 2000, 16);
            }
            uv_timer_stop(handle);
        }
    }

    if (appstate == 0)
    {
        y_log_message(Y_LOG_LEVEL_INFO, "stopping checking for data");
        uv_timer_stop(handle);
    }
}

void cb(uv_poll_t* handle, int status, int events)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    LoopData* f = (LoopData*) b;
    char ch;
    scanf("%c", &ch);
    if (ch == 'q')
    {
        if(f->releasing == false && doui == false)
        {
            appstate--;
            y_log_message(Y_LOG_LEVEL_INFO, "User requested stop appstate is now %i", appstate);
            fprintf(stdout, "User requested stop appstate is now %i\n", appstate);
            fflush(stdout);
        }
    }

    if (appstate == 0)
    {
        y_log_message(Y_LOG_LEVEL_INFO, "simd is exiting...");
        uv_timer_stop(&datachecktimer);
        uv_timer_stop(&gamefindtimer);
        uv_poll_stop(handle);
    }
}


int main(int argc, char** argv)
{
    char* home_dir_str = getpwuid(getuid())->pw_dir;
    if(home_dir_str == NULL)
    {
        fprintf(stderr, "You need a home directory");
        return 0;
    }
    simds.home_dir = strdup(home_dir_str);

    y_init_logs("simd", Y_LOG_MODE_FILE, Y_LOG_LEVEL_DEBUG, "/tmp/simd.log", "Initializing logs mode: file, logs level: debug");

    // cli parameters
    p = malloc(sizeof(Parameters));
    ConfigError ppe = getParameters(argc, argv, p);
    set_settings(p, &simds);
    if (ppe == E_SUCCESS_AND_EXIT)
    {
        goto cleanup_final;
    }

    fprintf(stderr, "Started. Found home directory and interpreted parameters.\n");

    // config file
    bool good_config = false;
    char* configfile;
    asprintf(&simds.configfile, "%s%s", simds.home_dir, "/.config/simd/simd.config");
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, simds.configfile))
    {
        char* configerror;
        asprintf(&configerror, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        y_log_message(Y_LOG_LEVEL_WARNING, "Error with config file: %s", configerror);
        y_log_message(Y_LOG_LEVEL_WARNING, "Disabling Automatic Bridge Mode");
        free(configerror);
    }
    else
    {
        y_log_message(Y_LOG_LEVEL_INFO, "Opened and validated simd configuration file");
        fprintf(stderr, "Opened and validated simd configuration file.\n");
        good_config = true;
    }
    config_destroy(&cfg);

    GameCompatInfo* game_compat_info;
    int compat_info_size = 0;
    if(good_config == true)
    {
        compat_info_size = getNumberOfConfigs(simds.configfile);
        game_compat_info = malloc(compat_info_size * sizeof(GameCompatInfo));

        loadconfig(simds, compat_info_size, game_compat_info);

        y_log_message(Y_LOG_LEVEL_INFO, "Successfullly loaded configuration file");
        fprintf(stderr, "Successfully loaded configuration file.\n");
    }

    struct termios newsettings, canonicalmode;
    if(simds.daemon == true)
    {
        pid_t pid;

        /* Fork off the parent process */
        pid = fork();

        /* An error occurred */
        if (pid < 0)
            exit(EXIT_FAILURE);

        /* Success: Let the parent terminate */
        if (pid > 0)
            exit(EXIT_SUCCESS);

        /* On success: The child process becomes session leader */
        if (setsid() < 0)
            exit(EXIT_FAILURE);

        /* Catch, ignore and handle signals */
        //TODO: Implement a working signal handler */
        signal(SIGCHLD, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        signal(SIGTERM, handle_sigterm);
        /* Fork off for the second time*/
        pid = fork();

        /* An error occurred */
        if (pid < 0)
            exit(EXIT_FAILURE);

        /* Success: Let the parent terminate */
        if (pid > 0)
            exit(EXIT_SUCCESS);

        /* Set new file permissions */
        umask(0);

        /* Change the working directory to the root directory */
        /* or another appropriated directory */
        chdir("/");

        /* Close all open file descriptors */
        int x;
        for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
        {
            close (x);
        }
    }
    else
    {

        tcgetattr(0, &canonicalmode);
        newsettings = canonicalmode;
        newsettings.c_lflag &= (~ICANON & ~ECHO);
        newsettings.c_cc[VMIN] = 1;
        newsettings.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &newsettings);
        char ch;
        struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
    }



    set_simapi_log_info(simapilib_loginfo);
    if(p->verbosity_count>0)
    {
        set_simapi_log_debug(simapilib_logdebug);
        set_simapi_log_trace(simapilib_logtrace);
    }

    simdata = malloc(sizeof(SimData));
    bzero(simdata, sizeof(SimData));

    simmap = createSimMap();
    simmap2 = createSimMap();

    compatmemmap = false;
    if(simds.auto_memmap == true)
    {
        compatmemmap = true;
        compatmap = malloc(sizeof(SimCompatMap));
        opensimcompatmap(compatmap);
    }

    opensimmap(simmap2);

    simdata->simapiversion = SIMAPI_VERSION;
    simdmap(simmap2, simdata);
    fprintf(stdout, "SimApi Version: %i\n", simdata->simapiversion);

    baton = (LoopData*) malloc(sizeof(LoopData));
    baton->simmap = simmap;
    baton->simmap2 = simmap2;
    baton->simdata = simdata;
    baton->simstate = false;
    baton->uion = false;
    baton->releasing = false;
    baton->sim = 0;
    baton->simds = simds;
    baton->compat_info_size = compat_info_size;
    baton->game_compat_info = game_compat_info;
    baton->req.data = (void*) baton;


    uv_handle_set_data((uv_handle_t*) &gamefindtimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &datachecktimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &datamaptimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &recv_socket, (void*) baton);

    appstate = 1;
    y_log_message(Y_LOG_LEVEL_DEBUG, "setting initial app state");
    uv_timer_init(uv_default_loop(), &gamefindtimer);
    uv_timer_init(uv_default_loop(), &datachecktimer);
    uv_timer_init(uv_default_loop(), &datamaptimer);

    if(simds.daemon == false)
    {
        fprintf(stdout, "Searching for sim data... Press q to quit...\n");
    }
    if(simds.auto_bridge == true)
    {
        y_log_message(Y_LOG_LEVEL_INFO, "Starting Bridge Polling Thread.");
        uv_timer_start(&gamefindtimer, gamefindcallback, 1000, 1000);
    }
    else
    {
        y_log_message(Y_LOG_LEVEL_INFO, "Starting Data Mapping Thread.");
        uv_timer_start(&datachecktimer, datacheckcallback, 1000, 1000);
    }

    uv_poll_t* poll;
    if(simds.daemon == false)
    {
        uv_handle_set_data((uv_handle_t*) &pollt, (void*) baton);
        uv_poll_init(uv_default_loop(), &pollt, 0);
        uv_poll_start(&pollt, UV_READABLE, cb);
    }

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    if(simds.daemon==false)
    {
        fprintf(stdout, "\n");
        fflush(stdout);
        tcsetattr(0, TCSANOW, &canonicalmode);
        uv_poll_stop(&pollt);
        release();
    }

cleanup_final:

    y_close_logs();
    return 0;
}
