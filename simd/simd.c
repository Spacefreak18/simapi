#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <poll.h>
#include <termios.h>
#include <signal.h>
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
#include "../simapi/simmap.h"
#include "../simapi/test.h"
#include "../simmap/basicmap.h"

#include "../simapi/getpid.h"
#include "loopdata.h"
#include "parameters.h"
#include "dirhelper.h"
#include "confighelper.h"
#include "poke.h"

#define PID_FILE "/tmp/simd.pid"

bool compatmemmap;

Parameters* p;
LoopData* baton;
SimData* simdata;
SimMap* simmap;
SimMap* simmap2;
SimCompatMap* compatmap;
GameCompatInfo* game_compat_info;
SimdSettings simds;

uv_poll_t pollt;
uv_timer_t gamefindtimer;
uv_timer_t datachecktimer;
uv_timer_t datamaptimer;
uv_timer_t bridgeclosetimer;
uv_udp_t recv_socket;
bool recv_socket_initialized = false;
bool recv_socket_bound = false;

int appstate = 0;
int compat_info_size = 0;
int gamepid = 0;

void shmdatamapcallback(uv_timer_t* handle);
void datacheckcallback(uv_timer_t* handle);
void gamefindcallback(uv_timer_t* handle);
void bridgeclosecallback(uv_timer_t* handle);

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

    simds->notify = true;
    if(p->notify_count > 0)
    {
        simds->notify = p->notify;
    }

    simds->auto_bridge = p->bridge;

    simds->poke = false;
    if(p->poke == true)
    {
        simds->pokesetting = strdup(p->pokesetting);
        simds->poke = true;
    }


    if(p->targetval == false)
    {
        simds->poke = false;
    }
    else
    {
        simds->targetvalue = strdup(p->targetvalue);
    }
    fprintf(stderr, "starting simd\n");
}

static void close_walk_cb(uv_handle_t* handle, void* arg)
{
    if (!uv_is_closing(handle))
    {
        uv_close(handle, NULL);
    }
}
#define ASSERT(expr) expr
void release()
{
    y_log_message(Y_LOG_LEVEL_INFO, "calling release method");
    uv_timer_stop(&gamefindtimer);
    uv_timer_stop(&datamaptimer);
    uv_timer_stop(&datachecktimer);
    if (recv_socket_initialized)
    {
        uv_udp_recv_stop(&recv_socket);
    }
    uv_timer_stop(&bridgeclosetimer);
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
    if(compat_info_size > 0)
    {
        for(int i = 0; i < compat_info_size; i++)
        {
            free(game_compat_info[i].Name);
            free(game_compat_info[i].LaunchExe);
            free(game_compat_info[i].LiveExe);
        }
        free(game_compat_info);
    }
    free(p);

    free(simds.home_dir);
    free(simds.configfile);

    unlink(PID_FILE);

    y_close_logs();
}

// Signal handler for SIGTERM
void handle_sigterm(int sig)
{
    y_log_message(Y_LOG_LEVEL_DEBUG, "SIGTERM received. Exiting gracefully...");

    release();
    exit(0);
}

void releaseloop(LoopData* f, SimData* simdata, SimMap* simmap)
{
    SimdSettings simds = f->simds;

    if(f->releasing == false)
    {

        f->releasing = true;
        appstate = 1;
        uv_timer_stop(&datamaptimer);
        y_log_message(Y_LOG_LEVEL_INFO, "stopping data mapping, please wait");
        f->uion = false;

        // help things spin down
        simdata->simstatus = 0;
        simdata->rpms = 0;
        simdata->velocity = 0;
        if (simmap2 != NULL)
        {
            simdmap(simmap2, simdata);
        }
        // Properly close the UDP socket if it's open
        if (recv_socket_initialized && uv_is_active((uv_handle_t*)&recv_socket))
        {
            uv_udp_recv_stop(&recv_socket);
        }

        int r = simfree(simdata, simmap, f->sim);
        y_log_message(Y_LOG_LEVEL_DEBUG, "simfree returned %i", r);

        if(simds.auto_memmap == true)
        {
            simcompatmapclear(compatmap);
            y_log_message(Y_LOG_LEVEL_DEBUG, "cleared memory mapped files");
        }

        y_log_message(Y_LOG_LEVEL_INFO, "stopped mapping data, press q again to quit");

        f->releasing = false;
        if(appstate > 1)
        {
            appstate = 1;
        }
    }

}

void shmdatamapcallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    LoopData* f = (LoopData*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;
    SimMap* simmap2 = f->simmap2;
    SimdSettings simds = f->simds;
    //appstate = 2;
    if (appstate == 2)
    {
        simdatamap(simdata, simmap, simmap2, f->sim, false, NULL);
    }

    if (f->simstate == false || simdata->simstatus <= 1 || appstate <= 1)
    {
        releaseloop(f, simdata, simmap);
    }
}

void on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf)
{
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
    bzero(buf->base, suggested_size);
}

static void on_udp_recv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* rcvbuf, const struct sockaddr* addr, unsigned flags)
{
    if (nread <= 0)
    {
        free(rcvbuf->base);
        return;
    }

    if (nread > 0)
    {
        y_log_message(Y_LOG_LEVEL_DEBUG, "UDP packet received: %zd bytes", nread);
    }

    char* a;
    a = rcvbuf->base;

    void* b = uv_handle_get_data((uv_handle_t*) handle);
    LoopData* f = (LoopData*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;
    SimMap* simmap2 = f->simmap2;

    if (appstate == 2)
    {
        simdatamap(simdata, simmap, simmap2, f->sim, true, a);
    }
    else
    {
        y_log_message(Y_LOG_LEVEL_DEBUG, "UDP packet received but appstate is %d (expected 2)", appstate);
    }

    /*
     * Relaxed shutdown condition: only stop mapping if explicitly requested
     * (appstate <= 1) or if the simulation state is definitively off. We no
     * longer shut down based on simstatus <= 1 from a single packet, as DR2 can
     * report MENU status (runTime = 0) during countdowns in Rally Cross.
     */
    if (f->simstate == false || appstate <= 1)
    {
        releaseloop(f, simdata, simmap);
    }

    free(rcvbuf->base);
}

int startudp(int port)
{
    // Already bound successfully - nothing to do
    if (recv_socket_bound)
    {
        return 0;
    }
    // Socket active or closing - wait for it to settle
    if (recv_socket_initialized && (uv_is_active((uv_handle_t*)&recv_socket) || uv_is_closing((uv_handle_t*)&recv_socket)))
    {
        return 0;
    }
    if (!recv_socket_initialized)
    {
        uv_udp_init(uv_default_loop(), &recv_socket);
        uv_handle_set_data((uv_handle_t*) &recv_socket, (void*) baton);
        recv_socket_initialized = true;
    }
    struct sockaddr_in recv_addr;
    uv_ip4_addr("0.0.0.0", port, &recv_addr);
    int err = uv_udp_bind(&recv_socket, (const struct sockaddr *) &recv_addr, UV_UDP_REUSEADDR);
    y_log_message(Y_LOG_LEVEL_DEBUG, "udp bind result is %i for port %i", err, port);

    if (err == 0)
    {
        recv_socket_bound = true;
    }

    return err;
}

int is_pid_running(pid_t pid)
{
    if (pid <= 0)
    {
        return 0;
    }

    // send signal 0 (no actual signal)
    if (kill(pid, 0) == 0)
    {
        return 1;
    }
    else
    {
        if (errno == ESRCH)
        {
            return 0;
        }
        else
            if (errno == EPERM)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        return 0;
    }
}

void bridgeclosecallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    LoopData* f = (LoopData*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;

    if(is_pid_running(f->game_pid) == 0)
    {
        y_log_message(Y_LOG_LEVEL_INFO, "No longer detected game pid %i, so closing bridge pid %i", f->game_pid, f->bridge_pid);;

        if(simds.notify == true)
        {
            char cmd[512];
            snprintf(cmd, sizeof(cmd), "notify-send -t 3000 \"%s\" \"game stopped\"", "simd");
            system(cmd);
        }

        if(f->bridge_pid > 0)
        {
            kill(f->bridge_pid, SIGTERM);
            y_log_message(Y_LOG_LEVEL_INFO, "Sent SIGTERM to bridge pid");
        }
        f->bridge_pid = 0;
        f->game_pid = 0;
        uv_timer_stop(handle);
        appstate = 1;
        releaseloop(f, simdata, simmap);
        //int r = simfree(simdata, simmap, f->sim);
        //y_log_message(Y_LOG_LEVEL_DEBUG, "simfree returned %i.", r);

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
    }
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
        char* tmp_launch_exe = game_compat_info[i].LaunchExe;
        gamepid = IsProcessRunning(tmp_launch_exe);
        if(gamepid > 0)
        {
            sim = game_compat_info[i].GameId;
            y_log_message(Y_LOG_LEVEL_INFO, "found a specified launch process for gameid %i running at pid %i.", sim, gamepid);
            break;
        }
    }
    if(gamepid <= 0 && sim <= 0)
    {
        i = -1;
        SimInfo si;
        sim = getSimExe(&si);
        gamepid = si.pid;
    }


    int err = 0;
    if(sim > 0)
    {
        y_log_message(Y_LOG_LEVEL_INFO, "Detected simulator id %i, starting appropriate bridge if necessary.", sim);


        f->game_pid = gamepid;
        if(does_sim_need_bridge(sim) == true && err == 0 && i > -1)
        {

            pid_t pid = gamepid;
            const char* env_var1 = "STEAM_COMPAT_TOOL_PATHS"; // this will need to be interpreted
            const char* env_var2 = "STEAM_COMPAT_DATA_PATH"; // this will be exactly what we need except append /pfx
            const char* env_var3 = "SIMD_BRIDGE_EXE"; // this will be exactly what we need
            const char* env_var4 = "SIMD_WRAP_EXE"; // this will be exactly what we need
            char* env_steam_compat_tool = getEnvValueForPid(pid, env_var1);
            char* env_steam_compat_data = getEnvValueForPid(pid, env_var2);
            char* env_simd_bridge_exe = getEnvValueForPid(pid, env_var3);
            char* env_simd_wrap_exe = getEnvValueForPid(pid, env_var4);

            if(env_steam_compat_tool == NULL || env_steam_compat_data == NULL || env_simd_bridge_exe == NULL)
            {
                err = -1;
                y_log_message(Y_LOG_LEVEL_WARNING, "Could not find one or all of the necessary environment variables. Found %s %s %s", env_steam_compat_tool, env_steam_compat_data, env_simd_bridge_exe);
                free(env_steam_compat_tool);
                free(env_steam_compat_data);
                free(env_simd_bridge_exe);
                free(env_simd_wrap_exe);
            }
            else
            {
                y_log_message(Y_LOG_LEVEL_DEBUG, "Retrieved env vars %s and %s and %s", env_steam_compat_tool, env_steam_compat_data, env_simd_bridge_exe);
            }

            char* wineprefix = NULL;
            if(err == 0)
            {
                if(env_steam_compat_data != NULL)
                {
                    asprintf(&wineprefix, "WINEPREFIX=%s/pfx", env_steam_compat_data);
                    free(env_steam_compat_data);
                }
            }

            char* wineexe = NULL;
            if(err == 0)
            {
                char* token = strtok(env_steam_compat_tool, ":");
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
                free(env_steam_compat_tool);

                if(wineexe != NULL)
                {
                    y_log_message(Y_LOG_LEVEL_DEBUG, "Determined wine executable path %s", wineexe);
                }
            }


            if(err == 0)
            {
                y_log_message(Y_LOG_LEVEL_DEBUG, "No errors found, will attempt to fork a process like this WINEFSYNC=1 %s %s %s", wineprefix, wineexe, env_simd_bridge_exe);

                static char* newargv[]= {"/usr/bin/steam-run", "/usr/bin/wine", "/home/user/git/simshmbridge/assets/acbridge.exe", NULL};
                static char* newenviron[]= {"WINEPREFIX=/home/user/.local/share/Steam/steamapps/compatdata/244210", "WINEFSYNC=1", NULL};

                if(env_simd_wrap_exe == NULL)
                {
                    newargv[0] = wineexe;
                    newargv[1] = env_simd_bridge_exe;
                    newargv[2] = NULL;
                }
                else
                {
                    y_log_message(Y_LOG_LEVEL_DEBUG, "Using wrap exe path %s", env_simd_wrap_exe);
                    newargv[0] = env_simd_wrap_exe;
                    newargv[1] = wineexe;
                    newargv[2] = env_simd_bridge_exe;
                }
                newenviron[0] = wineprefix;

                //newenviron[2] = winebin;

                uint8_t ret = 0;
                pid_t process;
                process = fork();
                f->bridge_pid = process;
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

                    if(env_simd_wrap_exe == NULL)
                    {
                        ret = execve(wineexe, newargv, newenviron);
                    }
                    else
                    {
                        ret = execve(env_simd_wrap_exe, newargv, newenviron);
                    }
                    _exit(127);
                }
                free(wineexe);
                free(wineprefix);
                free(env_simd_bridge_exe);
                free(env_simd_wrap_exe);

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
        }
        else
        {
            y_log_message(Y_LOG_LEVEL_DEBUG, "sim %i does not require a compatibility exe, will continue to mapping data", sim);
            uv_timer_start(&datachecktimer, datacheckcallback, 0, 1000);
            uv_timer_stop(handle);
        }

        if(err == 0)
        {
            if(simds.notify == true)
            {
                char cmd[512];
                const char* gamename = simapi_gametofullstr(sim);
                snprintf(cmd, sizeof(cmd), "notify-send -t 3000 \"%s\" \"Detected %s (%i)\"", "simd", gamename, sim);
                system(cmd);
            }
        }
    }
    if (appstate == 0)
    {
        y_log_message(Y_LOG_LEVEL_INFO, "stopping checking for exe");
        uv_timer_stop(handle);
    }
}


void udpstart(LoopData* f, SimData* simdata, SimMap* simmap)
{
    if (appstate == 2)
    {
        simdatamap(simdata, NULL, simmap2, f->sim, true, NULL);
    }
}

void datacheckcallback(uv_timer_t* handle)
{
    y_log_message(Y_LOG_LEVEL_DEBUG, "datacheckcallback triggered");
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
        if ( appstate == 1 )
        {
            appstate++;

            //simdata->tyrediameter[0] = -1;
            //simdata->tyrediameter[1] = -1;
            //simdata->tyrediameter[2] = -1;
            //simdata->tyrediameter[3] = -1;

            if(f->use_udp == true)
            {
                y_log_message(Y_LOG_LEVEL_INFO, "using udp for this sim title");
                udpstart(f, simdata, simmap);
                uv_udp_recv_start(&recv_socket, on_alloc, on_udp_recv);
            }
            else
            {
                uv_timer_start(&datamaptimer, shmdatamapcallback, 2000, 16);
            }
            uv_timer_stop(handle);
            // i can make this more frequent but i need to be conscious of resources, don't want to trash anyone's frame rates
            uv_timer_start(&bridgeclosetimer, bridgeclosecallback, 5, 5000);
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
        if(f->releasing == false)
        {
            appstate--;
            y_log_message(Y_LOG_LEVEL_INFO, "User requested stop appstate is now %i", appstate);
            fflush(stdout);
        }
    }

    if (appstate == 0)
    {
        y_log_message(Y_LOG_LEVEL_INFO, "simd is exiting...");
        uv_timer_stop(&datachecktimer);
        if (recv_socket_initialized)
        {
            uv_udp_recv_stop(&recv_socket);
        }
        uv_timer_stop(&bridgeclosetimer);
        uv_timer_stop(&gamefindtimer);
        uv_poll_stop(handle);
    }
}

/**
 * @brief Main entry point of the program.
 *
 * This function initializes the application and starts the main loop.
 *
 * @return Exit status of the program.
 */
int main(int argc, char** argv)
{
    char* home_dir_str = getpwuid(getuid())->pw_dir;
    if(home_dir_str == NULL)
    {
        fprintf(stderr, "You need a home directory");
        return 0;
    }
    simds.home_dir = strdup(home_dir_str);


    // cli parameters
    p = malloc(sizeof(Parameters));
    ConfigError ppe = getParameters(argc, argv, p);
    set_settings(p, &simds);
    if (ppe == E_SUCCESS_AND_EXIT)
    {
        goto cleanup_final;
    }
    int ylog_mode = Y_LOG_MODE_FILE;
    if(simds.daemon == false)
    {
        ylog_mode = Y_LOG_MODE_CONSOLE;
    }


    int pid_file_fd = open(PID_FILE, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if(pid_file_fd == -1)
    {
        if( simds.poke == true )
        {
            fprintf(stderr, "poke enabled and attempting poke.\n");
            poke(simds);
        }
        fprintf(stderr, "simd daemon already running, please remove /tmp/simd.pid if this is not the case.\n");

        goto cleanup_final;
    }
    close(pid_file_fd);

    y_init_logs("simd", ylog_mode, Y_LOG_LEVEL_DEBUG, "/tmp/simd.log", "Initializing logs mode: file, logs level: debug");
    y_log_message(Y_LOG_LEVEL_INFO, "Started. Found home directory and interpreted parameters.\n");

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
        good_config = true;
    }
    config_destroy(&cfg);


    if(good_config == true)
    {
        compat_info_size = getNumberOfConfigs(simds.configfile);
        game_compat_info = malloc(compat_info_size * sizeof(GameCompatInfo));

        loadconfig(simds, compat_info_size, game_compat_info);

        y_log_message(Y_LOG_LEVEL_INFO, "Successfullly loaded configuration file");
    }

    struct termios newsettings, canonicalmode;
    if(simds.daemon == true)
    {
        pid_t pid;

        /* Fork off the parent process */
        pid = fork();

        /* An error occurred */
        if (pid < 0)
        {
            exit(EXIT_FAILURE);
        }

        /* Success: Let the parent terminate */
        if (pid > 0)
        {
            exit(EXIT_SUCCESS);
        }

        /* On success: The child process becomes session leader */
        if (setsid() < 0)
        {
            exit(EXIT_FAILURE);
        }

        /* Catch, ignore and handle signals */
        //TODO: Implement a working signal handler */
        signal(SIGCHLD, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        signal(SIGTERM, handle_sigterm);
        /* Fork off for the second time*/
        pid = fork();

        /* An error occurred */
        if (pid < 0)
        {
            exit(EXIT_FAILURE);
        }

        /* Success: Let the parent terminate */
        if (pid > 0)
        {
            exit(EXIT_SUCCESS);
        }

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
    if (simmap2->fd != -1)
    {
        y_log_message(Y_LOG_LEVEL_INFO, "Successfully opened universal shared memory (fd: %d)", simmap2->fd);
    }
    else
    {
        y_log_message(Y_LOG_LEVEL_ERROR, "Failed to open universal shared memory!");
    }

    simdata->simapiversion = SIMAPI_VERSION;
    simdmap(simmap2, simdata);
    y_log_message(Y_LOG_LEVEL_INFO, "SimApi Version: %i\n", simdata->simapiversion);

    baton = (LoopData*) malloc(sizeof(LoopData));
    baton->simmap = simmap;
    baton->simmap2 = simmap2;
    baton->simdata = simdata;
    baton->simstate = false;
    baton->uion = false;
    baton->releasing = false;
    baton->sim = 0;
    baton->bridge_pid = 0;
    baton->game_pid = 0;
    baton->simds = simds;
    baton->compat_info_size = compat_info_size;
    baton->game_compat_info = game_compat_info;
    baton->req.data = (void*) baton;


    appstate = 1;
    y_log_message(Y_LOG_LEVEL_DEBUG, "setting initial app state");
    uv_timer_init(uv_default_loop(), &gamefindtimer);
    uv_timer_init(uv_default_loop(), &bridgeclosetimer);
    uv_timer_init(uv_default_loop(), &datachecktimer);
    uv_timer_init(uv_default_loop(), &datamaptimer);

    uv_handle_set_data((uv_handle_t*) &gamefindtimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &bridgeclosetimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &datachecktimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &datamaptimer, (void*) baton);

    y_log_message(Y_LOG_LEVEL_INFO, "Searching for sim data... Press q to quit...\n");
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

    if(simds.daemon == false)
    {
        uv_poll_init(uv_default_loop(), &pollt, 0);
        uv_handle_set_data((uv_handle_t*) &pollt, (void*) baton);
        uv_poll_start(&pollt, UV_READABLE, cb);
    }

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    if(simds.daemon==false)
    {
        fflush(stdout);
        tcsetattr(0, TCSANOW, &canonicalmode);
        uv_poll_stop(&pollt);
        release();
    }

cleanup_final:

    y_close_logs();
    return 0;
}
