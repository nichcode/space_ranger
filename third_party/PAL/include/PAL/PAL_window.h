
#pragma once

#include "PAL_defines.h"

/// @brief Window
struct PAL_Window;

/// @brief Window close callback
using PAL_WindowCloseFunc = void(*)(PAL_Window* window);

/// @brief Window key callback
using PAL_WindowKeyFunc = void(*)(PAL_Window* window, u32 key, i32 scancode, u32 action);

/// @brief Window button callback
using PAL_WindowButtonFunc = void(*)(PAL_Window* window, u32 button, u32 action);

/// @brief Window mouse moved callback
using PAL_WindowMouseMovedFunc = void(*)(PAL_Window* window, i32 x, i32 y);

/// @brief Window mouse scrolled callback
using PAL_WindowMouseScrolledFunc = void(*)(PAL_Window* window, f32 offset_x, f32 offset_y);

/// @brief Window moved callback
using PAL_WindowMovedFunc = void(*)(PAL_Window* window, i32 x, i32 y);

/// @brief Window resized callback
using PAL_WindowResizeFunc = void(*)(PAL_Window* window, u32 width, u32 height);

/// @brief Window focused callback
using PAL_WindowFocusedFunc = void(*)(PAL_Window* window, b8 focused);

/// @brief Window flags
enum PAL_WindowFlags_
{
    PAL_WindowFlags_Show = PAL_BIT(0),
    PAL_WindowFlags_Center = PAL_BIT(1)
};

/**
 * @brief Create a window.
 * @param title The title of the window
 * @param width The width of the window
 * @param height The height of the window
 * @param flags The window flags
 * @returns If successful, a pointer to the created window otherwise nullptr.
 */
PAL_API PAL_Window* PAL_CreateWindow(const char* title, u32 width, u32 height, u32 flags);

/**
 * @brief Destroy the window.
 * @param window The window
 */
PAL_API void PAL_DestroyWindow(PAL_Window* window);

/**
 * @brief Pull events for all created windows.
 */
PAL_API void PAL_PullEvents();

/**
 * @brief Hide window
 * @param window The window
 */
PAL_API void PAL_HideWindow(PAL_Window* window);

/**
 * @brief Show window
 * @param window The window
 */
PAL_API void PAL_ShowWindow(PAL_Window* window);

/**
 * @brief Set the title of the window
 * @param window The window
 * @param title The title
 */
PAL_API void PAL_SetWindowTitle(PAL_Window* window, const char* title);

/**
 * @brief Set the size of the window
 * @param window The window
 * @param width The width
 * @param height The height
 */
PAL_API void PAL_SetWindowSize(PAL_Window* window, u32 width, u32 height);

/**
 * @brief Set the position of the window
 * @param window The window
 * @param x The x position
 * @param y The y position
 */
PAL_API void PAL_SetWindowPos(PAL_Window* window, i32 x, i32 y);

/**
 * @brief Set the window close callback
 * @param func The callback
 */
PAL_API void PAL_SetWindowCloseCallback(PAL_WindowCloseFunc func);

/**
 * @brief Set the window key callback
 * @param func The callback
 */
PAL_API void PAL_SetWindowKeyCallback(PAL_WindowKeyFunc func);

/**
 * @brief Set the window button callback
 * @param func The callback
 */
PAL_API void PAL_SetWindowButtonCallback(PAL_WindowButtonFunc func);

/**
 * @brief Set the window mouse moved callback
 * @param func The callback
 */
PAL_API void PAL_SetWindowMouseMovedCallback(PAL_WindowMouseMovedFunc func);

/**
 * @brief Set the window mouse scrolled callback
 * @param func The callback
 */
PAL_API void PAL_SetWindowMouseScrolledCallback(PAL_WindowMouseScrolledFunc func);

/**
 * @brief Set the window moved callback
 * @param func The callback
 */
PAL_API void PAL_SetWindowMovedCallback(PAL_WindowMovedFunc func);

/**
 * @brief Set the window resized callback
 * @param func The callback
 */
PAL_API void PAL_SetWindowResizedCallback(PAL_WindowResizeFunc func);

/**
 * @brief Set the window focused callback
 * @param func The callback
 */
PAL_API void PAL_SetWindowFocusedCallback(PAL_WindowFocusedFunc func);

/**
 * @brief Reset all window callbacks
 */
PAL_API void PAL_WindowResetCallbacks();

/**
 * @brief Check if window close button has been clicked.
 * @param window The window
 * @returns True if clicked otherwise false
 */
PAL_API b8 PAL_WindowShouldClose(PAL_Window* window);

/**
 * @brief Get the state of the key.
 * @param window The window
 * @param key The key
 * @returns True if pressed otherwise false
 */
PAL_API b8 PAL_GetKeyState(PAL_Window* window, u32 key);

/**
 * @brief Get the state of the button.
 * @param window The window
 * @param button The button
 * @returns True if pressed otherwise false
 */
PAL_API b8 PAL_GetButtonState(PAL_Window* window, u32 button);

/**
 * @brief Get the title of the window.
 * @param window The window
 * @returns the title
 */
PAL_API const char* PAL_GetWindowTitle(PAL_Window* window);

/**
 * @brief Get the handle of the window.
 * @param window The window
 * @returns the handle
 */
PAL_API void* PAL_GetWindowHandle(PAL_Window* window);

/**
 * @brief Get the width of the window.
 * @param window The window
 * @returns the width
 */
PAL_API u32 PAL_GetWindowWidth(PAL_Window* window);

/**
 * @brief Get the height of the window.
 * @param window The window
 * @returns the height
 */
PAL_API u32 PAL_GetWindowHeight(PAL_Window* window);

/**
 * @brief Get the x position of the window.
 * @param window The window
 * @returns the x position
 */
PAL_API i32 PAL_GetWindowPosX(PAL_Window* window);

/**
 * @brief Get the y position of the window.
 * @param window The window
 * @returns the y position
 */
PAL_API i32 PAL_GetWindowPosY(PAL_Window* window);