
#pragma once

#include "pal2/pal_core.h"

static inline void logResult(
    PalResult result,
    const char* msg)
{
    char buffer[256];
    palFormatResult(result, 256, buffer);
    palLog(nullptr, "%s \n %s", msg, buffer);
}