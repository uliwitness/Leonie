#include "AnsiFiles.h"
#include <cassert>

void strncpy_s(char * dst, const char* src, size_t len)
{
	size_t written = strncpy(dst, src, len -1);
	dst[written] = '\0';
}