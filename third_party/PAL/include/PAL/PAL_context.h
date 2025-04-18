
#pragma once

#include "PAL_viewport.h"
#include "PAL_color.h"

struct PAL_Window;

/// @brief Context
struct PAL_Context;

/// @brief Context types
enum PAL_ContextTypes_
{
    PAL_ContextTypes_OpenGL
};

/// @brief Draw mode
enum PAL_DrawModes_
{
    PAL_DrawModes_Triangles
};

/// @brief Blend modes
enum PAL_BlendModes_
{
    PAL_BlendModes_None,
    PAL_BlendModes_Alpha
};

/// @brief Context descriptor
struct PAL_ContextDesc
{
    /// @brief The type of the context.
    u32 type;

    /// @brief The major version of the context.
    u32 major;

    /// @brief The minor version of the context.
    u32 minor;
};

/**
 * @brief Create a context.
 * @param window The window to create the context for.
 * @param desc The context descriptor.
 * @returns If successful, a pointer to the created context otherwise nullptr.
 */
PAL_API PAL_Context* PAL_CreateContext(PAL_Window* window, PAL_ContextDesc desc);

/**
 * @brief Destroy a context. 
 * @attention This destroys all objects created when the context was active. 
 * @param context The context.
 */
PAL_API void PAL_DestroyContext(PAL_Context* context);

/**
 * @brief Update the active context. 
 */
PAL_API void PAL_SwapBuffers();

/**
 * @brief Clear the active context with the specified color. 
 * @param color The clear color.
 */
PAL_API void PAL_Clear(const PAL_Color color);

/**
 * @brief Make the context active. 
 * @param context The context
 */
PAL_API void PAL_MakeActive(PAL_Context* context);

/**
 * @brief Set vsync for the active context.
 * @param vsync True to turn on vsync, otherwise false to turn off vsync
 */
PAL_API void PAL_SetVsync(b8 vsync);

/**
 * @brief Set the viewport for the active context.
 * @param viewport The viewport.
 */
PAL_API void PAL_SetViewport(PAL_Viewport viewport);

/**
 * @brief Set blend mode for the active context.
 * @param blend_mode The blend_mode.
 */
PAL_API void PAL_SetBlendMode(u32 blend_mode);

/**
 * @brief Draw arrays using the active context.
 * @param mode The Rendering mode.
 * @param count The count.
 */
PAL_API void PAL_DrawArrays(u32 mode, u32 count);

/**
 * @brief Draw elements using the active context.
 * @param mode The Rendering mode.
 * @param count The count.
 */
PAL_API void PAL_DrawElements(u32 mode, u32 count);

/**
 * @brief Draw elements instanced using the active context.
 * @param mode The Rendering mode.
 * @param count The count.
 * @param instance_count The instanced count.
 */
PAL_API void PAL_DrawElementsInstanced(u32 mode, u32 count, u32 instance_count);

/**
 * @brief Draw arrays instanced using the active context.
 * @param mode The Rendering mode.
 * @param count The count.
 * @param instance_count The instanced count.
 */
PAL_API void PAL_DrawArraysInstanced(u32 mode, u32 count, u32 instance_count);

/**
 * @brief Get the viewport of the active context.
 * @returns the viewport.
 */
PAL_API PAL_Viewport PAL_GetViewport();
