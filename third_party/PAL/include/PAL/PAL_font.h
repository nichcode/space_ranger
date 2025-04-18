
#pragma once

#include "PAL_defines.h"

struct PAL_Texture;

/// @brief Font
struct PAL_Font;

/// @brief Font Glyph
struct PAL_Glyph
{
    /// @brief The x index of the glyph
    f32 index_x = 0.0f;

    /// @brief The y index of the glyph
    f32 index_y = 0.0f;

    /// @brief The x offset of the glyph
    f32 offset_x = 0.0f;
    
    /// @brief The y offset of the glyph
    f32 offset_y = 0.0f;

    /// @brief The width of the glyph
    f32 width = 0.0f;
    
    /// @brief The height of the glyph
    f32 height = 0.0f;

    /// @brief The x advance of the glyph
    u32 advance_x = 0;
    
    /// @brief The y advance of the glyph
    u32 advance_y = 0;
};

/**
 * @brief Load a font from disk. 
 * @attention A Context must be active.
 * @param file The filepath of the font.
 * @param size The size of the font.
 * @returns If successful, a pointer to the created font otherwise nullptr.
 */
PAL_API PAL_Font* PAL_LoadFont(const char* filepath, u32 size);

/**
 * @brief Destroy a font.
 * @attention A Context must be active.
 * @param font The font.
 */
PAL_API void PAL_DestroyFont(PAL_Font* font);

/**
 * @brief Get the size of the font.
 * @param font The font
 * @returns The size
 */
PAL_API u32 PAL_GetFontSize(PAL_Font* font);

/**
 * @brief Get the filepath of the font.
 * @param font The font
 * @returns The filepath
 */
PAL_API const char* PAL_GetFontPath(PAL_Font* font);

/**
 * @brief Get the baseline of the font.
 * @param font The font
 * @returns The baseline
 */
PAL_API f32 PAL_GetFontBaseLine(PAL_Font* font);

/**
 * @brief Get the texture atlas of the font.
 * @param font The font
 * @returns The texture
 */
PAL_API PAL_Texture* PAL_GetFontTexture(PAL_Font* font);

/**
 * @brief Get a glyph from the font.
 * @param font The font.
 * @param c The index of the glyph
 * @returns If successful, a pointer to the glyph otherwise nullptr.
 */
PAL_API PAL_Glyph* PAL_GetFontGlyph(PAL_Font* font, u8 c);

