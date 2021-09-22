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

enum atra_key_code {
    ATRA_KEY_NULL           = 0,    /* NULL, used for no key pressed */
    ATRA_KEY_SPACE          = 32,   /* Space */
    ATRA_KEY_APOSTROPHE     = 39,   /* ' */
    ATRA_KEY_COMMA          = 44,   /* , */
    ATRA_KEY_MINUS          = 45,   /* - */
    ATRA_KEY_PERIOD         = 46,   /* . */
    ATRA_KEY_SLASH          = 47,   /* / */
    ATRA_KEY_ZERO           = 48,   /* 0 */
    ATRA_KEY_ONE            = 49,   /* 1 */
    ATRA_KEY_TWO            = 50,   /* 2 */
    ATRA_KEY_THREE          = 51,   /* 3 */
    ATRA_KEY_FOUR           = 52,   /* 4 */
    ATRA_KEY_FIVE           = 53,   /* 5 */
    ATRA_KEY_SIX            = 54,   /* 6 */
    ATRA_KEY_SEVEN          = 55,   /* 7 */
    ATRA_KEY_EIGHT          = 56,   /* 8 */
    ATRA_KEY_NINE           = 57,   /* 9 */
    ATRA_KEY_SEMICOLON      = 59,   /* ; */
    ATRA_KEY_EQUAL          = 61,   /* = */
    ATRA_KEY_A              = 65,   /* A | a */
    ATRA_KEY_B              = 66,   /* B | b */
    ATRA_KEY_C              = 67,   /* C | c */
    ATRA_KEY_D              = 68,   /* D | d */
    ATRA_KEY_E              = 69,   /* E | e */
    ATRA_KEY_F              = 70,   /* F | f */
    ATRA_KEY_G              = 71,   /* G | g */
    ATRA_KEY_H              = 72,   /* H | h */
    ATRA_KEY_I              = 73,   /* I | i */
    ATRA_KEY_J              = 74,   /* J | j */
    ATRA_KEY_K              = 75,   /* K | k */
    ATRA_KEY_L              = 76,   /* L | l */
    ATRA_KEY_M              = 77,   /* M | m */
    ATRA_KEY_N              = 78,   /* N | n */
    ATRA_KEY_O              = 79,   /* O | o */
    ATRA_KEY_P              = 80,   /* P | p */
    ATRA_KEY_Q              = 81,   /* Q | q */
    ATRA_KEY_R              = 82,   /* R | r */
    ATRA_KEY_S              = 83,   /* S | s */
    ATRA_KEY_T              = 84,   /* T | t */
    ATRA_KEY_U              = 85,   /* U | u */
    ATRA_KEY_V              = 86,   /* V | v */
    ATRA_KEY_W              = 87,   /* W | w */
    ATRA_KEY_X              = 88,   /* X | x */
    ATRA_KEY_Y              = 89,   /* Y | y */
    ATRA_KEY_Z              = 90,   /* Z | z */
    ATRA_KEY_LEFT_BRACKET   = 91,   /* [ */
    ATRA_KEY_BACKSLASH      = 92,   /* '\' */
    ATRA_KEY_RIGHT_BRACKET  = 93,   /* ] */
    ATRA_KEY_GRAVE          = 96,   /* ` */
    ATRA_KEY_ESCAPE         = 256,  /* Esc */
    ATRA_KEY_ENTER          = 257,  /* Enter */
    ATRA_KEY_TAB            = 258,  /* Tab */
    ATRA_KEY_BACKSPACE      = 259,  /* Backspace */
    ATRA_KEY_INSERT         = 260,  /* Ins */
    ATRA_KEY_DELETE         = 261,  /* Del */
    ATRA_KEY_RIGHT          = 262,  /* Cursor right */
    ATRA_KEY_LEFT           = 263,  /* Cursor left */
    ATRA_KEY_DOWN           = 264,  /* Cursor down */
    ATRA_KEY_UP             = 265,  /* Cursor up */
    ATRA_KEY_PAGE_UP        = 266,  /* Page up */
    ATRA_KEY_PAGE_DOWN      = 267,  /* Page down */
    ATRA_KEY_HOME           = 268,  /* Home */
    ATRA_KEY_END            = 269,  /* End */
    ATRA_KEY_CAPS_LOCK      = 280,  /* Caps lock */
    ATRA_KEY_SCROLL_LOCK    = 281,  /* Scroll down */
    ATRA_KEY_NUM_LOCK       = 282,  /* Num lock */
    ATRA_KEY_PRINT_SCREEN   = 283,  /* Print screen */
    ATRA_KEY_PAUSE          = 284,  /* Pause */
    ATRA_KEY_F1             = 290,  /* F1 */
    ATRA_KEY_F2             = 291,  /* F2 */
    ATRA_KEY_F3             = 292,  /* F3 */
    ATRA_KEY_F4             = 293,  /* F4 */
    ATRA_KEY_F5             = 294,  /* F5 */
    ATRA_KEY_F6             = 295,  /* F6 */
    ATRA_KEY_F7             = 296,  /* F7 */
    ATRA_KEY_F8             = 297,  /* F8 */
    ATRA_KEY_F9             = 298,  /* F9 */
    ATRA_KEY_F10            = 299,  /* F10 */
    ATRA_KEY_F11            = 300,  /* F11 */
    ATRA_KEY_F12            = 301,  /* F12 */
    ATRA_KEY_KP_0           = 320,  /* Keypad 0 */
    ATRA_KEY_KP_1           = 321,  /* Keypad 1 */
    ATRA_KEY_KP_2           = 322,  /* Keypad 2 */
    ATRA_KEY_KP_3           = 323,  /* Keypad 3 */
    ATRA_KEY_KP_4           = 324,  /* Keypad 4 */
    ATRA_KEY_KP_5           = 325,  /* Keypad 5 */
    ATRA_KEY_KP_6           = 326,  /* Keypad 6 */
    ATRA_KEY_KP_7           = 327,  /* Keypad 7 */
    ATRA_KEY_KP_8           = 328,  /* Keypad 8 */
    ATRA_KEY_KP_9           = 329,  /* Keypad 9 */
    ATRA_KEY_KP_DECIMAL     = 330,  /* Keypad . */
    ATRA_KEY_KP_DIVIDE      = 331,  /* Keypad / */
    ATRA_KEY_KP_MULTIPLY    = 332,  /* Keypad * */
    ATRA_KEY_KP_SUBTRACT    = 333,  /* Keypad - */
    ATRA_KEY_KP_ADD         = 334,  /* Keypad + */
    ATRA_KEY_KP_ENTER       = 335,  /* Keypad Enter */
    ATRA_KEY_KP_EQUAL       = 336,  /* Keypad = */
    ATRA_KEY_LEFT_SHIFT     = 340,  /* Shift left */
    ATRA_KEY_LEFT_CONTROL   = 341,  /* Control left */
    ATRA_KEY_LEFT_ALT       = 342,  /* Alt left */
    ATRA_KEY_LEFT_SUPER     = 343,  /* Super left */
    ATRA_KEY_RIGHT_SHIFT    = 344,  /* Shift right */
    ATRA_KEY_RIGHT_CONTROL  = 345,  /* Control right */
    ATRA_KEY_RIGHT_ALT      = 346,  /* Alt right */
    ATRA_KEY_RIGHT_SUPER    = 347,  /* Super right */
    ATRA_KEY_KB_MENU        = 348   /* KB menu */
};

enum atra_mouse_button {
    ATRA_MOUSE_LEFT,                /* Left mouse button */
    ATRA_MOUSE_RIGHT,               /* Right mouse button */
    ATRA_MOUSE_MIDDLE,              /* Middle mouse button (mouse wheel) */
    ATRA_MOUSE_X1,                  /* Specialised extra mouse button */
    ATRA_MOUSE_X2                   /* Specialised extra mouse button */
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
ATRA_API void atra_window_setupviewport(int width, int height);

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
ATRA_API void atra_key_settextinput(int active);

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

#endif /* ATRA_CORE_H */
