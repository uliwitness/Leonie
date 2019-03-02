#pragma once

#include <unistd.h> // getcwd()

#if __cplusplus
#include "fake_filesystem.hpp"	// until <filesystem> ships for Xcode's clang.
using namespace fake;
#endif

#define LEOFOpen fopen
