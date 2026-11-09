/* Build entry point: platform-specific process discovery.
 * Implementation files are included, not separately compiled.
 */
#if defined(_WIN32) || defined(OS_WIN)
#include "getpid/win32.c"
#else
#include "getpid/posix.c"
#endif
