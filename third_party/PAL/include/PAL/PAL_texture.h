
#pragma once

#include "PAL_rect.h"

/// @brief Texture
struct PAL_Texture;

/// @brief Texture flags
enum PAL_TextureFlags_
{
    PAL_TextureFlags_None,
    PAL_TextureFlags_Target,
    PAL_TextureFlags_Storage
};

/// @brief Texture formats
enum PAL_TextureFormats_
{
    PAL_TextureFormats_R8,
    PAL_TextureFormats_RGB8,
    PAL_TextureFormats_RGB16,
    PAL_TextureFormats_RGB16F,
    PAL_TextureFormats_RGB32F,
    PAL_TextureFormats_RGBA8,
    PAL_TextureFormats_RGBA16,
    PAL_TextureFormats_RGBA16F,
    PAL_TextureFormats_RGBA32F
};

/// @brief Texture descriptor
struct PAL_TextureDesc
{
    /// @brief texure flags. Choose from PAL_TextureFlags_
    u32 flag = 0;

    /// @brief texure format. Choose from PAL_TextureFormats_
    u32 format = 0;

    /// @brief width of the texture.
    u32 width = 1;

    /// @brief width of the texture.
    u32 height = 1;
    
    /// @brief texure storage data. This is for storage textures.
    void* storage_data = nullptr;
};

/**
 * @brief Create a texture with a texture descriptor struct.
 * @attention A Context must be active.
 * @param desc The texture descriptor
 * @returns If successful, a pointer to the created texture otherwise nullptr.
 */
PAL_API PAL_Texture* PAL_CreateTexture(PAL_TextureDesc desc);

/**
 * @brief Load a texture from disk.
 * @attention A Context must be active.
 * @param filepath The filepath
 * @returns If successful, a pointer to the created texture otherwise nullptr.
 */
PAL_API PAL_Texture* PAL_LoadTexture(const char* filepath);

/**
 * @brief Destroy a texture.
 * @attention A Context must be active.
 * @param texture The texture
 */
PAL_API void PAL_DestroyTexture(PAL_Texture* texture);

/**
 * @brief Make the texture active to the texture slot
 * @param texture The texture
 * @param slot The tetxure slot
 */
PAL_API void PAL_BindTexture(PAL_Texture* texture, u32 slot);

/**
 * @brief Make the texture the active render target
 * @attention the texture must have PAL_TextureFlags_Target flag.
 * @param texture The texture
 */
PAL_API void PAL_BindTarget(PAL_Texture* texture);

/**
 * @brief Release the render target
 * @attention the texture must have PAL_TextureFlags_Target flag.
 * @param texture The texture
 *
 */
PAL_API void PAL_UnbindTarget(PAL_Texture* texture);

/**
 * @brief Get the width of the texture.
 * @param texture The texture
 * @returns the width
 */
PAL_API u32 PAL_GetTextureWidth(PAL_Texture* texture);

/**
 * @brief Get the height of the texture.
 * @param texture The texture
 * @returns the height
 */
PAL_API u32 PAL_GetTextureHeight(PAL_Texture* texture);

/**
 * @brief Get the filepath of the texture.
 * @param texture The texture
 * @returns the filepath
 */
PAL_API const char* PAL_GetTexturePath(PAL_Texture* texture);

/**
 * @brief Get the rect of the texture.
 * @param texture The texture
 * @returns the rect
 */
PAL_API PAL_Rect PAL_GetTextureRect(PAL_Texture* texture);