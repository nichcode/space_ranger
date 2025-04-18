
#pragma once

#include "PAL_defines.h"

/// @brief Log types
enum PAL_LogLevels_
{
    PAL_LogLevels_Trace,
    PAL_LogLevels_Info,
    PAL_LogLevels_Warn,
    PAL_LogLevels_Error
};

/**
 * @brief Log a message to the console.
 * @param level The log level of the message
 * @param msg The message to log. This uses c-style formatting
 * @example %i, %f
 */
PAL_API void PAL_Log(u32 level, const char* msg, ...);

/**
 * @brief Log a trace message to the console.
 * @param msg The message to log. This uses c-style formatting
 * @example %i, %f
 */
PAL_API void PAL_LogTrace(const char* msg, ...);

/**
 * @brief Log a info message to the console.
 * @param msg The message to log. This uses c-style formatting
 * @example %i, %f
 */
PAL_API void PAL_LogInfo(const char* msg, ...);

/**
 * @brief Log a warn message to the console.
 * @param msg The message to log. This uses c-style formatting
 * @example %i, %f
 */
PAL_API void PAL_LogWarn(const char* msg, ...);

/**
 * @brief Log a error message to the console.
 * @param msg The message to log. This uses c-style formatting
 * @example %i, %f
 */
PAL_API void PAL_LogError(const char* msg, ...);