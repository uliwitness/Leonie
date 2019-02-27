#pragma once

#include <stddef.h>
#include <stdio.h>

#if __cplusplus
#include <filesystem>
using namespace std;

extern "C" {
#endif

const char* getcwd(char* buf, size_t bufSize);
FILE* LEOFOpen(const char* path, const char* mode);

#if __cplusplus
}
#endif
