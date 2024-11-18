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

#include <simdata.h>
#include <simmapper.h>

#include "loopdata.h"
#include "parameters.h"

bool compatmemmap;

loop_data* baton;
SimData* simdata;
SimMap* simmap;
SimMap* simmap2;
SimCompatMap* compatmap;

uv_idle_t idler;
uv_timer_t datachecktimer;
uv_timer_t datamaptimer;
uv_udp_t recv_socket;



bool doui = false;
int appstate = 0;

void shmdatamapcallback(uv_timer_t* handle);
void datacheckcallback(uv_timer_t* handle);

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


void release()
{

    uv_timer_stop(&datamaptimer);
    uv_timer_stop(&datachecktimer);

    if(compatmemmap == true)
    {
        freesimcompatmap(compatmap);
        free(compatmap);
    }
    freesimmap(simmap);
    freesimmap(simmap2);

    free(baton);
    free(simdata);

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
    loop_data* f = (loop_data*) b;
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
            f->numdevices = 0;
            y_log_message(Y_LOG_LEVEL_INFO, "stopped mapping data, press q again to quit");
            //stopui(ms->ui_type, f);
            // free loop data

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
    loop_data* f = (loop_data*) b;
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

void datacheckcallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;
    SimMap* simmap2 = f->simmap2;

    if ( appstate == 1 )
    {
        SimInfo si = getSim(simdata, simmap, false, startudp);
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
            doui = true;
            simdata->tyrediameter[0] = -1;
            simdata->tyrediameter[1] = -1;
            simdata->tyrediameter[2] = -1;
            simdata->tyrediameter[3] = -1;

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
    loop_data* f = (loop_data*) b;
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
        uv_poll_stop(handle);
    }
}


int main(int argc, char** argv)
{

    Parameters* p = malloc(sizeof(Parameters));

    ConfigError ppe = getParameters(argc, argv, p);

    if (ppe == E_SUCCESS_AND_EXIT)
    {
        goto cleanup_final;
    }

    struct termios newsettings, canonicalmode;
    if(p->daemon == true)
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

    y_init_logs("simd", Y_LOG_MODE_FILE, Y_LOG_LEVEL_DEBUG, "/tmp/simd.log", "Initializing logs mode: file, logs level: debug");

    set_simapi_log_info(simapilib_loginfo);
    if(p->verbosity_count>0)
    {
        set_simapi_log_debug(simapilib_logdebug);
        set_simapi_log_trace(simapilib_logtrace);
    }

    simdata = malloc(sizeof(SimData));
    simmap = createSimMap();
    simmap2 = createSimMap();

    compatmemmap = false;
    if(p->memmap == true)
    {
        compatmemmap = true;
        compatmap = malloc(sizeof(SimCompatMap));
        opensimcompatmap(compatmap);
    }

    opensimmap(simmap2);

    simdata->simversion = SIMAPI_VERSION;
    simdmap(simmap2, simdata);

    baton = (loop_data*) malloc(sizeof(loop_data));
    baton->simmap = simmap;
    baton->simmap2 = simmap2;
    baton->simdata = simdata;
    baton->simstate = false;
    baton->uion = false;
    baton->releasing = false;
    baton->sim = 0;
    baton->req.data = (void*) baton;


    uv_handle_set_data((uv_handle_t*) &datachecktimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &datamaptimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &recv_socket, (void*) baton);

    appstate = 1;
    y_log_message(Y_LOG_LEVEL_DEBUG, "setting initial app state");
    uv_timer_init(uv_default_loop(), &datachecktimer);
    if(p->daemon == false)
    {
        fprintf(stdout, "Searching for sim data... Press q to quit...\n");
    }
    uv_timer_start(&datachecktimer, datacheckcallback, 1000, 1000);


    if(p->daemon == false)
    {
        uv_poll_t* poll = (uv_poll_t*) malloc(uv_handle_size(UV_POLL));
        uv_handle_set_data((uv_handle_t*) poll, (void*) baton);
        if (0 != uv_poll_init(uv_default_loop(), poll, 0))
        {
            return 1;
        };
        if (0 != uv_poll_start(poll, UV_READABLE, cb))
        {
            return 2;
        };
    }

    uv_timer_init(uv_default_loop(), &datamaptimer);

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    if(p->daemon==false)
    {
        fprintf(stdout, "\n");
        fflush(stdout);
        tcsetattr(0, TCSANOW, &canonicalmode);
        release();
    }
    //freesimcompatmap(compatmap);
    //freesimmap(simmap2);

    //free(baton);
    //free(simdata);
    //free(simmap);
    //free(simmap2);
    //free(compatmap);

    //y_close_logs();
cleanup_final:

    return 0;
}

//int main()
//{
//
//    y_init_logs("simd", Y_LOG_MODE_FILE, Y_LOG_LEVEL_DEBUG, "/tmp/simd.log", "Initializing logs mode: file, logs level: debug");
//
//    SimData* simdata = malloc(sizeof(SimData));
//    SimMap* simmap = malloc(sizeof(SimMap));
//    SimMap* simmap2 = malloc(sizeof(SimMap));
//    SimCompatMap* compatmap = malloc(sizeof(SimCompatMap));
//
//    struct termios newsettings, canonicalmode;
//    tcgetattr(0, &canonicalmode);
//    newsettings = canonicalmode;
//    newsettings.c_lflag &= (~ICANON & ~ECHO);
//    newsettings.c_cc[VMIN] = 1;
//    newsettings.c_cc[VTIME] = 0;
//    tcsetattr(0, TCSANOW, &newsettings);
//    char ch;
//    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
//
//    uv_poll_t* poll = (uv_poll_t*) malloc(uv_handle_size(UV_POLL));
//
//    opensimmap(simmap2);
//    opensimcompatmap(compatmap);
//
//    simdata->simversion = SIMAPI_VERSION;
//    simdmap(simmap2, simdata);
//
//    loop_data* baton = (loop_data*) malloc(sizeof(loop_data));
//    baton->simmap = simmap;
//    baton->simmap2 = simmap2;
//    baton->simdata = simdata;
//    baton->simstate = false;
//    baton->uion = false;
//    baton->releasing = false;
//    baton->sim = 0;
//    baton->req.data = (void*) baton;
//    uv_handle_set_data((uv_handle_t*) &datachecktimer, (void*) baton);
//    uv_handle_set_data((uv_handle_t*) &datamaptimer, (void*) baton);
//    uv_handle_set_data((uv_handle_t*) &recv_socket, (void*) baton);
//    uv_handle_set_data((uv_handle_t*) poll, (void*) baton);
//    appstate = 1;
//    y_log_message(Y_LOG_LEVEL_DEBUG, "setting initial app state");
//    uv_timer_init(uv_default_loop(), &datachecktimer);
//    fprintf(stdout, "Searching for sim data... Press q to quit...\n");
//    uv_timer_start(&datachecktimer, datacheckcallback, 1000, 1000);
//
//    set_simapi_log_info(simapilib_loginfo);
//    set_simapi_log_debug(simapilib_logdebug);
//    set_simapi_log_trace(simapilib_logtrace);
//
//
//    if (0 != uv_poll_init(uv_default_loop(), poll, 0))
//    {
//        return 1;
//    };
//    if (0 != uv_poll_start(poll, UV_READABLE, cb))
//    {
//        return 2;
//    };
//
//
//    uv_timer_init(uv_default_loop(), &datamaptimer);
//
//    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
//    fprintf(stdout, "\n");
//    fflush(stdout);
//    tcsetattr(0, TCSANOW, &canonicalmode);
//
//    freesimcompatmap(compatmap);
//    freesimmap(simmap2);
//
//    free(baton);
//    free(simdata);
//    free(simmap);
//    free(simmap2);
//    free(compatmap);
//
//    y_close_logs();
//    return 0;
//}

