/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  core.h - Window, OpenGL context and input management.
 *
 *  */

#ifndef ATRA_CORE_H
#define ATRA_CORE_H

#include "common.h"
#include "texture.h"

#ifdef __cplusplus
extern "C" {
#endif

enum atra_window_flag {
    ATRA_WINDOW_FULLSCREEN  = BIT(0),   /* Run program in fullscreen mode */
    ATRA_WINDOW_OPENGL      = BIT(1),   /* Window has an OpenGL context */
    ATRA_WINDOW_SHOWN       = BIT(2),   /* Window is visible */
    ATRA_WINDOW_HIDDEN      = BIT(3),   /* Window is not visible */
    ATRA_WINDOW_BORDERLESS  = BIT(4),   /* Borderless window mode */
    ATRA_WINDOW_RESIZABLE   = BIT(5),   /* Allow resizable window */
    ATRA_WINDOW_MINIMIZED   = BIT(6),   /* Minimized window */
    ATRA_WINDOW_MAXIMIZED   = BIT(7),   /* Maximized window */
    ATRA_WINDOW_MOUSE_GRAB  = BIT(8),   /* Window has grabbed mouse input */
    ATRA_WINDOW_INPUT_FOCUS = BIT(9),   /* Window has input focus */
    ATRA_WINDOW_MOUSE_FOCUS = BIT(10),  /* Window has mouse focus */
    ATRA_WINDOW_VSYNC       = BIT(11),  /* Attempt to enable v-sync */
    ATRA_WINDOW_HIGHDPI     = BIT(13),  /* Allow high-DPI mode */
};

enum atra_key_scancode {
    ATRA_KEY_NULL = 0,
    ATRA_KEY_A = 4,
    ATRA_KEY_B = 5,
    ATRA_KEY_C = 6,
    ATRA_KEY_D = 7,
    ATRA_KEY_E = 8,
    ATRA_KEY_F = 9,
    ATRA_KEY_G = 10,
    ATRA_KEY_H = 11,
    ATRA_KEY_I = 12,
    ATRA_KEY_J = 13,
    ATRA_KEY_K = 14,
    ATRA_KEY_L = 15,
    ATRA_KEY_M = 16,
    ATRA_KEY_N = 17,
    ATRA_KEY_O = 18,
    ATRA_KEY_P = 19,
    ATRA_KEY_Q = 20,
    ATRA_KEY_R = 21,
    ATRA_KEY_S = 22,
    ATRA_KEY_T = 23,
    ATRA_KEY_U = 24,
    ATRA_KEY_V = 25,
    ATRA_KEY_W = 26,
    ATRA_KEY_X = 27,
    ATRA_KEY_Y = 28,
    ATRA_KEY_Z = 29,
    ATRA_KEY_1 = 30,
    ATRA_KEY_2 = 31,
    ATRA_KEY_3 = 32,
    ATRA_KEY_4 = 33,
    ATRA_KEY_5 = 34,
    ATRA_KEY_6 = 35,
    ATRA_KEY_7 = 36,
    ATRA_KEY_8 = 37,
    ATRA_KEY_9 = 38,
    ATRA_KEY_0 = 39,
    ATRA_KEY_RETURN = 40,
    ATRA_KEY_ESCAPE = 41,
    ATRA_KEY_BACKSPACE = 42,
    ATRA_KEY_TAB = 43,
    ATRA_KEY_SPACE = 44,
    ATRA_KEY_MINUS = 45,
    ATRA_KEY_EQUALS = 46,
    ATRA_KEY_LEFTBRACKET = 47,
    ATRA_KEY_RIGHTBRACKET = 48,
    ATRA_KEY_BACKSLASH = 49,
    ATRA_KEY_NONUSHASH = 50,
    ATRA_KEY_SEMICOLON = 51,
    ATRA_KEY_APOSTROPHE = 52,
    ATRA_KEY_GRAVE = 53,
    ATRA_KEY_COMMA = 54,
    ATRA_KEY_PERIOD = 55,
    ATRA_KEY_SLASH = 56,
    ATRA_KEY_CAPSLOCK = 57,
    ATRA_KEY_F1 = 58,
    ATRA_KEY_F2 = 59,
    ATRA_KEY_F3 = 60,
    ATRA_KEY_F4 = 61,
    ATRA_KEY_F5 = 62,
    ATRA_KEY_F6 = 63,
    ATRA_KEY_F7 = 64,
    ATRA_KEY_F8 = 65,
    ATRA_KEY_F9 = 66,
    ATRA_KEY_F10 = 67,
    ATRA_KEY_F11 = 68,
    ATRA_KEY_F12 = 69,
    ATRA_KEY_PRINTSCREEN = 70,
    ATRA_KEY_SCROLLLOCK = 71,
    ATRA_KEY_PAUSE = 72,
    ATRA_KEY_INSERT = 73,
    ATRA_KEY_HOME = 74,
    ATRA_KEY_PAGEUP = 75,
    ATRA_KEY_DELETE = 76,
    ATRA_KEY_END = 77,
    ATRA_KEY_PAGEDOWN = 78,
    ATRA_KEY_RIGHT = 79,
    ATRA_KEY_LEFT = 80,
    ATRA_KEY_DOWN = 81,
    ATRA_KEY_UP = 82,
    ATRA_KEY_NUMLOCKCLEAR = 83,
    ATRA_KEY_KP_DIVIDE = 84,
    ATRA_KEY_KP_MULTIPLY = 85,
    ATRA_KEY_KP_MINUS = 86,
    ATRA_KEY_KP_PLUS = 87,
    ATRA_KEY_KP_ENTER = 88,
    ATRA_KEY_KP_1 = 89,
    ATRA_KEY_KP_2 = 90,
    ATRA_KEY_KP_3 = 91,
    ATRA_KEY_KP_4 = 92,
    ATRA_KEY_KP_5 = 93,
    ATRA_KEY_KP_6 = 94,
    ATRA_KEY_KP_7 = 95,
    ATRA_KEY_KP_8 = 96,
    ATRA_KEY_KP_9 = 97,
    ATRA_KEY_KP_0 = 98,
    ATRA_KEY_KP_PERIOD = 99,
    ATRA_KEY_NONUSBACKSLASH = 100,
    ATRA_KEY_APPLICATION = 101,
    ATRA_KEY_POWER = 102,
    ATRA_KEY_KP_EQUALS = 103,
    ATRA_KEY_F13 = 104,
    ATRA_KEY_F14 = 105,
    ATRA_KEY_F15 = 106,
    ATRA_KEY_F16 = 107,
    ATRA_KEY_F17 = 108,
    ATRA_KEY_F18 = 109,
    ATRA_KEY_F19 = 110,
    ATRA_KEY_F20 = 111,
    ATRA_KEY_F21 = 112,
    ATRA_KEY_F22 = 113,
    ATRA_KEY_F23 = 114,
    ATRA_KEY_F24 = 115,
    ATRA_KEY_EXECUTE = 116,
    ATRA_KEY_HELP = 117,
    ATRA_KEY_MENU = 118,
    ATRA_KEY_SELECT = 119,
    ATRA_KEY_STOP = 120,
    ATRA_KEY_AGAIN = 121,   /**< redo */
    ATRA_KEY_UNDO = 122,
    ATRA_KEY_CUT = 123,
    ATRA_KEY_COPY = 124,
    ATRA_KEY_PASTE = 125,
    ATRA_KEY_FIND = 126,
    ATRA_KEY_MUTE = 127,
    ATRA_KEY_VOLUMEUP = 128,
    ATRA_KEY_VOLUMEDOWN = 129,
    ATRA_KEY_KP_COMMA = 133,
    ATRA_KEY_KP_EQUALSAS400 = 134,
    ATRA_KEY_INTERNATIONAL1 = 135,
    ATRA_KEY_INTERNATIONAL2 = 136,
    ATRA_KEY_INTERNATIONAL3 = 137,
    ATRA_KEY_INTERNATIONAL4 = 138,
    ATRA_KEY_INTERNATIONAL5 = 139,
    ATRA_KEY_INTERNATIONAL6 = 140,
    ATRA_KEY_INTERNATIONAL7 = 141,
    ATRA_KEY_INTERNATIONAL8 = 142,
    ATRA_KEY_INTERNATIONAL9 = 143,
    ATRA_KEY_LANG1 = 144,
    ATRA_KEY_LANG2 = 145,
    ATRA_KEY_LANG3 = 146,
    ATRA_KEY_LANG4 = 147,
    ATRA_KEY_LANG5 = 148,
    ATRA_KEY_LANG6 = 149,
    ATRA_KEY_LANG7 = 150,
    ATRA_KEY_LANG8 = 151,
    ATRA_KEY_LANG9 = 152,
    ATRA_KEY_ALTERASE = 153,
    ATRA_KEY_SYSREQ = 154,
    ATRA_KEY_CANCEL = 155,
    ATRA_KEY_CLEAR = 156,
    ATRA_KEY_PRIOR = 157,
    ATRA_KEY_RETURN2 = 158,
    ATRA_KEY_SEPARATOR = 159,
    ATRA_KEY_OUT = 160,
    ATRA_KEY_OPER = 161,
    ATRA_KEY_CLEARAGAIN = 162,
    ATRA_KEY_CRSEL = 163,
    ATRA_KEY_EXSEL = 164,
    ATRA_KEY_KP_00 = 176,
    ATRA_KEY_KP_000 = 177,
    ATRA_KEY_THOUSANDSSEPARATOR = 178,
    ATRA_KEY_DECIMALSEPARATOR = 179,
    ATRA_KEY_CURRENCYUNIT = 180,
    ATRA_KEY_CURRENCYSUBUNIT = 181,
    ATRA_KEY_KP_LEFTPAREN = 182,
    ATRA_KEY_KP_RIGHTPAREN = 183,
    ATRA_KEY_KP_LEFTBRACE = 184,
    ATRA_KEY_KP_RIGHTBRACE = 185,
    ATRA_KEY_KP_TAB = 186,
    ATRA_KEY_KP_BACKSPACE = 187,
    ATRA_KEY_KP_A = 188,
    ATRA_KEY_KP_B = 189,
    ATRA_KEY_KP_C = 190,
    ATRA_KEY_KP_D = 191,
    ATRA_KEY_KP_E = 192,
    ATRA_KEY_KP_F = 193,
    ATRA_KEY_KP_XOR = 194,
    ATRA_KEY_KP_POWER = 195,
    ATRA_KEY_KP_PERCENT = 196,
    ATRA_KEY_KP_LESS = 197,
    ATRA_KEY_KP_GREATER = 198,
    ATRA_KEY_KP_AMPERSAND = 199,
    ATRA_KEY_KP_DBLAMPERSAND = 200,
    ATRA_KEY_KP_VERTICALBAR = 201,
    ATRA_KEY_KP_DBLVERTICALBAR = 202,
    ATRA_KEY_KP_COLON = 203,
    ATRA_KEY_KP_HASH = 204,
    ATRA_KEY_KP_SPACE = 205,
    ATRA_KEY_KP_AT = 206,
    ATRA_KEY_KP_EXCLAM = 207,
    ATRA_KEY_KP_MEMSTORE = 208,
    ATRA_KEY_KP_MEMRECALL = 209,
    ATRA_KEY_KP_MEMCLEAR = 210,
    ATRA_KEY_KP_MEMADD = 211,
    ATRA_KEY_KP_MEMSUBTRACT = 212,
    ATRA_KEY_KP_MEMMULTIPLY = 213,
    ATRA_KEY_KP_MEMDIVIDE = 214,
    ATRA_KEY_KP_PLUSMINUS = 215,
    ATRA_KEY_KP_CLEAR = 216,
    ATRA_KEY_KP_CLEARENTRY = 217,
    ATRA_KEY_KP_BINARY = 218,
    ATRA_KEY_KP_OCTAL = 219,
    ATRA_KEY_KP_DECIMAL = 220,
    ATRA_KEY_KP_HEXADECIMAL = 221,
    ATRA_KEY_LCTRL = 224,
    ATRA_KEY_LSHIFT = 225,
    ATRA_KEY_LALT = 226,
    ATRA_KEY_LGUI = 227,
    ATRA_KEY_RCTRL = 228,
    ATRA_KEY_RSHIFT = 229,
    ATRA_KEY_RALT = 230,
    ATRA_KEY_RGUI = 231,
    ATRA_KEY_MODE = 257,
    ATRA_KEY_AUDIONEXT = 258,
    ATRA_KEY_AUDIOPREV = 259,
    ATRA_KEY_AUDIOSTOP = 260,
    ATRA_KEY_AUDIOPLAY = 261,
    ATRA_KEY_AUDIOMUTE = 262,
    ATRA_KEY_MEDIASELECT = 263,
    ATRA_KEY_WWW = 264,
    ATRA_KEY_MAIL = 265,
    ATRA_KEY_CALCULATOR = 266,
    ATRA_KEY_COMPUTER = 267,
    ATRA_KEY_AC_SEARCH = 268,
    ATRA_KEY_AC_HOME = 269,
    ATRA_KEY_AC_BACK = 270,
    ATRA_KEY_AC_FORWARD = 271,
    ATRA_KEY_AC_STOP = 272,
    ATRA_KEY_AC_REFRESH = 273,
    ATRA_KEY_AC_BOOKMARKS = 274,
    ATRA_KEY_BRIGHTNESSDOWN = 275,
    ATRA_KEY_BRIGHTNESSUP = 276,
    ATRA_KEY_DISPLAYSWITCH = 277,
    ATRA_KEY_KBDILLUMTOGGLE = 278,
    ATRA_KEY_KBDILLUMDOWN = 279,
    ATRA_KEY_KBDILLUMUP = 280,
    ATRA_KEY_EJECT = 281,
    ATRA_KEY_SLEEP = 282,
    ATRA_KEY_APP1 = 283,
    ATRA_KEY_APP2 = 284,
    ATRA_KEY_AUDIOREWIND = 285,
    ATRA_KEY_AUDIOFASTFORWARD = 286,
    ATRA_KEY_TOTAL = 512
};

enum atra_mouse_button {
    ATRA_MOUSE_NONE,
    ATRA_MOUSE_LEFT,                /* Left mouse button */
    ATRA_MOUSE_RIGHT,               /* Right mouse button */
    ATRA_MOUSE_MIDDLE,              /* Middle mouse button (mouse wheel) */
    ATRA_MOUSE_X1,                  /* Specialised extra mouse button */
    ATRA_MOUSE_X2,                  /* Specialised extra mouse button */
    ATRA_MOUSE_TOTAL
};

enum atra_mouse_cursor {
    ATRA_CURSOR_ARROW,              /* Normal arrow cursor style */
    ATRA_CURSOR_IBEAM,              /* Text selection I-beam cursor style */
    ATRA_CURSOR_WAIT,               /* Wait cursor, indicating loading */
    ATRA_CURSOR_CROSSHAIR,          /* Crosshair cursor style */
    ATRA_CURSOR_WAITARROW,          /* Arrow with wait symbol combined */
    ATRA_CURSOR_RESIZE_TL_BR,       /* Falling slope diagonal resize cursor */
    ATRA_CURSOR_RESIZE_TR_BL,       /* Rising slope diagonal resize cursor */
    ATRA_CURSOR_RESIZE_LR,          /* Horizontal resize cursor */
    ATRA_CURSOR_RESIZE_TB,          /* Vertical resize cursor */
    ATRA_CURSOR_RESIZE_ALL,         /* Omni-directional resize cursor */
    ATRA_CURSOR_NOT_ALLOWED,        /* Slashed-circle to indicate invalidity */
    ATRA_CURSOR_POINTING_HAND,      /* Pointing hand (activate) style */
    ATRA_CURSOR_TOTAL
};

enum atra_gamepad_button {
    ATRA_GAMEPAD_NULL = -1,         /* No button */ 
    ATRA_GAMEPAD_UNKNOWN,           /* Unknown button */
    ATRA_GAMEPAD_LEFT_FACE_UP,      /* Gamepad left DPAD up button */
    ATRA_GAMEPAD_LEFT_FACE_RIGHT,   /* Gamepad left DPAD right button */
    ATRA_GAMEPAD_LEFT_FACE_DOWN,    /* Gamepad left DPAD down button */
    ATRA_GAMEPAD_LEFT_FACE_LEFT,    /* Gamepad left DPAD left button */
    ATRA_GAMEPAD_RIGHT_FACE_UP,     /* Gamepad right button up */
    ATRA_GAMEPAD_RIGHT_FACE_RIGHT,  /* Gamepad right button right */
    ATRA_GAMEPAD_RIGHT_FACE_DOWN,   /* Gamepad right button down */
    ATRA_GAMEPAD_RIGHT_FACE_LEFT,   /* Gamepad right button left */
    ATRA_GAMEPAD_LEFT_TRIGGER_1,    /* Gamepad top/back trigger left one */
    ATRA_GAMEPAD_LEFT_TRIGGER_2,    /* Gamepad top/back trigger left two */
    ATRA_GAMEPAD_RIGHT_TRIGGER_1,   /* Gamepad top/back trigger right one */
    ATRA_GAMEPAD_RIGHT_TRIGGER_2,   /* Gamepad top/back trigger right two */
    ATRA_GAMEPAD_MIDDLE_LEFT,       /* Gamepad center buttons, left one */
    ATRA_GAMEPAD_MIDDLE,            /* Gamepad center buttons, middle one */
    ATRA_GAMEPAD_MIDDLE_RIGHT,      /* Gamepad center buttons, right one */
    ATRA_GAMEPAD_LEFT_THUMB,        /* Gamepad joystick pressed button left */
    ATRA_GAMEPAD_RIGHT_THUMB        /* Gamepad joystick pressed button right */
};

enum atra_gamepad_analog {
    ATRA_GAMEPAD_AXIS_LEFT_X,           /* Gamepad left stick X axis */
    ATRA_GAMEPAD_AXIS_LEFT_Y,           /* Gamepad left stick Y axis */
    ATRA_GAMEPAD_AXIS_RIGHT_X,          /* Gamepad right stick X axis */
    ATRA_GAMEPAD_AXIS_RIGHT_Y,          /* Gamepad right stick Y axis */
    ATRA_GAMEPAD_AXIS_LEFT_TRIGGER,     /* Gamepad back trigger left, pressure level: [1..-1] */
    ATRA_GAMEPAD_AXIS_RIGHT_TRIGGER,    /* Gamepad back trigger right, pressure level: [1..-1] */
};

/* Top-level event categories (matches SDL's event enumeration) */
enum atra_event {
    ATRA_QUIT = 0x100,                  /* User-generated exit event */
    ATRA_APP_TERMINATING,               /* OS is closing the program */
    ATRA_APP_LOWMEMORY,                 /* OS is running out of memory */
    ATRA_APP_WILLENTERBACKGROUND,       /* Program is entering background */
    ATRA_APP_DIDENTERBACKGROUND,        /* Program is in the background */
    ATRA_APP_WILLENTERFOREGROUND,       /* Program is entering foreground */
    ATRA_APP_DIDENTERFOREGROUND,        /* Program is in the foreground */
    ATRA_WINDOWEVENT = 0x200,           /* Window state change */
    ATRA_KEYDOWN = 0x300,               /* Keyboard key pressed down */
    ATRA_KEYUP,                         /* Keyboard key released */
    ATRA_TEXTEDITING,                   /* Keyboard text editing */
    ATRA_TEXTINPUT,                     /* Keyboard text input */
    ATRA_KEYMAPCHANGED,                 /* Keyboard mapping has changed */
    ATRA_MOUSEMOTION = 0x400,           /* Mouse position has changed */
    ATRA_MOUSEBUTTONDOWN,               /* Mouse button pressed */
    ATRA_MOUSEBUTTONUP,                 /* Mouse button released */
    ATRA_MOUSEWHEEL,                    /* Mouse wheel moved */
    ATRA_CONTROLLERAXISMOTION = 0x650,  /* Controller joystick moved */
    ATRA_CONTROLLERBUTTONDOWN,          /* Controller button pressed */
    ATRA_CONTROLLERBUTTONUP,            /* Controller button released */
    ATRA_CONTROLLERDEVICEADDED,         /* Controller device connected */
    ATRA_CONTROLLERDEVICEREMOVED,       /* Controller device disconnected */
    ATRA_CONTROLLERDEVICEREMAPPED,      /* Controller mapping updated */
    ATRA_CLIPBOARDUPDATE = 0x900,       /* System clipboard has changed */
    ATRA_DROPFILE = 0x1000,             /* File request via drop on window */
    ATRA_DROPTEXT,                      /* Text request via drop on window */
    ATRA_DROPBEGIN,                     /* Set of drops is beginning */
    ATRA_DROPCOMPLETE                   /* Set of drops is complete */
};

/* Initialises all core subsystems */
ATRA_API void atra_engine_start(int width, int height, const char *title);

/* Shuts down all core subsystems */
ATRA_API void atra_engine_stop(void);

/* Updates the state of all engine subsystems.  Should be called every tick */
ATRA_API void atra_engine_update(void);

/*
 *  Window and OpenGL context management functions
 */

/* Returns 1 if the window should close */
ATRA_API int atra_window_shouldclose(void);

/* Closes the window and uninitializes the OpenGL 3.3 context */
ATRA_API void atra_window_close(void);

/* Returns 1 if the window is initialized */
ATRA_API int atra_window_isready(void);

/* Returns 1 if the window is in fullscreen mode */
ATRA_API int atra_window_isfullscreen(void);

/* Returns 1 if the window is currently hidden */
ATRA_API int atra_window_ishidden(void);

/* Returns 1 if the window is currently maximized */
ATRA_API int atra_window_ismaximized(void);

/* Returns 1 if the window is currently minimized */
ATRA_API int atra_window_isminimized(void);

/* Returns 1 if the window is currently focused */
ATRA_API int atra_window_isfocused(void);

/* Returns 1 if the window has been resized during the previous frame */
ATRA_API int atra_window_isresized(void);

/* Returns 1 if a specific window flag is set */
ATRA_API int atra_window_isflagset(u32 flag);

/* Sets the state of the window using flags */
ATRA_API void atra_window_setflags(u32 flags);

/* Clears all window state flags */
ATRA_API void atra_window_clearflags(u32 flags);

/* Toggles window fullscreen state */
ATRA_API void atra_window_togglefullscreen(void);

/* Maximizes the window */
ATRA_API void atra_window_maximize(void);

/* Minimizes the window */
ATRA_API void atra_window_minimize(void);

/* Restores the window to a non-minimized, non-maximized state */
ATRA_API void atra_window_restore(void);

/* Sets the window icon to the given image */
ATRA_API void atra_window_seticon(Image *image);

/* Sets the title text of the window */
ATRA_API void atra_window_settitle(const char *title);

/* Sets the position of the window */
ATRA_API void atra_window_setposition(int x, int y);

/* Sets the minimum allowed size of the window */
ATRA_API void atra_window_setminsize(int w, int h);

/* Sets the width and height of the window */
ATRA_API void atra_window_setsize(int w, int h);

/* Set viewport for a given width and height */
ATRA_API void atra_window_setupviewport(void);

/* Returns a pointer to the window handle */
ATRA_API void * atra_window_gethandle(void);

/* Returns the width of the screen */
ATRA_API int atra_window_getwidth(void);

/* Returns the height of the screen */
ATRA_API int atra_window_getheight(void);

/* Swaps the front and back buffer */
ATRA_API void atra_window_swapbuffers(void);

/* Returns a pointer to the scaling matrix of the window */
ATRA_API mat4 * atra_window_getscreenscalemat(void);

/* Returns the width of a specified monitor */
ATRA_API int atra_monitor_getwidth(void);

/* Returns the height of a specified monitor */
ATRA_API int atra_monitor_getheight(void);

/* Returns the refresh rate of a specified monitor */
ATRA_API int atra_monitor_getrefreshrate(void);

/* Sets x and y to the current position of the window */
ATRA_API void atra_window_getpos(int *x, int *y);

/* Sets the text in the operating system's clipboard */
ATRA_API void atra_clipboard_settext(const char *text);

/* Returns the text in the operating system's clipboard */
ATRA_API char * atra_clipboard_gettext(void);

/*
 *  Keyboard functions
 */

/* Returns 1 if key is currently up */
ATRA_API int atra_key_isup(i32 key);

/* Returns 1 if key is currently down */
ATRA_API int atra_key_isdown(i32 key);

/* Returns 1 if key has transitioned down in the last frame */
ATRA_API int atra_key_ispressed(i32 key);

/* Returns 1 if key has transitioned up in the last frame */
ATRA_API int atra_key_isreleased(i32 key);

/* Defines the given key as the key to exit the program (default = 0) */
ATRA_API void atra_key_setexit(i32 key);

/* Returns and pops the next key pressed from the input queue */
ATRA_API i32 atra_key_getpressed(void);

/* Returns the next text input event in the text queue (UTF-8) */
ATRA_API const char * atra_key_gettext(void);

/* Enables or disables text input mode */
ATRA_API void atra_key_settextinputmode(int active);

/* Returns 1 if text input mode is enabled */
ATRA_API int atra_key_textinputenabled(void);

/* Returns a pointer to the internal composition text */
ATRA_API const char * atra_key_getcomptext(void);

/* Returns the length of the composition text */
ATRA_API i32 atra_key_getcomplength(void);

/* Returns the position of the cursor in the composition text */
ATRA_API i32 atra_key_getcompcursorpos(void);

/*
 *  Gamepad functions
 */

/* Returns 1 if a gamepad is detected and available */
ATRA_API int atra_pad_isavailable(i32 gamepad);

/* Returns 1 if gamepad name matches the name provided */
ATRA_API int atra_pad_isnamed(i32 gamepad, const char *name);

/* Returns 1 if gamepad button is currently up */
ATRA_API int atra_pad_isup(i32 gamepad, i32 button);

/* Returns 1 if gamepad button is currently down */
ATRA_API int atra_pad_isdown(i32 gamepad, i32 button);

/* Returns 1 if gamepad button had transitioned down in the last frame */
ATRA_API int atra_pad_ispressed(i32 gamepad, i32 button);

/* Returns 1 if gamepad button had transitioned up in the last frame */
ATRA_API int atra_pad_isreleased(i32 gamepad, i32 button);

/* Returns and pops the next gamepad button pressed from the input queue */
ATRA_API i32 atra_pad_getpressed(void);

/* Returns the number of analog inputs on the gamepad */
ATRA_API i32 atra_pad_getanalogcount(void);

/* Returns the movement value for a specific analog input */
ATRA_API float atra_pad_getanalogmovement(i32 gamepad, i32 analog_input);

/* Sets the internal gamepad mappings */
ATRA_API int atra_pad_setmappings(const char *mappings);


/*
 *  Mouse functions
 */

/* Returns 1 if mouse button is currently up */
ATRA_API int atra_mouse_isup(i32 button);

/* Returns 1 if mouse button is currently down */
ATRA_API int atra_mouse_isdown(i32 button);

/* Returns 1 if mouse button had transitioned down in the last frame */
ATRA_API int atra_mouse_ispressed(i32 button);

/* Returns 1 if mouse button had transitioned up in the last frame */
ATRA_API int atra_mouse_isreleased(i32 button);

/* Returns 1 if mouse is locked */
ATRA_API int atra_mouse_islocked(void);

/* Sets x and y to window-space coordinates of the mouse cursor */
ATRA_API void atra_mouse_getpos(int *x, int *y);

/* Sets x and y to the relative change in mouse coordinates between frames */
ATRA_API void atra_mouse_getposchange(int *x, int *y);

/* Sets the mouse cursor position to (x, y) */
ATRA_API void atra_mouse_setpos(int x, int y);

/* Returns the mouse wheel movement value (+/- y) */
ATRA_API float atra_mouse_getwheelmovement(void);

/* Sets the mouse cursor display style */
ATRA_API void atra_mouse_setcursor(i32 cursor);

/* Sets the mouse cursor image to use when in a specific cursor style */
ATRA_API void atra_mouse_setcursorimage(i32 cursor, Image *image, int hot_x, int hot_y);

/* Hides the mouse cursor and locks it to the centre of the screen */
ATRA_API void atra_mouse_lock(void);

/* Shows the mouse cursor, freeing it from its locked state */
ATRA_API void atra_mouse_unlock(void);

/*
 *  Time related functions
 */

/* Returns the number of milliseconds since Atra Engine's initialization */
ATRA_API u32 atra_time_getcurrent(void);

/* Returns the change in time since the last time this function was called
 * Returns the same as atra_time_getcurrent() on first call */
ATRA_API u32 atra_time_getdelta(void);

/* Returns the number of milliseconds elapsed during the update phase */
ATRA_API u32 atra_time_getupdatetime(void);

/* Returns the number of milliseconds elapsed during the render phase */
ATRA_API u32 atra_time_getrendertime(void);

/* Returns the number of milliseconds elapsed in drawing a single frame */
ATRA_API u32 atra_time_getframetime(void);

/* Returns the total number of frames drawn */
ATRA_API u32 atra_time_getframecount(void);

#ifdef __cplusplus
}
#endif

#endif /* ATRA_CORE_H */
