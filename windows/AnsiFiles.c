#include "AnsiFiles.h"
#include <assert.h>
#include <stddef.h>

const char* getcwd(char* buf, size_t bufSize)
{
	assert(bufSize > 0);
	buf[0] = 0;
	return buf;
}

FILE* LEOFOpen(const char* path, const char* mode)
{
	FILE* theFile = NULL;
	errno_t err = fopen_s(&theFile, path, mode);
	if (err != 0) theFile = NULL;
	return theFile;
}
