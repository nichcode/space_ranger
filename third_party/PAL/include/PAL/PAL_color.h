
#pragma once

#include "PAL_defines.h"

/// @brief Color
struct PAL_Color
{
    /// @brief red component of the color
    f32 r = 0.0f;

    /// @brief green component of the color
    f32 g = 0.0f;

    /// @brief blue component of the color
    f32 b = 0.0f;

    /// @brief alpha component of the color
    f32 a = 0.0f;
};

/**
 * @brief Create a color from a set of rgba values in (0-255) range
 * @param r The red component of the color
 * @param g The green component of the color
 * @param b The blue component of the color
 * @param a The alpha component of the color
 * @returns a color
 */
inline PAL_Color PAL_ColorFromU8(u8 r, u8 g, u8 b, u8 a)
{
    PAL_Color color;
    color.r = (f32)r / 255.0f;
    color.g = (f32)g / 255.0f;
    color.b = (f32)b / 255.0f;
    color.a = (f32)a / 255.0f;

    return color;
}

/// @brief Color red
#define PAL_RED { 1.0f, 0.0f, 0.0f, 1.0f }

/// @brief Color green
#define PAL_GREEN { 0.0f, 1.0f, 0.0f, 1.0f }

/// @brief Color blue
#define PAL_BLUE { 0.0f, 0.0f, 1.0f, 1.0f }

/// @brief Color white
#define PAL_WHITE { 1.0f, 1.0f, 1.0f, 1.0f }

/// @brief Color black
#define PAL_BLACK { 0.0f, 0.0f, 0.0f, 1.0f }