#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <libgen.h>
#include <stdbool.h>
#include "getpid.h"
#include <stdint.h>
#include <sys/types.h>

static int isMatch(const char *possibleMatch, const char *checkAgainst)
{
    size_t i;

    for( i=0; checkAgainst[i] != '\0'; i++)
    {
        if ( possibleMatch[i] == '\0' )
        {
            /* If other string ended before we matched */
            return 0;
        } else if ( possibleMatch[i] != checkAgainst[i] )
        {
            /* If current index does not match */
            return 0;
        }
    }

    /* We matched it all */
    return 1;
}


#define READ_DATA_INCR_BUFSIZ 65535

static size_t readData(char **buf, FILE *envFile)
{
    size_t curSize = READ_DATA_INCR_BUFSIZ;
    size_t bytesRead;
    size_t totalBytesRead = 0;

    char *cur;

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

    char *buf, *cur, *val, *ret;
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

int* pidof (char* pname)
{
    DIR* dirp;
    FILE* fp;
    FILE* fp2;
    struct dirent* entry;
    int* pidlist, pidlist_index = 0, pidlist_realloc_count = 1;
    char path[2048], cmdline[2048], read_buf[16392], read_buf2[16392];


    dirp = opendir ("/proc/");
    if (dirp == NULL)
    {
        perror ("Fail");
        return NULL;
    }

    pidlist = malloc (sizeof (int) * PID_LIST_BLOCK);
    if (pidlist == NULL)
    {
        return NULL;
    }

    while ((entry = readdir (dirp)) != NULL)
    {
        if (check_if_number (entry->d_name))
        {
            memset(&path, 0, sizeof(path));
            memset(&cmdline, 0, sizeof(cmdline));
            memset(&read_buf, 0, sizeof(read_buf));
            memset(&read_buf2, 0, sizeof(read_buf2));

            strcpy (path, "/proc/");
            strcat (path, entry->d_name);
            strcat (path, "/comm");

            strcpy (cmdline, "/proc/");
            strcat (cmdline, entry->d_name);
            strcat (cmdline, "/cmdline");

            /* A file may not exist, it may have been removed.
             * dut to termination of the process. Actually we need to
             * make sure the error is actually file does not exist to
             * be accurate.
             */
            fp = fopen (path, "r");
            if (fp != NULL)
            {
                fgets(read_buf, sizeof read_buf, fp);
                //fscanf (fp, "%s", read_buf);
                fp2= fopen (cmdline, "r");
                fgets(read_buf2, sizeof read_buf2, fp2);
                //fscanf (fp2, "%s", read_buf2);
                char* i = strcasestr(read_buf, pname);
                char* j = strcasestr(read_buf2, pname);
                //fprintf(stderr, "from %s searching for %s in %s from cmdline %s occurs at %s or %s\n", path, pname, read_buf, read_buf2, i, j);
                if(i>0 || j>0)
                {
                    /* add to list and expand list if needed */
                    pidlist[pidlist_index++] = atoi (entry->d_name);
                    if (pidlist_index == PID_LIST_BLOCK* pidlist_realloc_count)
                    {
                        pidlist_realloc_count++;
                        pidlist = realloc (pidlist, sizeof (int) * PID_LIST_BLOCK* pidlist_realloc_count);  //Error check todo
                        if (pidlist == NULL)
                        {
                            return NULL;
                        }
                    }
                }
                fclose(fp);
                fclose(fp2);
            }
        }
    }


    closedir (dirp);
    pidlist[pidlist_index] = -1; /* indicates end of list */
    return pidlist;
}

//bool IsProcessRunning(char* pidstring)
//{
//    int* list, i;
//
//    list = pidof(pidstring);
//    bool running = false;
//    if(list[0]!=-1)
//    {
//        running = true;
//    }
//    free (list);
//    return running;
//}

int IsProcessRunning(char* pidstring)
{
    int* list, i;

    list = pidof(pidstring);

    int ret = list[0];
    free (list);
    return ret;
}

//int main()
//{
//    bool b = IsProcessRunning("acs");
//    if(b == true)
//    {
//        fprintf(stderr, "you have a running acs\n");
//    }
//}
