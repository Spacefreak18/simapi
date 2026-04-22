#include <sys/types.h>

#define MAX_BUF 1024
#define PID_LIST_BLOCK 32

int is_pid_running(pid_t pid);

struct SimProcessInfo get_process_match(char* pidstrings[], int num);

char* getEnvValueForPid(pid_t pid, const char* envName);
