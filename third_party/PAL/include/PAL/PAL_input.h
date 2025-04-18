
#pragma once

#include "PAL_defines.h"

/// @brief Buttons
enum PAL_Buttons_
{
    PAL_Buttons_Left,
    PAL_Buttons_Right,
    PAL_Buttons_Middle,

    PAL_Buttons_Max
};

/// @brief Keys
enum PAL_Keys_
{
    PAL_Keys_A,
    PAL_Keys_B,
    PAL_Keys_C,
    PAL_Keys_D,
    PAL_Keys_E,
    PAL_Keys_F,
    PAL_Keys_G,
    PAL_Keys_H,
    PAL_Keys_I,
    PAL_Keys_J,
    PAL_Keys_K,
    PAL_Keys_L,
    PAL_Keys_M,
    PAL_Keys_N,
    PAL_Keys_O,
    PAL_Keys_P,
    PAL_Keys_Q,
    PAL_Keys_R,
    PAL_Keys_S,
    PAL_Keys_T,
    PAL_Keys_U,
    PAL_Keys_V,
    PAL_Keys_W,
    PAL_Keys_X,
    PAL_Keys_Y,
    PAL_Keys_Z,

    PAL_Keys_Escape,
    PAL_Keys_Enter,
    PAL_Keys_Tab,
    PAL_Keys_Backspace,
    PAL_Keys_Insert,
    PAL_Keys_Delete,
    PAL_Keys_Right,
    PAL_Keys_Left,
    PAL_Keys_Down,
    PAL_Keys_Up,
    PAL_Keys_PageUp,
    PAL_Keys_PageDown,
    PAL_Keys_Home,
    PAL_Keys_End,
    PAL_Keys_CapsLock,
    PAL_Keys_ScrollLock,
    PAL_Keys_NumLock,
    PAL_Keys_PAL_intScreen,
    PAL_Keys_Pause,
    PAL_Keys_F1,
    PAL_Keys_F2,
    PAL_Keys_F3,
    PAL_Keys_F4,
    PAL_Keys_F5,
    PAL_Keys_F6,
    PAL_Keys_F7,
    PAL_Keys_F8,
    PAL_Keys_F9,
    PAL_Keys_F10,
    PAL_Keys_F11,
    PAL_Keys_F12,

    PAL_Keys_LeftShift,
    PAL_Keys_LeftControl,
    PAL_Keys_LeftAlt,
    PAL_Keys_LeftSuper,
    PAL_Keys_RightShift,
    PAL_Keys_RightControl,
    PAL_Keys_RightAlt,
    PAL_Keys_RightSuper,
    PAL_Keys_Menu,

    PAL_Keys_LeftBracket, /* [ */
    PAL_Keys_Backslash, /* \ */
    PAL_Keys_RightBracket,  /* ] */
    PAL_Keys_GraveAccent,  /* ` */
    PAL_Keys_SemiColon, /* ; */
    PAL_Keys_Equal, /* = */

    PAL_Keys_Space,
    PAL_Keys_Apostrophe, /* ' */
    PAL_Keys_Comma, /* , */
    PAL_Keys_Minus, /* - */
    PAL_Keys_Period, /* . */
    PAL_Keys_Slash, /* / */

    PAL_Keys_0, /* 0 */
    PAL_Keys_1, /* 1 */
    PAL_Keys_2, /* 2 */
    PAL_Keys_3, /* 3 */
    PAL_Keys_4, /* 4 */
    PAL_Keys_5, /* 5 */
    PAL_Keys_6, /* 6 */
    PAL_Keys_7, /* 7 */
    PAL_Keys_8, /* 8 */
    PAL_Keys_9, /* 9 */

    PAL_Keys_P0,
    PAL_Keys_P1,
    PAL_Keys_P2,
    PAL_Keys_P3,
    PAL_Keys_P4,
    PAL_Keys_P5,
    PAL_Keys_P6,
    PAL_Keys_P7,
    PAL_Keys_P8,
    PAL_Keys_P9,
    PAL_Keys_PDecimal,
    PAL_Keys_PDivide,
    PAL_Keys_PMultiply,
    PAL_Keys_PSubtract,
    PAL_Keys_PAdd,
    PAL_Keys_PEnter,
    PAL_Keys_PEqual,

    PAL_Keys_Max
};

/// @brief Actions
enum PAL_Actions_
{
    PAL_Actions_Release,
    PAL_Actions_Press,
    PAL_Actions_Repeat,

    PAL_Actions_Max
};

/**
 * @brief Get the name of the key
 * @param key The key
 * @returns The name
 */
PAL_API const char* PAL_GetKeyName(u32 key);

/**
 * @brief Get the name of the button
 * @param button The button
 * @returns The name
 */
PAL_API const char* PAL_GetButtonName(u32 button);

/**
 * @brief Get the name of the action
 * @param action The action
 * @returns The name
 */
PAL_API const char* PAL_GetActionName(u32 action);