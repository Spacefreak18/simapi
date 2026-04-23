#include <stdio.h>
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
#include "getpid.h"

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

    envFile = fopen(path, "r");
    if ( envFile == NULL )
    {
//        errno = ESRCH;
        return NULL;
    }

    maxIdx = readData(&buf, envFile);

    fclose(envFile);

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


struct SimProcessInfo pidof (char* pname[], int num)
{

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
                if (strncmp(cm, pname[i], 15) == 0) {
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


