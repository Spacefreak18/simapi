#include <sys/types.h>

#define MAX_BUF 1024
#define PID_LIST_BLOCK 32

int is_pid_running(pid_t pid);

int IsProcessRunning(char* pidstring);

char* getEnvValueForPid(pid_t pid, const char* envName);
