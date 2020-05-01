#pragma once

#include <string.h>

#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define strlcpy(dst, src, siz) strncpy_s((dst), (siz), (src), (siz))