// The output from the GNU flex tool always contains "#include <unistd.h>"
// and Microsoft does not provide that file. So this file allows it to compile
#ifndef _WIN32
#include "unistd.h"
#endif

