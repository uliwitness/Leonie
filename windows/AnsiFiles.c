#include "AnsiFiles.h"
#include <assert.h>
#include <stddef.h>

FILE* LEOFOpen(const char* path, const char* mode)
{
	FILE* theFile = NULL;
	errno_t err = fopen_s(&theFile, path, mode);
	if (err != 0) theFile = NULL;
	return theFile;
}
