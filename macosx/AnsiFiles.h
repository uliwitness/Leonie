#pragma once

#include <unistd.h> // getcwd()
#include "fake_filesystem.hpp"	// until <filesystem> ships for Xcode's clang.
using namespace fake;