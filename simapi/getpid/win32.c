#ifdef _WIN32
/* Windows process discovery; replaces the Linux getpid.c implementation. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../simapi.h"

/* Native Windows process detection; no procfs/libproc2 or Flatpak. */
static int simapi_name_contains(const char *name, const char *part)
{
    size_t n = strlen(part);
    if (!n) return 0;
    for (; *name; name++)
        if (_strnicmp(name, part, n) == 0) return 1;
    return 0;
}

struct SimProcessInfo get_process_match(char *names[], int count)
{
    struct SimProcessInfo match = {-1, -1};
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return match;

    PROCESSENTRY32A entry;
    memset(&entry, 0, sizeof(entry));
    entry.dwSize = sizeof(entry);
    if (Process32FirstA(snap, &entry)) {
        do {
            for (int i = 0; i < count && names[i]; ++i) {
                if (simapi_name_contains(entry.szExeFile, names[i])) {
                    match.pid = (int)entry.th32ProcessID;
                    match.pos = i;
                    CloseHandle(snap);
                    return match;
                }
            }
        } while (Process32NextA(snap, &entry));
    }
    CloseHandle(snap);
    return match;
}

int is_pid_running(simapi_pid_t pid)
{
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
                                 (DWORD)pid);
    if (!process) return 0;
    DWORD code = 0;
    int running = GetExitCodeProcess(process, &code) && code == STILL_ACTIVE;
    CloseHandle(process);
    return running;
}

int simapi_in_flatpak(void) { return 0; }
int simapi_host_pid_alive(simapi_pid_t pid) { return is_pid_running(pid); }

int simapi_host_file_exists(const char *path)
{
    if (!path) return 0;
    DWORD attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES &&
           !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

char *simapi_host_capture(const char *cmd, size_t *out_len)
{
    (void)cmd;
    if (out_len) *out_len = 0;
    return NULL; /* Flatpak-specific helper; not applicable to Windows. */
}

char *getEnvValueForPid(simapi_pid_t pid, const char *name)
{
    (void)pid;
    (void)name;
    /* Reading another process's environment requires separate Win32 work. */
    return NULL;
}

#endif /* _WIN32 */
