
#pragma once

#include "pal2/pal_core.h"
#include <cstdlib>

#define WORLD_WIDTH 1280.0f
#define WORLD_HEIGHT 720.0f

#if defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK() __builtin_trap()
#endif // _MSC_VER

static inline void logResult(
    PalResult result,
    const char* msg)
{
    char buffer[256];
    palFormatResult(result, 256, buffer);
    palLog(nullptr, "%s \n %s", msg, buffer);
}