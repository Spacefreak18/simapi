/* Platform dispatch. The implementations are included as a single TU.
 * Keep them out of CMake's source list.
 */
#if defined(_WIN32) || defined(OS_WIN)
#include "simmapper/win32.c"
#else
#include "simmapper/posix.c"
#endif
