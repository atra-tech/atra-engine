/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  core.c - Window, OpenGL context and input management.
 *
 *  */

#include "atra.h"

#define DEFAULT_SCREEN_WIDTH    1280
#define DEFAULT_SCREEN_HEIGHT   720
#define DEFAULT_REFRESH_RATE    60

#define KEY_MAXKEYS         512     /* Maximum keyboard keys */
#define MOUSE_MAXBUTTONS    8       /* Maximum mouse buttons */
#define PAD_MAX             4       /* Maximum gamepad count */
#define PAD_MAXANALOG       8       /* Maximum gamepad analog inputs */
#define PAD_MAXBUTTONS      32      /* Maximum gamepad buttons */
#define EVENT_MAXQUEUE      32      /* Maximum input event queue size */
#define TEXT_INPUT_LEN      32      /* Maxumum characters in text input */

typedef SDL_Event Event;
typedef SDL_Cursor Cursor;
typedef SDL_GameController GamePad;

typedef struct System {
    int isterminating;
    int low_mem;
    int isbackground;
} System;

typedef struct Window {
    SDL_Window *window;
    SDL_GLContext context;
    const char *title;
    u32 flags;
    int ready;
    int fullscreen;
    int should_close;
    int resized;
    int refresh_rate;
    mat4 screen_scale;

    struct { int x, y; } pos;
    struct { int w, h; } monitor;
    struct { int w, h; } screen;
    struct { int w, h; } fbo;
    struct { int w, h; } render;
    struct { int x, y; } render_offset;
} Window;

/* Large struct for tracking all input states and configurations */
typedef struct Input {
    struct {
        i32 exit_key;                       /* Key that terminates program */
        u8 keystate_curr[KEY_MAXKEYS];      /* State of all keys this frame */
        u8 keystate_prev[KEY_MAXKEYS];      /* State of all keys last frame */
        i32 keyqueue[EVENT_MAXQUEUE];       /* Keyboard input queue */
        int keyqueue_size;                  /* Pending keyboard event count */
        char textqueue[EVENT_MAXQUEUE][32]; /* Text input event queue */
        int textqueue_size;                 /* Pending text input events */

        /* Text input mode variables */
        char *comp_text;                    /* Editing text (UTF-8) */
        i32 comp_length;                    /* Number of characters in comp */
        i32 cursor_pos;                     /* Location to begin editing from */
    } key;

    struct {
        ivec2 offset;                       /* Mouse position offset */
        ivec2 pos_curr;                     /* Mouse position this frame */
        ivec2 pos_prev;                     /* Mouse position last frame */
        vec2 scale;                         /* Mouse scaling */

        Cursor *cursor;                     /* SDL system cursor handle */
        int cursor_style;                   /* Current mouse cursor type */
        int cursor_hidden;                  /* True when mouse is hidden */
        int cursor_locked;                  /* True when the mouse is locked */
        int cursor_inwindow;                /* True when mouse is in window */

        int buttons_curr[MOUSE_MAXBUTTONS]; /* Mouse button state this frame */
        int buttons_prev[MOUSE_MAXBUTTONS]; /* Mouse button state last frame */
        float wheelmove_curr;               /* Mouse wheel change this frame */
        float wheelmove_prev;               /* Mouse wheel change last frame */
        i32 eventqueue[EVENT_MAXQUEUE];     /* Pending mouse input events */
        int eventqueue_size;
    } mouse;

    struct {
        GamePad *pads[PAD_MAX];             /* SDL gamepad handles */
        int padcount;                       /* Number of detected gamepads */
        i32 button_prev;                    /* Last gamepad button pressed */
        int analog_count;                   /* Number of analog inputs */
        int hashaptic;                      /* Gamepads have haptic feedback */
        u32 haptic_prop;                    /* Haptic feedback settings */
        int ready[PAD_MAX];                 /* Gamepad ready for processing */

        i8 buttonstate_curr[PAD_MAX][PAD_MAXBUTTONS];   /* Gamepad button states this frame */
        i8 buttonstate_prev[PAD_MAX][PAD_MAXBUTTONS];   /* Gamepad button states last frame */
        float analogstate[PAD_MAX][PAD_MAXANALOG];      /* Gamepad analog input states */

        i32 eventqueue[EVENT_MAXQUEUE];      /* Gamepad event queue */
        int eventqueue_size;
    } pad;
} Input;

/* Time recording type for tracking game-related time statistics */
typedef struct Time {
    u32 current;        /* Current time */
    u32 prev;           /* Previous time */
    u32 update;         /* Time elapsed during update phase */
    u32 render;         /* Time elapsed during render phase */
    u32 frame;          /* Time to draw one frame */
    u32 target;         /* Desired time for one frame, 0 disables */
    u32 frame_count;    /* Number of frames drawn */
} Time;

const static char *event_names[] = {
    "ATRA_QUIT",
    "ATRA_APP_TERMINATING",
    "ATRA_APP_LOWMEMORY",
    "ATRA_APP_WILLENTERBACKGROUND",
    "ATRA_APP_DIDENTERBACKGROUND",
    "ATRA_APP_WILLENTERFOREGROUND",
    "ATRA_APP_DIDENTERFOREGROUND",
    "ATRA_WINDOWEVENT",
    "ATRA_KEYDOWN",
    "ATRA_KEYUP",
    "ATRA_TEXTEDITING",
    "ATRA_TEXTINPUT",
    "ATRA_KEYMAPCHANGED",
    "ATRA_MOUSEMOTION",
    "ATRA_MOUSEBUTTONDOWN",
    "ATRA_MOUSEBUTTONUP",
    "ATRA_MOUSEWHEEL",
    "ATRA_CONTROLLERAXISMOTION",
    "ATRA_CONTROLLERBUTTONDOWN",
    "ATRA_CONTROLLERBUTTONUP",
    "ATRA_CONTROLLERDEVICEADDED",
    "ATRA_CONTROLLERDEVICEREMOVED",
    "ATRA_CONTROLLERDEVICEREMAPPED",
    "ATRA_CLIPBOARDUPDATE",
    "ATRA_DROPFILE",
    "ATRA_DROPTEXT",
    "ATRA_DROPBEGIN",
    "ATRA_DROPCOMPLETE"
};

//static System sys = {0};
static Window window = {0};
static Input input = {0};
static Time time_data = {0};

ATRA_INLINE void atra_window_init(int width, int height, const char *title);
ATRA_INLINE void atra_context_init(int width, int height);
ATRA_INLINE void atra_input_init(void);
ATRA_INLINE void atra_mouse_init(void);
ATRA_INLINE void atra_pad_init(void);
ATRA_INLINE void atra_time_init(void);

ATRA_INLINE void atra_input_update(void);
ATRA_INLINE void atra_mouse_update(Event *e);
ATRA_INLINE void atra_pad_update(Event *e);
ATRA_INLINE void atra_key_update(Event *e);
ATRA_INLINE void atra_time_update(void);

ATRA_INLINE void atra_key_pushkeyevent(i32 key);
ATRA_INLINE void atra_key_pushtextevent(const char[32]);

/* Initialises all core subsystems, creating a window in the process */
ATRA_API
void
atra_engine_start(int width, int height, const char *title)
{
    atra_log_start(512, NULL);
    atra_window_init(width, height, title);
    atra_input_init();
    atra_time_init();
    atra_mempool_init(32 * 1024 * 1024);
    atra_gl_init(width, height);
}

ATRA_API
void
atra_engine_update(void)
{
    atra_input_update();
    atra_time_update();
}

/* Shuts down all core subsystems */
ATRA_API
void
atra_engine_stop(void)
{
    atra_log_print(LOG_INFO, "Stopping Atra Engine");

    atra_state_stop();
    atra_gl_stop();
    atra_log_stop();
    atra_mempool_stop();

    SDL_DestroyWindow(window.window);
    SDL_GL_DeleteContext(window.context);
    IMG_Quit();
    SDL_Quit();
}

/*
 *  Window and OpenGL context management functions
 */

/* Initializes the program window and OpenGL 3.3 context */
ATRA_INLINE
void
atra_window_init(int width, int height, const char *title)
{
    atra_log_print(LOG_INFO, "Initializing Atra Engine %s", ATRA_VERSION);

    if (title) {
        atra_window_settitle(title);
        window.title = atra_util_strdup(title);
    } else {
        atra_window_settitle("Atra Engine");
        window.title = atra_util_strdup("Atra Engine");
    }
    input.mouse.scale[0] = 1.0f;
    input.mouse.scale[1] = 1.0f;
    input.mouse.cursor_style = ATRA_CURSOR_ARROW;
    input.pad.button_prev = ATRA_GAMEPAD_NULL;

    atra_context_init(width, height);
   
    if (!window.window) {
        atra_log_print(LOG_FATAL, "core: Unable to create window");
        atra_util_sdl_printerror();
        atra_util_die(NULL);
    }
}

/* Initialize the window with an OpenGL context */
ATRA_INLINE
void
atra_context_init(int width, int height)
{
    SDL_DisplayMode display;
    int version_major, version_minor;
    int displayfail;

    window.screen.w = width;
    window.screen.h = height;
    glm_mat4_identity(window.screen_scale);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        atra_log_print(LOG_FATAL, "core: Unable to initialize SDL");
        atra_util_sdl_printerror();
        atra_util_die(NULL);
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        atra_log_print(LOG_FATAL, "core: Unable to initialise SDL_image");
        atra_util_sdl_printerror();
        atra_util_die(NULL);
    }

    /* Get monitor dimensions */
    displayfail = SDL_GetCurrentDisplayMode(0, &display);

    if (displayfail) {
        atra_log_print(LOG_ERROR, "core: Unable to get display mode " \
                "for monitor #%d", 0);
        atra_util_sdl_printerror();
        window.monitor.w = DEFAULT_SCREEN_WIDTH;
        window.monitor.h = DEFAULT_SCREEN_HEIGHT;
        window.refresh_rate = DEFAULT_REFRESH_RATE;
    } else {
        window.monitor.w = display.w;
        window.monitor.h = display.h;

        if (display.refresh_rate) {
            window.refresh_rate = display.refresh_rate;
        } else {
            window.refresh_rate = DEFAULT_REFRESH_RATE;
        }
    }

    /* Match window to monitor display settings if window dimensions are 0 */
    if (window.screen.w == 0) window.screen.w = window.monitor.w;
    if (window.screen.h == 0) window.screen.h = window.monitor.h;

    /* Check window flags */
    if (BIT_CHK(window.flags, ATRA_WINDOW_FULLSCREEN)) {
        window.fullscreen = 1;
    }

    if (BIT_CHK(window.flags, ATRA_WINDOW_MINIMIZED)) {
        BIT_CLR(window.flags, ATRA_WINDOW_MINIMIZED);
        atra_log_print(LOG_WARN, "core: Attempted minimization at window init");
    }

    if (BIT_CHK(window.flags, ATRA_WINDOW_MAXIMIZED)) {
        BIT_CLR(window.flags, ATRA_WINDOW_MAXIMIZED);
        atra_log_print(LOG_WARN, "core: Attempted maximization at window init");
    }

    /* Initialise screen display settings */
    window.pos.x = window.monitor.w / 2 - window.screen.w / 2;
    window.pos.y = window.monitor.h / 2 - window.screen.h / 2;

    if (window.pos.x < 0) window.pos.x = 0;
    if (window.pos.y < 0) window.pos.y = 0;
    
    window.render.w = window.screen.w;
    window.render.h = window.screen.h;
    
    /* Report display settings to log */
    atra_log_print(LOG_INFO, "Monitor size: %dx%d", window.monitor.w, window.monitor.h);
    atra_log_print(LOG_INFO, "Window size: %dx%d", window.screen.w, window.screen.h);
    atra_log_print(LOG_INFO, "Render size: %dx%d", window.render.w, window.render.h);
    atra_log_print(LOG_INFO, "Viewport offset: %d, %d", window.render_offset.x, window.render_offset.y);

    /* Prepare window settings */
    window.flags = ATRA_WINDOW_SHOWN |
            ATRA_WINDOW_RESIZABLE |
            ATRA_WINDOW_OPENGL;
    
#if defined(ATRA_OPENGL_VERSION_21)
    version_major = 2;
    version_minor = 1;
#else
    version_major = 3;
    version_minor = 3;
#endif
    atra_log_print(LOG_INFO, "Initialising OpenGL v%d.%d", version_major, version_minor);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, version_major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, version_minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    /* Create the window */
    window.window = SDL_CreateWindow(window.title,
            window.pos.x, window.pos.y,
            window.screen.w, window.screen.h,
            window.flags);

    if (window.window == NULL) {
        atra_log_print(LOG_FATAL, "core: Unable to create window");
        atra_util_sdl_printerror();
        atra_util_die(NULL);
    }

    /* Initialize and bind OpenGL context to the window */
    window.context = SDL_GL_CreateContext(window.window);

    if (window.context == NULL) {
        atra_log_print(LOG_FATAL, "core: Unable to init OpenGL context");
        atra_util_sdl_printerror();
        atra_util_die(NULL);
    }
    SDL_GL_MakeCurrent(window.window, window.context);

    if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0) {
        atra_log_print(LOG_FATAL, "core: Unable to set GLAD process loader");
        atra_util_die(NULL);
    }

    if (BIT_CHK(window.flags, ATRA_WINDOW_VSYNC)) {
        if (SDL_GL_SetSwapInterval(1) < 0) {
            atra_log_print(LOG_ERROR, "core: Unable to enable v-sync");
            atra_util_sdl_printerror();
        }
    }

    atra_window_setupviewport(window.screen.w, window.screen.h);
}

/* Returns 1 if the window should close */
ATRA_API
int
atra_window_shouldclose(void)
{
    return window.should_close;
}

/* Closes the window and uninitializes the OpenGL 3.3 context */
ATRA_API
void
atra_window_close(void)
{
    window.should_close = 1;

    /* TODO: Deallocate window title and other stuff */
}

/* Returns 1 if the window is initialized */
ATRA_API
int
atra_window_isready(void)
{
    return window.ready;
}

/* Returns 1 if the window is in fullscreen mode */
ATRA_API
int
atra_window_isfullscreen(void)
{
    return window.fullscreen;
}

/* Returns 1 if the window is currently hidden */
ATRA_API
int
atra_window_ishidden(void)
{
    return BIT_CHK(window.flags, ATRA_WINDOW_HIDDEN);
}

/* Returns 1 if the window is currently maximized */
ATRA_API
int
atra_window_ismaximized(void)
{
    return BIT_CHK(window.flags, ATRA_WINDOW_MAXIMIZED);
}

/* Returns 1 if the window is currently minimized */
ATRA_API
int
atra_window_isminimized(void)
{
    return BIT_CHK(window.flags, ATRA_WINDOW_MINIMIZED);
}

/* Returns 1 if the window is currently focused */
ATRA_API
int
atra_window_isfocused(void)
{
    return BIT_CHK(window.flags, ATRA_WINDOW_INPUT_FOCUS);
}

/* Returns 1 if the window has been resized during the previous frame */
ATRA_API
int
atra_window_isresized(void)
{
    return window.resized;
}

/* Returns 1 if a specific window flag is set */
ATRA_API
int
atra_window_isflagset(u32 flag)
{
    return BIT_CHK(window.flags, flag);
}

/* Sets the state of the window using flags */
ATRA_API
void
atra_window_setflags(u32 flags)
{
    BIT_SET(window.flags, flags);
}

/* Clears window state flags */
ATRA_API
void
atra_window_clearflags(u32 flags)
{
    BIT_CLR(window.flags, flags);
}

/* Toggles window fullscreen state */
ATRA_API
void
atra_window_togglefullscreen(void)
{
    int fail;

    if (window.fullscreen) {
        fail = SDL_SetWindowFullscreen(window.window, 0);
        window.fullscreen = 0;

        if (fail) {
            atra_log_print(LOG_ERROR, "core: Unable to disable fullscreen");
            atra_util_sdl_printerror();
            window.fullscreen = 1;
        }
    } else {
        fail = SDL_SetWindowFullscreen(window.window, ATRA_WINDOW_FULLSCREEN);
        window.fullscreen = 1;

        if (fail) {
            atra_log_print(LOG_ERROR, "core: Unable to enable fullscreen");
            atra_util_sdl_printerror();
            window.fullscreen = 0;
        }
    }
}

/* Maximizes the window */
ATRA_API
void
atra_window_maximize(void)
{
    SDL_MaximizeWindow(window.window);
}

/* Minimizes the window */
ATRA_API
void
atra_window_minimize(void)
{
    SDL_MinimizeWindow(window.window);
}

/* Restores the window to a non-minimized, non-maximized state */
ATRA_API
void
atra_window_restore(void)
{
    SDL_RestoreWindow(window.window);
}

/* Sets the window icon to the given image */
ATRA_API
void
atra_window_seticon(Image *image)
{
    SDL_Surface *surface;

    surface = SDL_CreateRGBSurfaceFrom(image->data, image->width, image->height,
            32, image->width * 4, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

    SDL_SetWindowIcon(window.window, surface);

    SDL_FreeSurface(surface);
}

/* Sets the title text of the window */
ATRA_API
void
atra_window_settitle(const char *title)
{
    SDL_SetWindowTitle(window.window, title);
}

/* Sets the position of the window */
ATRA_API
void
atra_window_setposition(int x, int y)
{
    SDL_SetWindowPosition(window.window, x, y);
}

/* Sets the minimum allowed size of the window */
ATRA_API
void
atra_window_setminsize(int w, int h)
{
    SDL_SetWindowMinimumSize(window.window, w, h);
}

/* Sets the width and height of the window */
ATRA_API
void
atra_window_setsize(int w, int h)
{
    SDL_SetWindowSize(window.window, w, h);
}

/* Set viewport for a given width and height */
ATRA_API
void
atra_window_setupviewport(int width, int height)
{
    atra_gl_viewport(
        window.render_offset.x / 2,
        window.render_offset.y / 2,
        window.screen.w - window.render_offset.x,
        window.screen.h - window.render_offset.y
    );

    atra_gl_matrixmode(ATRA_PROJECTION);
    atra_gl_loadidentity();
    atra_gl_ortho(0, window.screen.w, window.screen.h, 0, 0.0f, 1.0f);

    atra_gl_matrixmode(ATRA_MODELVIEW);
    atra_gl_loadidentity();
}

/* Returns a pointer to the window handle */
ATRA_API
void *
atra_window_gethandle(void)
{
    return window.window;
}

/* Returns the width of the screen */
ATRA_API
int
atra_window_getwidth(void)
{
    return window.screen.w;
}

/* Returns the height of the screen */
ATRA_API
int
atra_window_getheight(void)
{
    return window.screen.h;
}

/* Swaps the front and back buffer */
ATRA_API
void
atra_window_swapbuffers(void)
{
    SDL_GL_SwapWindow(window.window);
}

/* Returns a pointer to the scaling matrix of the window */
ATRA_API
mat4 *
atra_window_getscreenscalemat(void)
{
    return &window.screen_scale;
}

/* Returns the width of a specified monitor */
ATRA_API
int
atra_monitor_getwidth(void)
{
    return window.monitor.w;
}

/* Returns the height of a specified monitor */
ATRA_API
int
atra_monitor_getheight(void)
{
    return window.monitor.h;
}

/* Returns the refresh rate of a specified monitor */
ATRA_API
int
atra_monitor_getrefreshrate(void)
{
    return window.refresh_rate;
}

/* Sets x and y to the current position of the window */
ATRA_API
void
atra_window_getpos(int *x, int *y)
{
    SDL_GetWindowPosition(window.window, x, y);
}

/* Sets the text in the operating system's clipboard (UTF-8) */
ATRA_API
void
atra_clipboard_settext(const char *text)
{
    SDL_SetClipboardText(text);
}

/* Returns the text in the operating system's clipboard (UTF-8)
 * Must free() after use. */
ATRA_API
char *
atra_clipboard_gettext(void)
{
    return SDL_GetClipboardText();
}


/* 
 *  General input management functions
 */

/* Initialise the input system, automatically detecting devices */
ATRA_INLINE
void
atra_input_init(void)
{
    atra_mouse_init();
    atra_pad_init();
}

/* Updates the input system state to reflect the current state of input devices
 * Must be called each frame for real-time input updates */
ATRA_API
void
atra_input_update(void)
{
    /* TODO: Find way to deal with contradictory single-frame events
     *       i.e. button pressed and released in a single frame 
     */
    SDL_Event event;

    /* Reset queues to original state */
    input.key.keyqueue_size = 0;
    input.key.textqueue_size = 0;
    atra_key_getpressed();
    atra_key_gettext();
    
    /* Update frame dependent variables */
    input.mouse.pos_prev[0] = input.mouse.pos_curr[0];
    input.mouse.pos_prev[1] = input.mouse.pos_curr[1];

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            window.should_close = 1;
        } else if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                window.resized = 1;
                window.screen.w = event.window.data1;
                window.screen.h = event.window.data2;
                //glViewport(0, 0, window.screen.w, window.screen.h);
                atra_window_setupviewport(window.screen.w, window.screen.h);
            }
        }
        atra_key_update(&event);
        atra_mouse_update(&event);
        atra_pad_update(&event);
    }
}


/*
 *  Keyboard functions
 */

/* Update input structure keyboard subsystem state */
ATRA_INLINE
void
atra_key_update(Event *e)
{
    i32 key = e->key.keysym.scancode;

    switch (e->type) {
    case ATRA_KEYDOWN:
        input.key.keystate_prev[key] = input.key.keystate_curr[key];
        input.key.keystate_curr[key] = 1;
        atra_key_pushkeyevent(key);
        break;
    case ATRA_KEYUP:
        input.key.keystate_prev[key] = input.key.keystate_curr[key];
        input.key.keystate_curr[key] = 0;
        break;
    case ATRA_TEXTEDITING:
        input.key.comp_text = e->edit.text;
        input.key.comp_length = e->edit.length;
        input.key.cursor_pos = e->edit.start;
        break;
    case ATRA_TEXTINPUT:
        atra_key_pushtextevent(e->text.text);
        break;
    case ATRA_KEYMAPCHANGED:
        break;
    }
}

ATRA_INLINE
void
atra_key_pushkeyevent(i32 key)
{
    if (input.key.keyqueue_size >= EVENT_MAXQUEUE) {
        atra_log_print(LOG_ERROR, "core: Keyboard input queue overflow: " \
                "dropping input");
        return;
    }
    input.key.keyqueue[input.key.keyqueue_size++] = key;
}

ATRA_INLINE
void
atra_key_pushtextevent(const char text[32])
{
    if (input.key.keyqueue_size >= EVENT_MAXQUEUE) {
        atra_log_print(LOG_ERROR, "core: Keyboard character queue overflow: " \
                "dropping text input");
        return;
    }
    memcpy(input.key.textqueue[input.key.textqueue_size++], text, 32);
}

/* Returns 1 if key is currently up */
ATRA_API
int
atra_key_isup(i32 key)
{
    return input.key.keystate_curr[key] == 0;
}

/* Returns 1 if key is currently down */
ATRA_API
int
atra_key_isdown(i32 key)
{
    return input.key.keystate_curr[key];
}

/* Returns 1 if key has transitioned down in the last frame */
ATRA_API
int
atra_key_ispressed(i32 key)
{
    int curr = input.key.keystate_curr[key];
    int prev = input.key.keystate_prev[key];

    return (curr == 1) && (curr != prev);
}

/* Returns 1 if key has transitioned up in the last frame */
ATRA_API
int
atra_key_isreleased(i32 key)
{
    int curr = input.key.keystate_curr[key];
    int prev = input.key.keystate_prev[key];

    return (curr == 0) && (curr != prev);
}

/* Defines the given key as the key to exit the program (default = 0) */
ATRA_API
void
atra_key_setexit(i32 key)
{
    input.key.exit_key = key;
}

/* Returns and pops the next key pressed from the input queue */
ATRA_API
i32
atra_key_getpressed(void)
{
    static i32 index = 0;

    if (index >= input.key.keyqueue_size) {
        index = 0;
        return ATRA_KEY_NULL;
    } else {
        index++;
        return input.key.keyqueue[index];
    }
}

/* Returns the next text input event in the text queue (UTF-8) */
ATRA_API
const char *
atra_key_gettext(void)
{
    static i32 index = 0;

    if (index >= input.key.textqueue_size) {
        index = 0;
        return NULL;
    } else {
        index++;
        return input.key.textqueue[index];
    }
}

/* Returns a pointer to the internal composition text */
ATRA_API
const char *
atra_key_getcomptext(void)
{
    return input.key.comp_text;
}

/* Returns the length of the composition text */
ATRA_API
i32
atra_key_getcomplength(void)
{
    return input.key.comp_length;
}

/* Returns the position of the cursor in the composition text */
ATRA_API
i32
atra_key_getcompcursorpos(void)
{
    return input.key.cursor_pos;
}


/*
 *  Gamepad functions
 */

/* Initialise gamepad system, detecting connected devices */
ATRA_INLINE
void
atra_pad_init(void)
{
    GamePad *pad;
    int joystick_count, i;

    if (SDL_WasInit(SDL_INIT_GAMECONTROLLER) != 1) {
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    }

    /* Get number of connected gamepads */
    joystick_count = SDL_NumJoysticks();
    input.pad.padcount = 0;

    for (i = 0; i < joystick_count; i++) {
        input.pad.padcount += SDL_IsGameController(i);
    }

    /* Connect gamepads and record SDL handles */
    if (input.pad.padcount > 0) {
        for (i = 0; i < input.pad.padcount; i++) {
            pad = SDL_GameControllerOpen(i);

            if (SDL_GameControllerGetAttached(pad) == 1) {
                input.pad.pads[i] = pad;
                input.pad.ready[i] = 1;
            } else {
                atra_log_print(LOG_ERROR, "core: Unable to connect controller");
                atra_util_sdl_printerror();
            }
        }
        SDL_GameControllerEventState(SDL_ENABLE);
    }
}

/* Update input structure gamepad subsystem state */
ATRA_INLINE
void
atra_pad_update(Event *e)
{

}

/* Returns 1 if a gamepad is detected and available */
ATRA_API
int
atra_pad_isavailable(i32 gamepad)
{
    if (gamepad < 0 || gamepad >= PAD_MAX) {
        atra_log_print(LOG_WARN, "core: Attempted availability check on " \
                "invalid gamepad ID");
        return 0;
    }

    return input.pad.ready[gamepad];
}

/* Returns 1 if gamepad name matches the name provided */
ATRA_API
int
atra_pad_isnamed(i32 gamepad, const char *name)
{
    const char *joyname;
    
    if (input.pad.ready[gamepad] == 0) {
        atra_log_print(LOG_WARN, "core: Attempted retrieval of name from " \
                "invalid gamepad ID");
    }
    joyname = SDL_JoystickNameForIndex(gamepad);

    if (strcmp(name, joyname) == 0) {
        return 1;
    } else {
        atra_log_print(LOG_WARN, "core: Unnamed gamepad detected");
        atra_util_sdl_printerror();
        return 0;
    }
}

/* Returns 1 if gamepad button is currently up */
ATRA_API
int
atra_pad_isup(i32 gamepad, i32 button)
{
    return input.pad.buttonstate_curr[gamepad][button] == 0;
}

/* Returns 1 if gamepad button is currently down */
ATRA_API
int
atra_pad_isdown(i32 gamepad, i32 button)
{
    return input.pad.buttonstate_curr[gamepad][button];
}

/* Returns 1 if gamepad button had transitioned down in the last frame */
ATRA_API
int
atra_pad_ispressed(i32 gamepad, i32 button)
{
    int curr = input.pad.buttonstate_curr[gamepad][button];
    int prev = input.pad.buttonstate_prev[gamepad][button];

    return (curr == 1) && (curr != prev);
}

/* Returns 1 if gamepad button had transitioned up in the last frame */
ATRA_API
int
atra_pad_isreleased(i32 gamepad, i32 button)
{
    int curr = input.pad.buttonstate_curr[gamepad][button];
    int prev = input.pad.buttonstate_prev[gamepad][button];

    return (curr == 0) && (curr != prev);
}

/* Returns and pops the next gamepad button pressed from the input queue */
ATRA_API
i32
atra_pad_getpressed(void)
{
    static int index = 0;

    if (index >= input.pad.eventqueue_size) {
        index = 0;
        return 0;
    } else {
        return input.pad.eventqueue[index++];
    }

}

/* Returns the number of analog inputs on the gamepad */
ATRA_API
i32
atra_pad_getanalogcount(void)
{

}

/* Returns the movement value for a specific analog input */
ATRA_API
float
atra_pad_getanalogmovement(i32 gamepad, i32 analog_input)
{

}

/* Sets the internal gamepad mappings */
ATRA_API
int
atra_pad_setmappings(const char *mappings)
{

}


/*
 *  Mouse functions
 */

/* Initialises the mouse input subsystem */
ATRA_INLINE
void
atra_mouse_init(void)
{
    input.mouse.cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    
    if (input.mouse.cursor == NULL) {
        atra_log_print(LOG_ERROR, "Mouse cursor style changing is unsupported");
        atra_util_sdl_printerror();
    }
    SDL_SetCursor(input.mouse.cursor);
}

/* Update input structure mouse subsystem state */
ATRA_INLINE
void
atra_mouse_update(Event *e)
{
    u8 button;

    switch (e->type) {
    case ATRA_MOUSEMOTION:
        input.mouse.pos_prev[0] = input.mouse.pos_curr[0];
        input.mouse.pos_prev[1] = input.mouse.pos_curr[1];
        input.mouse.pos_curr[0] = e->motion.x;
        input.mouse.pos_curr[1] = e->motion.y;
        break;
    case ATRA_MOUSEBUTTONDOWN:
        button = e->button.button;
        input.mouse.buttons_prev[button] = input.mouse.buttons_curr[button];
        input.mouse.buttons_curr[button] = 1;
        break;
    case ATRA_MOUSEBUTTONUP:
        button = e->button.button;
        input.mouse.buttons_prev[button] = input.mouse.buttons_curr[button];
        input.mouse.buttons_curr[button] = 0;
        break;
    case ATRA_MOUSEWHEEL:
        e->wheel.y *= 1 - 2 * (e->wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
        input.mouse.wheelmove_prev = input.mouse.wheelmove_curr;
        input.mouse.wheelmove_curr = e->wheel.y;
    }
}

/* Returns 1 if mouse button is currently up */
ATRA_API
int
atra_mouse_isup(i32 button)
{
    return input.mouse.buttons_curr[button] == 0;
}

/* Returns 1 if mouse button is currently down */
ATRA_API
int
atra_mouse_isdown(i32 button)
{
    return input.mouse.buttons_curr[button];
}

/* Returns 1 if mouse button had transitioned down in the last frame */
ATRA_API
int
atra_mouse_ispressed(i32 button)
{
    i32 curr = input.mouse.buttons_curr[button];
    i32 prev = input.mouse.buttons_prev[button];

    return (curr == 1) && (curr != prev);
}

/* Returns 1 if mouse button had transitioned up in the last frame */
ATRA_API
int
atra_mouse_isreleased(i32 button)
{
    i32 curr = input.mouse.buttons_curr[button];
    i32 prev = input.mouse.buttons_prev[button];

    return (curr == 0) && (curr != prev);
}

/* Sets x and y to window-space coordinates of the mouse cursor */
ATRA_API
void
atra_mouse_getpos(int *x, int *y)
{
    *x = input.mouse.pos_curr[0];
    *y = input.mouse.pos_curr[1];
}

/* Sets x and y to the relative change in mouse coordinates between frames */
ATRA_API
void
atra_mouse_getposchange(int *x, int *y)
{
    if (input.mouse.cursor_locked) {
        SDL_GetRelativeMouseState(x, y);
    } else {
        *x = input.mouse.pos_curr[0] - input.mouse.pos_prev[0];
        *y = input.mouse.pos_curr[1] - input.mouse.pos_prev[1];
    }
}

/* Sets the mouse cursor position to (x, y) */
ATRA_API
void
atra_mouse_setpos(int x, int y)
{
    SDL_WarpMouseInWindow(window.window, x, y);
    input.mouse.pos_curr[0] = x;
    input.mouse.pos_curr[1] = y;
}

/* Returns the mouse wheel movement value (+/- y) */
ATRA_API
float
atra_mouse_getwheelmovement(void)
{
    return input.mouse.wheelmove_curr;
}

/* Sets the mouse cursor display style */
ATRA_API
void
atra_mouse_setcursor(i32 cursor)
{
    SDL_FreeCursor(input.mouse.cursor);
    input.mouse.cursor = SDL_CreateSystemCursor(cursor);

    if (input.mouse.cursor == NULL) {
        atra_log_print(LOG_ERROR, "core: Cursor style changing is unsupported");
        atra_util_sdl_printerror();
    }
    SDL_SetCursor(input.mouse.cursor);
}

/* Hides the mouse cursor and locks it to the centre of the screen */
ATRA_API
void
atra_mouse_lock(void)
{
    input.mouse.cursor_hidden = 1;
    input.mouse.cursor_locked = 1;
    SDL_SetRelativeMouseMode(1);
}

/* Shows the mouse cursor, freeing it from its locked state */
ATRA_API
void
atra_mouse_unlock(void)
{
    input.mouse.cursor_hidden = 0;
    input.mouse.cursor_locked = 0;
    SDL_SetRelativeMouseMode(0);
}

/*
 *  Time related functions
 */

/* Initialise the time subsystem */
ATRA_INLINE
void
atra_time_init(void)
{
    time_data.current = SDL_GetTicks();
    time_data.prev = time_data.current;
}

/* Update the time subsystem */
ATRA_INLINE
void
atra_time_update(void)
{
    time_data.prev = time_data.current;
    time_data.current = SDL_GetTicks();
}

/* Returns the number of milliseconds since Atra Engine's initialization */
ATRA_API
u32
atra_time_getcurrent(void)
{
    return time_data.current;
}

/* Returns the change in time since the last time this function was called
 * Returns the same as atra_time_getcurrent() on first call */
ATRA_API
u32
atra_time_getdelta(void)
{
    return time_data.current - time_data.prev;
}

/* Returns the number of milliseconds elapsed during the update phase */
ATRA_API
u32
atra_time_getupdatetime(void)
{
    return time_data.update;
}

/* Returns the number of milliseconds elapsed during the render phase */
ATRA_API
u32
atra_time_getrendertime(void)
{
    return time_data.render;
}

/* Returns the number of milliseconds elapsed in drawing a single frame */
ATRA_API
u32
atra_time_getframetime(void)
{
    return time_data.frame;
}

/* Returns the total number of frames drawn */
ATRA_API
u32
atra_time_getframecount(void)
{
    return time_data.frame_count;
}
