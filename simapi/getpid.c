#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <libgen.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <libproc2/pids.h>

#include "simapi.h"

static int isMatch(const char* possibleMatch, const char* checkAgainst)
{
    size_t i;

    for( i=0; checkAgainst[i] != '\0'; i++)
    {
        if ( possibleMatch[i] == '\0' )
        {
            /* If other string ended before we matched */
            return 0;
        }
        else
            if ( possibleMatch[i] != checkAgainst[i] )
            {
                /* If current index does not match */
                return 0;
            }
    }

    /* We matched it all */
    return 1;
}

int is_pid_running(pid_t pid)
{
    if (simapi_in_flatpak())
    {
        return simapi_host_pid_alive(pid);
    }

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

#define READ_DATA_INCR_BUFSIZ 65535

static size_t readData(char** buf, FILE *envFile)
{
    size_t curSize = READ_DATA_INCR_BUFSIZ;
    size_t bytesRead;
    size_t totalBytesRead = 0;

    char* cur;

    *buf = malloc(curSize + 1);

    cur = *buf;

_read_again:

    bytesRead = fread(cur, 1, READ_DATA_INCR_BUFSIZ, envFile);

    totalBytesRead += bytesRead;

    if ( bytesRead == READ_DATA_INCR_BUFSIZ || !feof(envFile) )
    {
        curSize += READ_DATA_INCR_BUFSIZ;

        *buf = realloc(*buf, curSize);

        cur = &buf[0][totalBytesRead];
        goto _read_again;
    }

    return totalBytesRead;
}


char* getEnvValueForPid(pid_t pid, const char* envName)
{
    static char path[64] = { '/', 'p', 'r', 'o', 'c', '/', 0 };

    char* buf, *cur, *val, *ret;
    FILE *envFile;
    size_t idx, maxIdx, thisLen, envNameLen;

    /* "Not Found" marker */
    ret = NULL;

    sprintf( &path[6], "%d/environ", pid );

    /* The pid came from the host's process table, so its /proc entry exists
     * only out there -- see simapi_host_capture(). */
    if ( simapi_in_flatpak() )
    {
        char hostcmd[96];
        snprintf(hostcmd, sizeof(hostcmd), "cat /proc/%d/environ", (int) pid);
        buf = simapi_host_capture(hostcmd, &maxIdx);
        if ( buf == NULL )
        {
            return NULL;
        }
    }
    else
    {
        envFile = fopen(path, "r");
        if ( envFile == NULL )
        {
//        errno = ESRCH;
            return NULL;
        }

        maxIdx = readData(&buf, envFile);

        fclose(envFile);
    }

    envNameLen = strlen(envName);
    cur = buf;
    idx = 0;


    while( idx < maxIdx )
    {
        thisLen = strlen(cur);

        if( !isMatch(cur, envName) || cur[envNameLen] != '=' )
        {
            cur = &cur[thisLen + 1];
            idx += thisLen + 1;
            continue;
        }
        val = cur + envNameLen + 1;

        ret = malloc(thisLen - envNameLen);
        strcpy(ret, val);

        break;
    }

    free(buf);

    return ret;
}


/* checks if the string is purely an integer
 * we can do it with `strtol' also
 */
int check_if_number (char* str)
{
    int i;
    for (i=0; str[i] != '\0'; i++)
    {
        if (!isdigit (str[i]))
        {
            return 0;
        }
    }
    return 1;
}



/* ------------------------------------------------------------------------
 * Looking at the host from inside a Flatpak sandbox.
 *
 * A sandbox gets its own PID namespace -- measured on a real install, 5 pids
 * visible inside against 497 on the host -- so the /proc walk below finds only
 * the sandbox itself and no sim is ever detected. Flatpak has no option to
 * share the host's namespace (--allow=devel and --filesystem=host both change
 * nothing), leaving `flatpak-spawn --host`, which asks the session helper to
 * run a command outside. That needs --talk-name=org.freedesktop.Flatpak in the
 * manifest.
 *
 * The output comes back through a FILE, not a pipe. A popen()'d
 * `flatpak-spawn --host ...` hands the pipe's write end to the session helper,
 * and a copy of it outlives every process the sandbox can see: reading to EOF
 * never returns. Measured -- simd sat in anon_pipe_read with no children left
 * and no further log line, forever. So the host writes to a file under
 * $XDG_RUNTIME_DIR/app/$FLATPAK_ID, the one directory mounted at the same path
 * on both sides, and renames it into place; the sandbox then reads an ordinary
 * file where EOF means what it says. The rename is what makes "the file
 * exists" mean "the output is complete".
 *
 * None of this is reached outside a sandbox: every caller keeps its original
 * code path, so native builds are byte-for-byte unaffected.
 * --------------------------------------------------------------------- */
int simapi_in_flatpak(void)
{
    static int cached = -1;
    if (cached < 0)
    {
        cached = (access("/.flatpak-info", F_OK) == 0) ? 1 : 0;
    }
    return cached;
}

/* Visible under the same path inside the sandbox and out. */
static const char* host_share_dir(void)
{
    static char dir[256];
    if (dir[0] == '\0')
    {
        const char* runtime = getenv("XDG_RUNTIME_DIR");
        const char* app_id = getenv("FLATPAK_ID");
        if (runtime != NULL && app_id != NULL)
        {
            snprintf(dir, sizeof(dir), "%s/app/%s", runtime, app_id);
        }
        else
        {
            snprintf(dir, sizeof(dir), "/tmp");
        }
    }
    return dir;
}

/* Runs `hostcmd` on the host and returns its stdout, or NULL. Caller frees. */
char* simapi_host_capture(const char* hostcmd, size_t* out_len)
{
    static unsigned long seq = 0;
    char out[320];
    char cmd[1024];

    if (out_len != NULL)
    {
        *out_len = 0;
    }

    snprintf(out, sizeof(out), "%s/simapi-host-%d-%lu", host_share_dir(), (int) getpid(), seq++);

    /* Written to <out>.part and renamed, so the file only appears complete.
     * The exit status is deliberately ignored: simd runs a libuv loop whose
     * SIGCHLD handling reaps children this call did not spawn, which makes
     * system()'s own waitpid fail and lose the status. */
    snprintf(cmd, sizeof(cmd),
             /* hostcmd is grouped: it may itself be a sequence, and without
              * the braces the redirect would bind to its last command only --
              * which silently dropped half the process table. */
             "flatpak-spawn --host sh -c '{ %s ; } > %s.part 2>/dev/null; mv %s.part %s' >/dev/null 2>&1",
             hostcmd, out, out, out);
    (void) system(cmd);

    /* system() normally returns only once the command is done; the wait can
     * still be lost to that same reaping, so give the rename a moment. */
    FILE* f = NULL;
    for (int waited = 0; waited < 2000; waited += 20)
    {
        f = fopen(out, "r");
        if (f != NULL)
        {
            break;
        }
        usleep(20 * 1000);
    }

    if (f == NULL)
    {
        return NULL;
    }

    size_t cap = 8192;
    size_t len = 0;
    char* buf = malloc(cap);

    if (buf != NULL)
    {
        for (;;)
        {
            size_t n = fread(buf + len, 1, cap - len - 1, f);
            len += n;
            if (len + 1 < cap)
            {
                break;      /* short read on a real file means EOF */
            }
            cap *= 2;
            char* grown = realloc(buf, cap);
            if (grown == NULL)
            {
                break;
            }
            buf = grown;
        }
        buf[len] = '\0';
    }

    fclose(f);
    unlink(out);

    if (out_len != NULL)
    {
        *out_len = len;
    }
    return buf;
}

/* Is a pid in the *host's* namespace alive? kill(2) would resolve the number
 * against the sandbox's own processes -- usually nothing, occasionally the
 * wrong one. */
int simapi_host_pid_alive(pid_t pid)
{
    char hostcmd[96];
    snprintf(hostcmd, sizeof(hostcmd), "kill -0 %d 2>/dev/null && echo alive", (int) pid);

    char* out = simapi_host_capture(hostcmd, NULL);
    int alive = (out != NULL && strncmp(out, "alive", 5) == 0);
    free(out);
    return alive;
}

/* Does a path exist on the *host*? For paths that are the host's by nature --
 * a Proton binary under STEAM_COMPAT_TOOL_PATHS, say -- since the sandbox's
 * own filesystem knows nothing about them. Checking locally is how the bridge
 * launch failed silently: Proton was never found, so the fork never happened
 * and nothing was logged above debug level. */
int simapi_host_file_exists(const char* path)
{
    char hostcmd[1024];
    snprintf(hostcmd, sizeof(hostcmd), "test -e '%s' && echo yes", path);

    char* out = simapi_host_capture(hostcmd, NULL);
    int exists = (out != NULL && strncmp(out, "yes", 3) == 0);
    free(out);
    return exists;
}

static struct SimProcessInfo pidof_host(char* pname[], int num)
{
    struct SimProcessInfo p;
    p.pid = -1;
    p.pos = -1;

    /* Two ps runs, each line tagged, because a single `ps -o pid=,comm=,args=`
     * cannot be parsed: comm may contain spaces. Wine names Assetto Corsa's
     * process "AC: main thread", which a naive scanf read as comm="AC:" and
     * argv[0]="main" -- nothing matched "acs.exe", simapi never set simstatus,
     * and simd mapped no telemetry at all while the game ran. Splitting the
     * two fields into their own lines keeps each one unambiguous: "A <pid>
     * <args...>" gives argv[0] as the first token after the pid, and
     * "C <pid> <comm...>" gives comm as the whole rest of the line, which is
     * the same pair libproc2 hands the native path below. */
    char* table = simapi_host_capture(
        "ps -A -o pid=,args= | sed \"s/^/A /\"; ps -A -o pid=,comm= | sed \"s/^/C /\"",
        NULL);
    if (table == NULL)
    {
        return p;
    }

    char* saveptr = NULL;
    for (char* line = strtok_r(table, "\n", &saveptr);
         line != NULL;
         line = strtok_r(NULL, "\n", &saveptr))
    {
        char kind = line[0];
        if (kind != 'A' && kind != 'C')
        {
            continue;
        }

        int pid = 0;
        int consumed = 0;
        if (sscanf(line + 1, " %d %n", &pid, &consumed) < 1 || consumed == 0)
        {
            continue;
        }

        char* field = line + 1 + consumed;
        if (kind == 'A')
        {
            /* argv[0] only. The whole of `ps -o args=` would be looser than
             * libproc2's PIDS_CMDLINE_V first element: a shell whose arguments
             * merely mention a sim's exe -- a Steam launch command, say --
             * would match, and simd would track that shell as the game. */
            char* end = strchr(field, ' ');
            if (end != NULL)
            {
                *end = '\0';
            }
        }

        for (int i = 0; pname[i] != NULL && i < num; i++)
        {
            if (strcasestr(field, pname[i]) != NULL)
            {
                p.pid = pid;
                p.pos = i;
                free(table);
                return p;
            }
        }
    }

    free(table);
    return p;
}

struct SimProcessInfo pidof (char* pname[], int num)
{

    if (simapi_in_flatpak())
    {
        return pidof_host(pname, num);
    }

    struct SimProcessInfo p;
    p.pid = -1;
    p.pos = -1;


    enum pids_item items[] = { PIDS_ID_PID, PIDS_CMD, PIDS_CMDLINE_V };
	enum rel_items { rel_pid, rel_cmd, rel_cmdline };

    //enum pids_item items[] = { PIDS_ID_PID, PIDS_CMDLINE };
    //enum rel_items { rel_pid, rel_cmdline }; 
    
    struct pids_info *info = NULL;
    struct pids_stack *stack;


    if (procps_pids_new(&info, items, 3) < 0)
    {
        // do something better here
        return p;
    }

    while ((stack = procps_pids_get(info, PIDS_FETCH_TASKS_ONLY)) != NULL) {

#ifdef USE_OLD_PID_VAL
        int pid = PIDS_VAL(rel_pid, s_int, stack, info);
        char* cm = PIDS_VAL(rel_cmd, str, stack, info);
        char** cmd = PIDS_VAL(rel_cmdline, strv, stack, info);
#else
        int pid = PIDS_VAL(rel_pid, s_int, stack);
        char* cm = PIDS_VAL(rel_cmd, str, stack);
        char** cmd = PIDS_VAL(rel_cmdline, strv, stack);
#endif

        if (cm != NULL) {
            char* cmdline = *cmd;
            for (int i = 0; pname[i] != NULL && i < num; i++) {
                // in the future if we need to do the entire cmdline we can
                // for now we will just do a 15 character match
                //fprintf(stderr, "cmdline is %s, process is %s", cmdline, pname[i]);
                if (strcasestr(cmdline, pname[i]) > 0 || strcasestr(cm, pname[i]) > 0) {
                    struct SimProcessInfo p;
                    p.pid = pid;
                    p.pos = i;
                    return p;
                }
            }
        }
    }

    procps_pids_unref(&info);
    return p;
}


struct SimProcessInfo get_process_match(char* pidstrings[], int num)
{
    struct SimProcessInfo p;
    p = pidof(pidstrings, num);
    return p;
}


