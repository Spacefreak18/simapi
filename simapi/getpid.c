#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <libgen.h>
#include <stdbool.h>
#include "getpid.h"
#include <stdint.h>


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
    char path[2048], cmdline[2048], read_buf[8196], read_buf2[8196];


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
            memset(read_buf, 0, sizeof(read_buf));
            memset(read_buf2, 0, sizeof(read_buf2));

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
                fscanf (fp, "%s", read_buf);
                fp2= fopen (cmdline, "r");
                fscanf (fp2, "%s", read_buf2);
                char* i = strcasestr(read_buf2, pname);
                //fprintf(stderr, "from %s searching for %s in %s from cmdline %s occurs at %s\n", path, pname, read_buf, read_buf2, i);
                if(i>0)
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
                fclose (fp);
            }
        }
    }


    closedir (dirp);
    pidlist[pidlist_index] = -1; /* indicates end of list */
    return pidlist;
}

bool IsProcessRunning(char* pidstring)
{
    int* list, i;

    list = pidof(pidstring);
    bool running = false;
    if(list[0]!=-1)
    {
        running = true;
    }
    free (list);
    return running;
}

//int main()
//{
//    bool b = IsProcessRunning("acs");
//    if(b == true)
//    {
//        fprintf(stderr, "you have a running acs\n");
//    }
//}
