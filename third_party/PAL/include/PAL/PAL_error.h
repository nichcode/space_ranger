
#pragma once

#include "PAL_defines.h"

/**
 * @brief Get the message about the last error that occurred on the current thread.
 * @attention this function is thread safe.
 * @returns the message of the last error that occurred otherwise an empty string.
 */
PAL_API const char * PAL_GetError();

/**
 * @brief Clear any previous error message for this thread.
 */
PAL_API void PAL_ClearError();