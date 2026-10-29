/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved
 *
 *  gui.h - A GUI module for building an interactive user interface.
 *          Provides a selection of basic components, but also allows for the
 *          user to build any custom UI component from basic elements.
 *
 * */

#ifndef ATRA_GUI_H
#define ATRA_GUI_H

#define ATRA_GUI_MAX_TITLE_LEN      64
#define ATRA_GUI_MAX_ID_LEN         64
#define ATRA_GUI_MAX_ANIM_TFORMS    8
#define ATRA_GUI_SCROLL_PIXEL_COUNT 20
#define ATRA_GUI_SCROLL_SPEED       10

#define ATRA_GUI_DEFAULT_FONT_PATH  "res/font/encode-sans/EncodeSans-Regular.ttf"
#define ATRA_GUI_DEFAULT_FONT_SIZE  64

#include "common.h"
#include "texture.h"
#include "font.h"
#include "model.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Element preset types */
enum gui_elem_type {
    ATRA_GUI_NONE,          /* Typeless */
    ATRA_GUI_PANEL,         /* Sub-window to group elements with */
    ATRA_GUI_LABEL,         /* Static text display */
    ATRA_GUI_BUTTON,        /* Button that can be pushed to trigger an event */
    ATRA_GUI_TOGGLE,        /* Togglable button */
    ATRA_GUI_SLIDER,        /* Variable slider between a min and max value */
    ATRA_GUI_PROGRESSBAR,   /* Bar which visually displays percentage progress */
    ATRA_GUI_SCROLLBAR,     /* Bar to control and display scrolling in a window/panel */
    ATRA_GUI_TEXTBOX,       /* Multi-line text box which accepts keyboard input */
    ATRA_GUI_TEXTFIELD,     /* Single-line text box which accepts keyboard input */
    ATRA_GUI_COMBOBOX,      /* Single-choice dropdown menu which displays the selected option */
    ATRA_GUI_MENU,          /* List of text buttons which disappears on making a selection */
    ATRA_GUI_TOOLTIP        /* A non-interactable window which follows the mouse cursor */
};

enum gui_window_attrib {
    ATRA_GUI_WINDOW_MOVABLE     = BIT(0),   /* Can be moved using the header */
    ATRA_GUI_WINDOW_SCALABLE    = BIT(1),   /* Can be scaled using edges */
    ATRA_GUI_WINDOW_CLOSABLE    = BIT(2),   /* Has `X` button on header to close window */
    ATRA_GUI_WINDOW_MINIMISABLE = BIT(3),   /* Has `-` button on header to minimise window */
    ATRA_GUI_WINDOW_SCROLLABLE  = BIT(4),   /* Can be scrolled (adds scrollbar) */
    ATRA_GUI_WINDOW_HIDESCROLL  = BIT(5),   /* Scrollbar autohides when not in use */
    ATRA_GUI_WINDOW_FALLTHROUGH = BIT(6),   /* Input events are not captured by the window */
    ATRA_GUI_WINDOW_ALWAYSTOP   = BIT(7),   /* Is always placed as close to stack top as possible */
    ATRA_GUI_WINDOW_ALWAYSBOT   = BIT(8),   /* Is always places as close to stack bottom as possible */
    ATRA_GUI_WINDOW_HASTITLE    = BIT(9),   /* Has window name text displayed in the header */
    ATRA_GUI_WINDOW_HASBORDER   = BIT(10)   /* Has a border outlining the window */
};

enum gui_window_state {
    ATRA_GUI_WINDOW_MINIMISED = BIT(0),     /* Is currently minimised */
    ATRA_GUI_WINDOW_HIDDEN    = BIT(1),     /* Is not drawn, even the header is hidden */
    ATRA_GUI_WINDOW_CLOSED    = BIT(2),     /* Window is closed and will be deleted */
    ATRA_GUI_WINDOW_SCROLLED  = BIT(3)      /* Window has recieved a scroll event */
};

enum gui_elem_attrib {
    ATRA_GUI_ELEM_CLICKABLE   = BIT(0),     /* Accepts mouse click input events */
    ATRA_GUI_ELEM_TOGGLING    = BIT(1),     /*  */
    ATRA_GUI_ELEM_AUTOSCALE   = BIT(2),     /* Auto-scales relative to window dimensions */
    ATRA_GUI_ELEM_HASTEXT     = BIT(3),     /* Contains drawable text */
    ATRA_GUI_ELEM_TEXTINPUT   = BIT(4),     /* Accepts keyboard text input */
    ATRA_GUI_ELEM_MULTILINE   = BIT(5),     /* Displays text over multiple lines */
    ATRA_GUI_ELEM_NOWRAP      = BIT(6),     /* Disable wrapping words to the next line */
    ATRA_GUI_ELEM_SCROLLABLE  = BIT(7),     /* Can be scrolled (adds scrollbar, implies SCISSORTEST) */
    ATRA_GUI_ELEM_SCISSORTEST = BIT(8),     /* Enables scissor test and scrolling (without scrollbar control) */
    ATRA_GUI_ELEM_HIDESCROLL  = BIT(9),     /* Scrollbar autohides when not in use */
    ATRA_GUI_ELEM_HASBORDER   = BIT(10),     /* Has a border outlining the element */
    ATRA_GUI_ELEM_FALLTHROUGH = BIT(11),    /* Mouse input falls through to next element/window */
    ATRA_GUI_ELEM_KEEPRATIO_W = BIT(12),    /* Keeps element aspect ratio (constant width) */
    ATRA_GUI_ELEM_KEEPRATIO_H = BIT(13),    /* Keeps element aspect ratio (constant height) */
    ATRA_GUI_ELEM_HIDDEN      = BIT(14),    /* Prevents rendering of the element */
    ATRA_GUI_ELEM_HIDEALL     = BIT(15),    /* Prevents rendering of the element and its children */
    ATRA_GUI_ELEM_AUTOALIGN   = BIT(16)     /* Auto-align all of the element's children */
};

enum gui_elem_state {
    ATRA_GUI_ELEM_NORMAL,
    ATRA_GUI_ELEM_HOVER,
    ATRA_GUI_ELEM_PRESS,
    ATRA_GUI_ELEM_ACTIVE,
    ATRA_GUI_ELEM_DISABLED
};

enum gui_event_type {
    ATRA_GUI_EVENT_MOUSEOVER,   /* Mouse cursor starts hovering over element */
    ATRA_GUI_EVENT_MOUSELEAVE,  /* Mouse cursor stops hovering over element */
    ATRA_GUI_EVENT_MOUSEDOWN,   /* Triggered on pressing mouse button on an element */
    ATRA_GUI_EVENT_MOUSEUP,     /* Triggered on releasing mouse button on an element */
    ATRA_GUI_EVENT_MOUSECLICK,  /* Full down-up mouse click pair on same element */
    ATRA_GUI_EVENT_SCROLLUP,    /* Scroll wheel is rolled 'up' while hovering this element */
    ATRA_GUI_EVENT_SCROLLDN,    /* Scroll wheel is rolled 'down' while hovering this element */
    ATRA_GUI_EVENT_GAINFOCUS,   /* This element has gained input focus */
    ATRA_GUI_EVENT_LOSEFOCUS,   /* This element has lost input focus */
    ATRA_GUI_EVENT_KEYDOWN,     /* Keyboard button has been pressed while elem is focused */
    ATRA_GUI_EVENT_KEYUP,       /* Keyboard button has been released while elem is focused */
    ATRA_GUI_EVENT_CLOSE,       /* Triggered just before element deletion */
    ATRA_GUI_EVENT_TEXTENTERKEY,/* Triggers on ENTER keypress on single-line text input element */
    ATRA_GUI_EVENT_TOTAL
}; 

enum gui_align {
    ATRA_GUI_TEXTALIGN_LEFT   = BIT(0),
    ATRA_GUI_TEXTALIGN_CENTRE = BIT(1),
    ATRA_GUI_TEXTALIGN_RIGHT  = BIT(2),
    ATRA_GUI_TEXTALIGN_TOP    = BIT(3),
    ATRA_GUI_TEXTALIGN_MIDDLE = BIT(4),
    ATRA_GUI_TEXTALIGN_BOTTOM = BIT(5),
};

enum gui_alignment {
    ATRA_GUI_TEXTALIGN_MID_LEFT   = ATRA_GUI_TEXTALIGN_MIDDLE | ATRA_GUI_TEXTALIGN_LEFT,
    ATRA_GUI_TEXTALIGN_MID_CENTRE = ATRA_GUI_TEXTALIGN_MIDDLE | ATRA_GUI_TEXTALIGN_CENTRE,
    ATRA_GUI_TEXTALIGN_MID_RIGHT  = ATRA_GUI_TEXTALIGN_MIDDLE | ATRA_GUI_TEXTALIGN_RIGHT
};

enum gui_style_entry_type {
    ATRA_GUI_STYLE_COLOR,
    ATRA_GUI_STYLE_IMAGE
};

enum gui_style_type {
    ATRA_GUI_STYLE_BASE,
    ATRA_GUI_STYLE_TEXT,
    ATRA_GUI_STYLE_TEXTEDIT,
    ATRA_GUI_STYLE_TEXTSELECTION,
    ATRA_GUI_STYLE_TEXTCURSOR,
    ATRA_GUI_STYLE_SCROLLBAR
};

enum gui_easing_func {
    ATRA_GUI_EASING_SINE_IN,
    ATRA_GUI_EASING_SINE_OUT,
    ATRA_GUI_EASING_SINE_BOTH,
    ATRA_GUI_EASING_QUAD_IN,
    ATRA_GUI_EASING_QUAD_OUT,
    ATRA_GUI_EASING_QUAD_BOTH,
    ATRA_GUI_EASING_CUBIC_IN,
    ATRA_GUI_EASING_CUBIC_OUT,
    ATRA_GUI_EASING_CUBIC_BOTH,
    ATRA_GUI_EASING_QUART_IN,
    ATRA_GUI_EASING_QUART_OUT,
    ATRA_GUI_EASING_QUART_BOTH,
    ATRA_GUI_EASING_QUINT_IN,
    ATRA_GUI_EASING_QUINT_OUT,
    ATRA_GUI_EASING_QUINT_BOTH,
    ATRA_GUI_EASING_EXP_IN,
    ATRA_GUI_EASING_EXP_OUT,
    ATRA_GUI_EASING_EXP_BOTH,
    ATRA_GUI_EASING_CIRC_IN,
    ATRA_GUI_EASING_CIRC_OUT,
    ATRA_GUI_EASING_CIRC_BOTH,
    ATRA_GUI_EASING_BACK_IN,
    ATRA_GUI_EASING_BACK_OUT,
    ATRA_GUI_EASING_BACK_BOTH,
    ATRA_GUI_EASING_ELASTIC_IN,
    ATRA_GUI_EASING_ELASTIC_OUT,
    ATRA_GUI_EASING_ELASTIC_BOTH,
    ATRA_GUI_EASING_BOUNCE_IN,
    ATRA_GUI_EASING_BOUNCE_OUT,
    ATRA_GUI_EASING_BOUNCE_BOTH
};

enum gui_cursor_shape {
    ATRA_GUI_CURSOR_BAR,
    ATRA_GUI_CURSOR_UNDERSCORE,
    ATRA_GUI_CURSOR_BLOCK
};

enum gui_contraint_behavior_type {
    ATRA_GUI_CONS_DIST_TOP	 = BIT(0),	/* Y position based on distance of top of elem from top of parent */
    ATRA_GUI_CONS_DIST_BOT	 = BIT(1),	/* Y position based on distance of bottom of elem from bottom of parent */
    ATRA_GUI_CONS_DIST_LEFT	 = BIT(2),	/* X position based on distance of top of elem from top of parent */
    ATRA_GUI_CONS_DIST_RIGHT = BIT(3),	/* X position based on distance of bottom of elem from bottom of parent */
	ATRA_GUI_CONS_MIDDLE_X	 = BIT(4),	/* Overrides Y position alignment options to place elem in middle of parent */
	ATRA_GUI_CONS_MIDDLE_Y	 = BIT(5)	/* Overrides X position alignment options to place elem in middle of parent */
};

enum gui_constraint_data_type {
    ATRA_GUI_CONS_AUTO,     /* Allow the window to automatically decide the constraint */
    ATRA_GUI_CONS_PERCENT,  /* Constraint size measured as a percentage of parent [0, 1] */
    ATRA_GUI_CONS_PIXEL,    /* Constraint size measured in pixels */
};

typedef struct GUIStyleEntry {
    int type;
    union {
        Color color;
        vec2 tex_uvs[4];
    } data;
} GUIStyleEntry;

typedef struct GUIBorderStyle {
    Color color;            /* Color of border */
    uint size;              /* Border size, in pixels */
} GUIBorderStyle;

/* Style of the background/body of an element */
typedef struct GUIBaseStyle {
    GUIStyleEntry normal;
    GUIStyleEntry hover;
    GUIStyleEntry pressed;
    GUIStyleEntry disabled;
    GUIBorderStyle border;
    vec2 padding;           /* Empty space between border and external elements */
    vec2 margin;            /* Empty space between border and internal contents */
    float rounding;         /* Radius of border rounding, in pixels */
} GUIBaseStyle;

/* Style used for an element with static text */
typedef struct GUITextStyle {
    Color normal;
    Color hover;
    Color pressed;
    Color disabled;
    Font *font;         /* Pointer to text font  */
    float size;         /* Height of font characters */
    float spacing;      /* Space between text */
} GUITextStyle;

/* Style used for an element with editable text */
typedef struct GUITextEditStyle {
    Color normal;
    Color hover;
    Color pressed;
    Color disabled;
} GUITextEditStyle;

/* Style for text selected with the mouse cursor */
typedef struct GUISelectedTextStyle {
    Color background;   /* Selection background highlighting */
    Color foreground;   /* Color of selected text */
} GUISelectedTextStyle;

/* Style relating to the text edit cursor */
typedef struct GUITextCursorStyle {
    int shape;          /* Cursor shape style */
    int blink_rate;     /* Number of times per second the cursor toggles */
    Color bg_normal;
    Color bg_hover;
    Color fg_normal;
    Color fg_hover;
} GUITextCursorStyle;

typedef struct GUISliderStyle {
    GUIBaseStyle body;
    GUIBaseStyle fill;
    GUIBaseStyle bar;
} GUISliderStyle;

typedef struct GUIProgressBarStyle {
    GUIBaseStyle body;
    GUIBaseStyle fill;
    GUITextStyle text;
    int has_text;
} GUIProgressBarStyle;

typedef struct GUIComboBoxStyle {
    GUIBaseStyle body;
    GUIBaseStyle menu;
    GUIBaseStyle entry;
    GUITextStyle text;
} GUIComboBoxStyle;

typedef struct GUIMenuStyle {
    GUIBaseStyle body;
    GUIBaseStyle entry;
    GUITextStyle text;
} GUIMenuStyle;

/* Style for the special scrollbar element */
typedef struct GUIScrollbarStyle {
    GUIBaseStyle body;
    GUIBaseStyle bar;
    GUIBaseStyle button_inc;
    GUIBaseStyle button_dec;
    int has_buttons;
} GUIScrollbarStyle;

typedef struct GUIElemStyle {
    GUIBaseStyle *base;                 /* Main body of the element */
    GUITextStyle *text;                 /* Static text */
    GUITextEditStyle *edit;             /* Editable text */
    GUISelectedTextStyle *selection;    /* Selected text */
    GUITextCursorStyle *cursor;         /* Text cursor */
    GUIScrollbarStyle *scrollbar;       /* Scrollbar component */
} GUIElemStyle;

typedef struct GUIConstraint {
    int data_type;
    union {
        int pixels;
        float percent;
    } value;
} GUIConstraint;

typedef struct GUIAnim {
    mat4 tforms[ATRA_GUI_MAX_ANIM_TFORMS];          /* Keyframe transforms */
    float tform_times[ATRA_GUI_MAX_ANIM_TFORMS];    /* Time (secs) of each keyframe */
    Color color_mod[ATRA_GUI_MAX_ANIM_TFORMS];      /* Color modifier for each keyframe */
    int tform_count;    /* Number of keyframe transforms */
    int easing_func;    /* Function used to interpolate between transforms */
    int repeating;      /* Should the animation repeat immediately after finishing? */
} GUIAnim;

/* A quad with ability for measurement in either pixels or percentages */
typedef struct GUIQuad {
    int types[4];       /* Data types of each dimension parameter */
    float x, y, w, h;
} GUIQuad;

/* A line of text pre-processed for drawing */
typedef struct GUITextLine {
    i32 *codes;     /* Pointer to codepoint array where complete text is stored */
    int length;     /* Number of codepoints in line */
    float width;    /* Width of resulting line when drawn to screen */
} GUILineText;

typedef struct GUIElem GUIElem;
typedef struct GUIWindow GUIWindow;

/* GUI elements contained inside a window */
struct GUIElem {
    char id_str[ATRA_GUI_MAX_ID_LEN];   /* String identifier (hash key) */
    GUIQuad quad;       /* Position and dimension contraints */
    Quad real_quad;     /* Screen-space quad, taking parent contraints into account */
    GUIAnim *anim;      /* Pointer to the current animation */
    GUIElemStyle style; /* Current element style */
    float anim_time;    /* Current time elapsed in animation */
    u32 attrs;          /* Element attribute flags */
    u32 state;          /* Current element state */
	u32 align;			/* How element quad data should be interpreted in relation to parent */
    u32 text_align;     /* Alignment rules to use when aligning text */
    int dirty;          /* Set when element requires a mesh rebuild */
    float ratio;        /* Element width/height ratio */

    char *text;         /* Pointer to string to draw inside element */
    i32 *codepoints;    /* UTF-8 codepoint array equivalent of `text` */
    i32 code_count;     /* Number of UTF-8 codepoints in codepoint array */
    List rows;          /* Pre-processed lines of text for rendering */
    int rebuild_rows;   /* Flag stating that the text row data should be rebuilt */

    mat4 transform;     /* Model matrix transform */

    Material *material; /* Pointer to material used to draw the main body mesh */
    Font *font;         /* Pointer to font used by the element */ 

    int scroll_x;       /* Scroll amount on the X axis */
    int scroll_y;       /* Scroll amount on the Y axis */
    GUIConstraint scroll_x_max;   /* Scroll amount maximum on the X axis (0 = no limit) */
    GUIConstraint scroll_y_max;   /* Scroll amount maximum on the Y axis (0 = no limit) */
    int scroll_amount_x;/* Smooth scrolling pixel difference until target (horizontal) */
    int scroll_amount_y;/* Smooth scrolling pixel difference until target (vertical) */
    int smooth_scroll;
    float scroll_timer; /* Time since last input event on scrollbar */
    GUIElem *scrollbar; /* Pointer to special scrollbar element */

    GUIWindow *window;  /* Pointer to the containing window of this element */
    GUIElem *parent;    /* Pointer to the directly containing element of this element */
    List children;      /* List of pointers to child elements */

    void (*update)(GUIElem *self, uint dt);                             /* Update callback */
    void (*callbacks[ATRA_GUI_EVENT_TOTAL])(GUIElem *self, int data);   /* Event callbacks */
};

struct GUIWindow {
    char id_str[ATRA_GUI_MAX_ID_LEN];   /* String identifier (hash key) */
    char title[ATRA_GUI_MAX_TITLE_LEN]; /* Window title to be displayed in header */
    GUIElem *elem;                      /* Body element of the window */
    u32 attrs;                          /* Window attribute flags */
    u32 states;                         /* Window state flags */
    int dirty;                          /* Flag signalling need for remesh */

    /* TODO: Maybe add a flag that will force remesh every frame if an element is animating? */
    /* TODO: Migrate List elements to unified table structure */

    int mesh_index;         /* List index of the current mesh */
    List meshes;            /* List of meshes to draw (Mesh) */
    List materials;         /* List of materials used by each mesh (Material *) */
    List transforms;        /* List of transforms for each mesh (mat4) */
    List scissors;          /* List of quads for scissor tests (IQuad) */
};


/*
 *  General Module Functions
 */

/* Initialise the GUI subsystem */
ATRA_API void atra_gui_init(void);

/* Shutdown the GUI subsystem */
ATRA_API void atra_gui_stop(void);

/* Handles user input events on the GUI elements.
 * Returns true if a mouse event was captured by a window, otherwise false. */
ATRA_API int atra_gui_handle(void);

/* Updates GUI elements for the current frame */
ATRA_API void atra_gui_update(uint dt);

/* Draws all windows, and their elements, in the window stack */
ATRA_API void atra_gui_render(void);

/* Signals a remesh on all windows and elements */
ATRA_API void atra_gui_fullremesh(void);

/* Sets the GUI texture atlas for all elements */
ATRA_API void atra_gui_settextureatlas(Texture *tex);


/*
 *  GUI Window Functions
 */

/* Creates a new window, automatically adding it to the window stack.
 * Returns a pointer to the new window. */
ATRA_API GUIWindow * atra_gui_window_create(u32 attrib, u32 state, const char *id_str);

/* Deallocates the given window, automatically removing it from the window stack. */
ATRA_API void atra_gui_window_delete(GUIWindow *window);

/* Runs updates on all of a window's elements */
ATRA_API void atra_gui_window_update(GUIWindow *window, uint dt);

/* Populates the window's mesh list with data from all contained elements */
ATRA_API void atra_gui_window_mesh(GUIWindow *window);

/* Deletes all mesh data from the window, leaving the relevant lists empty */
ATRA_API void atra_gui_window_clearmeshdata(GUIWindow *window);

/* Draws the window's meshes to the screen */
ATRA_API void atra_gui_window_render(GUIWindow *window);

/* Returns a pointer to a window with a matching ID string.
 * Returns NULL if no such window exists */
ATRA_API GUIWindow * atra_gui_window_find(const char *id);

/* Sets the window's base element */
ATRA_API void atra_gui_window_setelem(GUIWindow *window, GUIElem *elem);

/* Sets the window title string */
ATRA_API void atra_gui_window_settitle(GUIWindow *window, const char *title);


/*
 *  GUI Element Functions
 */

/* Creates a new GUI element.
 * This element must be managed and deleted by the user. */
ATRA_API GUIElem * atra_gui_elem_create(const char *id, u32 attrib, GUIQuad quad);

/* Deletes the given element, along with all its children */
ATRA_API void atra_gui_elem_delete(GUIElem *elem);

/* Handle user input for the given element and all of it's children */
ATRA_API int atra_gui_elem_handle(GUIElem *elem);

/* Updates the given element and all of it's children */
ATRA_API void atra_gui_elem_update(GUIElem *elem, uint dt);

/* Returns the number of elements in an element tree, given the root element */
ATRA_API int atra_gui_elem_treesize(GUIElem *elem);

/* Sets the X position constraint of the element */
ATRA_API void atra_gui_elem_setpos_x(GUIElem *elem, int type, float data);

/* Sets the Y position constraint of the element */
ATRA_API void atra_gui_elem_setpos_y(GUIElem *elem, int type, float data);

/* Sets the width constraint of the element */
ATRA_API void atra_gui_elem_setwidth(GUIElem *elem, int type, float data);

/* Sets the height constraint of the element */
ATRA_API void atra_gui_elem_setheight(GUIElem *elem, int type, float data);

/* Sets the maximum scroll amount of the element on the x axis */
ATRA_API void atra_gui_elem_setmaxscroll_x(GUIElem *elem, int type, float data);

/* Sets the maximum scroll amount of the element on the y axis */
ATRA_API void atra_gui_elem_setmaxscroll_y(GUIElem *elem, int type, float data);

/* Sets the alignment constraint flags of the element */
ATRA_API void atra_gui_elem_setalignment(GUIElem *elem, u32 flags);

/* Sets the pointer to a specific style subtype */
ATRA_API void atra_gui_elem_setstyle(GUIElem *elem, int type, void *style);

/* Set the element's contained text data, additionally constructs codepoint array */
ATRA_API void atra_gui_elem_settext(GUIElem *elem, const char *text);

/* Set the element's text alignment mode */
ATRA_API void atra_gui_elem_settextalign(GUIElem *elem, int alignment);

/* Alters the element's style to hide everything except its child elements.
 * Useful for having a group of floating elements without a background. */
/* TODO:  Maybe just skip meshing the element via a flag so it is reversible */
ATRA_API void atra_gui_elem_setinvisible(GUIElem *elem);

/* Flags both an element and it's window for remeshing */
ATRA_API void atra_gui_elem_flagremesh(GUIElem *elem);

/* Calculates the element's internal screen-space quad and that of all of it's children */
ATRA_API void atra_gui_elem_calcquad(GUIElem *elem);

/* Calculates the quads for an element's children in auto-align mode */
ATRA_API void atra_gui_elem_calcquad_auto(GUIElem *elem);

/* Returns a quad containing the screen-space pixel data */
ATRA_API IQuad atra_gui_elem_getquad_pix(GUIElem *elem);

/* Returns a quad containing the screen-space normalised [0, 1] data */
ATRA_API Quad atra_gui_elem_getquad_norm(GUIElem *elem);

/* Returns the position of the element in normalised coordinates.
 * Will take parent element position into account when `relative` flag is set */
ATRA_API void atra_gui_elem_getpos_norm(GUIElem *elem, float * restrict x, float * restrict y);

/* Returns the width of the element in normalised coordinates. 
 * Will take parent element dimensions into account when `relative` flag is set */
ATRA_API float atra_gui_elem_getwidth_norm(GUIElem *elem);

/* Returns the height of the element in normalised coordinates.
 * Will take parent element dimensions into account when `relative` flag is set */
ATRA_API float atra_gui_elem_getheight_norm(GUIElem *elem);

/* Composes an element's codepoints into row data for further processing */
ATRA_API void atra_gui_elem_calcrowdata(GUIElem *elem);

/* Draws a string inside a given rectangular bound */
ATRA_API void atra_gui_elem_drawtext(GUIElem *elem, Mesh *mesh, float depth);

/* Plays an animation for a GUI element with an optional delay (in milliseconds) */
ATRA_API void atra_gui_elem_playanim(GUIElem *elem, GUIAnim *anim, uint delay);

/* Returns the element with the matching string identifier */
ATRA_API GUIElem * atra_gui_elem_find(const char *str_id);

/* Adds an element as a child to an element */
ATRA_API void atra_gui_elem_addelem(GUIElem *parent, GUIElem *child);

/* Translates the element transform by the given x and y */
ATRA_API void atra_gui_elem_translate(GUIElem *elem, float x, float y);

/* Rotates the element transform by the given x, y and z */
ATRA_API void atra_gui_elem_rotate(GUIElem *elem, float x, float y, float z, float angle);

/* Scales the element transform by the given x and y */
ATRA_API void atra_gui_elem_scale(GUIElem *elem, float x, float y);


/*
 *  Specialised Element Creation Functions
 */

ATRA_API GUIElem * atra_gui_elem_create_panel(const char *id, int attrib, GUIQuad quad);
ATRA_API GUIElem * atra_gui_elem_create_label(const char *id, int attrib, GUIQuad quad);
ATRA_API GUIElem * atra_gui_elem_create_button(const char *id, int attrib, GUIQuad quad);
ATRA_API GUIElem * atra_gui_elem_create_toggle(const char *id, int attrib, GUIQuad quad);
ATRA_API GUIElem * atra_gui_elem_create_slider(const char *id, int attrib, GUIQuad quad);
ATRA_API GUIElem * atra_gui_elem_create_progressbar(const char *id, int attrib, GUIQuad quad);
ATRA_API GUIElem * atra_gui_elem_create_scrollbar(const char *id, int attrib, GUIQuad quad);
ATRA_API GUIElem * atra_gui_elem_create_textbox(const char *id, int attrib, GUIQuad quad);
ATRA_API GUIElem * atra_gui_elem_create_textfield(const char *id, int attrib, GUIQuad quad);
ATRA_API GUIElem * atra_gui_elem_create_combobox(const char *id, int attrib, GUIQuad quad);
ATRA_API GUIElem * atra_gui_elem_create_menu(const char *id, int attrib, GUIQuad quad);
ATRA_API GUIElem * atra_gui_elem_create_tooltip(const char *id, int attrib, GUIQuad quad);


/* 
 *  Specialised Element Auxiliary Functions
 */

/* Set the amount of the progress bar which should be filled in range [0, 1] */
ATRA_API void atra_gui_progressbar_setprogress(float prog);

/* Set the text in the centre of the progress bar (e.g. to display a percentage) */
ATRA_API void atra_gui_progressbar_settext(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* ATRA_GUI_H */
