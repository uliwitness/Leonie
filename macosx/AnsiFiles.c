#include "AnsiFiles.h"
#include <cassert>

const char* getcwd(char* buf, size_t bufSize)
{
	assert(bufSize > 0);
	buf[0] = 0;
	return buf;
}