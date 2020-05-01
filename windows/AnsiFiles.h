#pragma once

#include <stddef.h>
#include <stdio.h>
#include <direct.h>

#if __cplusplus
#include <filesystem>
using namespace std;

extern "C" {
#endif

#define getcwd _getcwd
FILE* LEOFOpen(const char* path, const char* mode);

#if __cplusplus
}
#endif
