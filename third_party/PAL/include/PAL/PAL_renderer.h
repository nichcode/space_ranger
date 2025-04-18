
#pragma once

#include "PAL_defines.h"
#include "PAL_viewport.h"
#include "PAL_color.h"
#include "PAL_rect.h"

/// @brief Renderer
struct PAL_Renderer;

struct PAL_Context;
struct PAL_Texture;
struct PAL_Font;

/// @brief Renderflips
enum PAL_Flips_
{
    PAL_Flips_None = 0,
    PAL_Flips_Horizontal = PAL_BIT(0),
    PAL_Flips_Vertical = PAL_BIT(1)
};

/// @brief Rotation anchors
enum PAL_Anchors_
{
    PAL_Anchors_TopLeft,
    PAL_Anchors_Center
};

/// @brief camera
struct PAL_Camera
{
    PAL_Viewport view;    
    f32 rotation = 0.0f;
};

/**
 * @brief Create a renderer. 
 * @attention A Context must be active.
 * @returns If successful, a pointer to the created renderer otherwise nullptr.
 */
PAL_API PAL_Renderer* PAL_CreateRenderer();

/**
 * @brief Destroy a renderer.
 * @attention A Context must be active.
 * @param renderer The renderer
 */
PAL_API void PAL_DestroyRenderer(PAL_Renderer* renderer);

/**
 * @brief Draw a rect using the renderer.
 * @param renderer The renderer
 * @param rect The rect
 * @param color The color
 */
PAL_API void PAL_RendererDrawRect(PAL_Renderer* renderer, const PAL_Rect rect, const PAL_Color color);

/**
 * @brief Draw a rect using the renderer.
 * @param renderer The renderer
 * @param rect The rect
 * @param rotation The angle in degrees
 * @param anchor The rotation anchor
 * @param color The color
 */
PAL_API void PAL_RendererDrawRectEx(PAL_Renderer* renderer, const PAL_Rect rect, f32 rotation, u32 anchor, const PAL_Color color);

/**
 * @brief Draw a texture using the renderer.
 * @param renderer The renderer
 * @param rect The rect
 * @param texture The texture
 */
PAL_API void PAL_RendererDrawTexture(PAL_Renderer* renderer, const PAL_Rect rect, PAL_Texture* texture);

/**
 * @brief Draw a texture using the renderer.
 * @param renderer The renderer
 * @param rect The rect
 * @param rotation The angle in degrees
 * @param anchor The rotation anchor
 * @param texture The texture
 * @param tint_color The tint_color
 * @param flip The render flip
 */
PAL_API void PAL_RendererDrawTextureEx(PAL_Renderer* renderer, const PAL_Rect rect, f32 rotation,
                                    u32 anchor, PAL_Texture* texture, const PAL_Color tint_color, u32 flip);


/**
 * @brief Draw a text using the renderer.
 * @param renderer The renderer
 * @param x The position x
 * @param y The position y
 * @param text The text
 * @param font The font
 * @param color The color
 */
PAL_API void PAL_RendererDrawText(PAL_Renderer* renderer, f32 x, f32 y, f32 scale, const char* text, PAL_Font* font, const PAL_Color color);

/**
 * @brief Push the command queue and update the renderer
 * @param renderer The renderer
 */
PAL_API void PAL_RendererFlush(PAL_Renderer* renderer);

/**
 * @brief Set the camera of the renderer
 * @param renderer The renderer
 * @param camera The camera
 */
PAL_API void PAL_SetRendererCamera(PAL_Renderer* renderer, PAL_Camera camera);