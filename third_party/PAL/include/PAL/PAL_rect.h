
#pragma once

#include "PAL_defines.h"

/// @brief Rect
struct PAL_Rect
{
    /// @brief x position of the rect
    f32 x = 0.0f;

    /// @brief y position of the rect
    f32 y = 0.0f;
    
    f32 w = 0.0f;
    /// @brief width of the rect
    
    f32 h = 0.0f;
    /// @brief height of the rect
};

/**
 * @brief Check AABB collision between two rects
 * @param a The first rect
 * @param b The second rect
 * @returns If successful, true otherwise false.
 */
PAL_API b8 PAL_RectCollide(const PAL_Rect a, const PAL_Rect b);