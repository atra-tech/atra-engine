/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved
 *
 *  gui.c - An immediate mode GUI module integrated with the engine.
 *
 * */

#include "atra.h"

#define MAP_DELFLAG_WINDOW      0
#define MAP_DELFLAG_ELEM        0

/* All basic styles for each element style sub-category */
struct GUIDefaultStyle {
    GUIBaseStyle base;                  /* Main body of the element */
    GUITextStyle text;                  /* Static text */
    GUITextEditStyle edit;              /* Editable text */
    GUISelectedTextStyle selection;     /* Selected text */
    GUITextCursorStyle cursor;          /* Text cursor */
    GUIScrollbarStyle scrollbar;        /* Scrollbar component */
    GUISliderStyle slider;              /* Slider component */
    GUIProgressBarStyle progress;       /* Progress bar component */
    GUIComboBoxStyle combo;             /* Combo box component */
    GUIMenuStyle menu;                  /* Menu component */
} gui_default_elem_style;

/* A collection of default base styles for each auto-constructed element type */
static struct GUIDefaultBaseStyles {
    GUIBaseStyle none;
    GUIBaseStyle panel;
    GUIBaseStyle label;
    GUIBaseStyle button;
    GUIBaseStyle toggle;
    GUIBaseStyle slider;
    GUIBaseStyle progressbar;
    GUIBaseStyle scrollbar;
    GUIBaseStyle textbox;
    GUIBaseStyle textfield;
    GUIBaseStyle combobox;
    GUIBaseStyle menu;
    GUIBaseStyle tooltip;
} gui_base_defaults;

/* A collection of default style compositions */
static struct GUIDefaultStyles{
    GUIElemStyle none;
    GUIElemStyle panel;
    GUIElemStyle label;
    GUIElemStyle button;
    GUIElemStyle toggle;
    GUIElemStyle slider;
    GUIElemStyle progressbar;
    GUIElemStyle scrollbar;
    GUIElemStyle textbox;
    GUIElemStyle textfield;
    GUIElemStyle combobox;
    GUIElemStyle menu;
    GUIElemStyle tooltip;
} gui_styles;

static struct GUISystem {
    GUITextCursorStyle style;   /*  */
    List window_stack;          /* Stack determining the order of how windows are layered */
    HashMap window_map;         /* String hashmap of all windows */
    HashMap elem_map;           /* String hashmap of all elements */
    GUIElem *hover;             /* Element which the mouse is currently hovering over */
    GUIElem *focus;             /* Element which received the last mouse event */
    Material material;          /* Default GUI material */
    Font font;                  /* Default GUI font */
} gui;

/* TODO: Move this data inside elements */
static struct GUITextCursor {
    size_t pos;         /* Position of the text cursor's current codepoint */
    size_t start;       /* Position of the first codepoint in selection */
    size_t end;         /* Position of the last codepoint in selection */
    u32 threshold;      /* Number of ticks required to toggle visibility (0 = disable blink) */
    u32 ticks;          /* Holds the number of milliseconds */
    int visible;        /* Set when the text cursor should be drawn */
    IQuad quad;         /* Visual coverage in screen-space of the cursor */
    int row_num;        /* Row in which the cursor is located */
    int moved;
} text_cursor;

ATRA_INLINE void atra_gui_init_none(void);
ATRA_INLINE void atra_gui_init_panel(void);
ATRA_INLINE void atra_gui_init_label(void);
ATRA_INLINE void atra_gui_init_button(void);
ATRA_INLINE void atra_gui_init_toggle(void);
ATRA_INLINE void atra_gui_init_slider(void);
ATRA_INLINE void atra_gui_init_progressbar(void);
ATRA_INLINE void atra_gui_init_scrollbar(void);
ATRA_INLINE void atra_gui_init_textbox(void);
ATRA_INLINE void atra_gui_init_textfield(void);
ATRA_INLINE void atra_gui_init_combobox(void);
ATRA_INLINE void atra_gui_init_menu(void);
ATRA_INLINE void atra_gui_init_tooltip(void);

/* Configures an element's base style */
ATRA_INLINE void atra_gui_configbasestyle(GUIBaseStyle *style, Color normal, Color hover,
        Color pressed, Color disabled, Color border_color, int border_size,
        int *padding, int *margin, int rounding);

/* Moves the given window to the top of the window stack */
ATRA_INLINE void atra_gui_window_movetotop(GUIWindow *window);

/* Returns 1 if the given element intersects with the mouse cursor.
 * Mouse coordinates must be normalised to screen-space. */
ATRA_INLINE int atra_gui_elem_mouseintersects(GUIElem *elem, float mx, float my);

/* Returns a pointer to the topmost element which the mouse should interact with. */
ATRA_INLINE GUIElem * atra_gui_findmousehover(int mx, int my);

/* Checks if a callback is non-null, and calls it */
ATRA_INLINE void atra_gui_elem_callback(GUIElem *elem, int callback_type, int arg);

/* Calculates the dimensional ratio of a GUI element */
ATRA_INLINE void atra_gui_elem_calcratio(GUIElem *elem);

/* Removes element text which is currently selected */
ATRA_INLINE void atra_gui_elem_removetextselection(GUIElem *elem);

/* Inserts given text string at cursor position */
ATRA_INLINE void atra_gui_elem_inserttext(GUIElem *elem, const char *str);

/* Toggles the visibility of the text cursor */
ATRA_INLINE void atra_gui_text_togglecursorblink(void);

/* Resets the text cursor to be visible */
ATRA_INLINE void atra_gui_text_resetcursorblink(void);

/* Text-cursor functions for special key functions */
ATRA_INLINE void atra_gui_text_home(GUIElem *elem);
ATRA_INLINE void atra_gui_text_end(GUIElem *elem);
ATRA_INLINE void atra_gui_text_delete(GUIElem *elem);
ATRA_INLINE void atra_gui_text_backspace(GUIElem *elem);
ATRA_INLINE void atra_gui_text_pageup(GUIElem *elem);
ATRA_INLINE void atra_gui_text_pagedown(GUIElem *elem);
ATRA_INLINE void atra_gui_text_enter(GUIElem *elem);
ATRA_INLINE void atra_gui_text_up(GUIElem *elem);
ATRA_INLINE void atra_gui_text_down(GUIElem *elem);
ATRA_INLINE void atra_gui_text_left(GUIElem *elem);
ATRA_INLINE void atra_gui_text_right(GUIElem *elem);


static const char gui_vert_shader[] =
#if defined(ATRA_OPENGL_VERSION_21)
    "#version 120                       \n"
    "attribute vec3 vert_pos;           \n"
    "attribute vec2 vert_uv;            \n"
    "attribute vec4 vert_color;         \n"
    "attribute vec2 vert_uv2;           \n"
    "varying vec2 frag_uv;              \n"
    "varying vec4 frag_color;           \n"
    "varying float frag_white;          \n"
#elif defined(ATRA_OPENGL_VERSION_33)
    "#version 330                       \n"
    "in vec3 vert_pos;                  \n"
    "in vec2 vert_uv;                   \n"
    "in vec4 vert_color;                \n"
    "in vec2 vert_uv2;                  \n" 
    "out vec2 frag_uv;                  \n"
    "out vec4 frag_color;               \n"
    "out float frag_white;              \n"
#endif
    "uniform mat4 viewmat;              \n"
    "uniform mat4 projmat;              \n"
    "uniform mat4 modelmat;             \n"
    "uniform mat4 mvp;				    \n"
	"uniform ivec2 screen_res;          \n"
    "void main()                        \n"
    "{                                  \n"
    "    frag_uv = vert_uv;             \n"
    "    frag_color = vert_color;       \n"
    "    frag_white = vert_uv2[0];      \n"
	"	 vec4 snap_pix = mvp * vec4(vert_pos, 1.0);\n"
//	"	 vec4 clip_pos = snap_pix;		\n"
//	"	 clip_pos.xyz = snap_pix.xyz / snap_pix.w;\n"
//	"	 clip_pos.x = floor(screen_res.x * clip_pos.x) / screen_res.x;\n"
//	"	 clip_pos.y = floor(screen_res.y * clip_pos.y) / screen_res.y;\n"
//	"	 clip_pos.xyz = clip_pos.xyz * snap_pix.w;\n"
    "    gl_Position = snap_pix; \n"
    "}                                  \n";

static const char *gui_frag_shader =
#if defined(ATRA_OPENGL_VERSION_21)
    "#version 120                       \n"
    "varying vec2 frag_uv;              \n"
    "varying vec4 frag_color;           \n"
    "varying float frag_white;          \n"
    "uniform sampler2D albedo_map;      \n"
    "const vec4 white_color = vec4(1.0, 1.0, 1.0, 1.0); \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 tex_color = texture2D(albedo_map, frag_uv); \n"
    //"    tex_color = white_color; \n"
    "    tex_color.rgb = tex_color.rgb + (white_color.rgb - tex_color.rgb) * frag_white; \n"
    "    gl_FragColor = tex_color * frag_color;   \n"
    "}                                  \n";
#elif defined(ATRA_OPENGL_VERSION_33)
    "#version 330                       \n"
    "in vec2 frag_uv;                   \n"
    "in vec4 frag_color;                \n"
    "in float frag_white;               \n"
    "out vec4 out_color;                \n"
    "uniform sampler2D albedo_map;      \n"
    "const vec4 white_color = vec4(1.0, 1.0, 1.0, 1.0); \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 tex_color = texture(texture0, fragTexCoord);   \n"
    "    tex_color = tex_color + (white_color - tex_color) * frag_white; \n"
    "    out_color = tex_color * fragColor;        \n"
    "}                                  \n";
#endif

/*
 *  Auto-alignment 
 */

/*
 *  General Module Functions
 */

#define SET_STYLE_COLOR(entry, r, g, b) \
        entry.type = ATRA_GUI_STYLE_COLOR; \
        entry.data.color = (Color){r, g, b, 0xFF}

#define SET_STYLE_COLOR_V(entry, c) \
        entry.type = ATRA_GUI_STYLE_COLOR; \
        entry.data.color = c

#define SET_COLOR(var, r, g, b) \
        var = (Color){r, g, b, 0xFF}

#define SET_VEC2(vec, x, y) \
        vec[0] = x; \
        vec[1] = y

/* Initialise the GUI subsystem */
ATRA_API
void
atra_gui_init(void)
{
    gui.window_stack = atra_list_create(sizeof(GUIWindow *));
    gui.window_map = atra_hashmap_create(sizeof(char *), sizeof(GUIWindow *),
            atra_hashmap_str_hash,
            atra_hashmap_str_eq);
    gui.elem_map = atra_hashmap_create(sizeof(char *), sizeof(GUIElem *),
            atra_hashmap_str_hash,
            atra_hashmap_str_eq);
    gui.font = atra_font_loadfile(ATRA_GUI_DEFAULT_FONT_PATH,
        ATRA_GUI_DEFAULT_FONT_SIZE, NULL, ATRA_FONT_DEFAULT_GLYPHCOUNT);

    gui.material.shader = atra_shader_loadbuffer(gui_vert_shader, gui_frag_shader);
    gui.material.maps[ATRA_TEXTURE_ALBEDO].tex.id = atra_gl_tex_getdefaultid();
    gui.material.maps[ATRA_TEXTURE_ALBEDO].tex.width = 1;
    gui.material.maps[ATRA_TEXTURE_ALBEDO].tex.height = 1; 
    gui.material.maps[ATRA_TEXTURE_ALBEDO].tex.mipmaps = 1;
    gui.material.maps[ATRA_TEXTURE_ALBEDO].tex.base_sample = ATRA_SAMPLE_NEAREST;
    gui.material.maps[ATRA_TEXTURE_ALBEDO].tex.mip_sample = ATRA_SAMPLE_NEAREST;

    /* Configure default text style */
    SET_COLOR(gui_default_elem_style.text.normal, 0xE0, 0xE0, 0xE0);
    SET_COLOR(gui_default_elem_style.text.hover, 0xE0, 0xE0, 0xE0);
    SET_COLOR(gui_default_elem_style.text.pressed, 0xE0, 0xE0, 0xE0);
    SET_COLOR(gui_default_elem_style.text.disabled, 0xE0, 0xE0, 0xE0);
    gui_default_elem_style.text.size = 16.0f;
    gui_default_elem_style.text.spacing = 1.5f;
    gui_default_elem_style.text.font = &gui.font;

    /* Configure default selection sub-style */
    SET_COLOR(gui_default_elem_style.selection.background, 0x20, 0x70, 0xA0);
    SET_COLOR(gui_default_elem_style.selection.foreground, 0xE0, 0xE0, 0xE0);

    const Color BLACK_GREY = {0x20, 0x20, 0x20, 0xFF};
    const Color DARK_GREY = {0x30, 0x30, 0x30, 0xFF};
    const Color MID_GREY = {0x40, 0x40, 0x40, 0xFF};
    const Color LIGHT_GREY = {0x60, 0x60, 0x60, 0xFF};
    //const Color WHITE_GREY = {0x80, 0x80, 0x80, 0xFF};
    const Color WHITE = {0xE0, 0xE0, 0xE0, 0xFF};

    //const Color BLUE = {0x20, 0x70, 0xA0};
    const Color LIGHT_BLUE = {0x20, 0x90, 0xB0, 0xFF};

    ivec2 NO_SPACE = {0, 0};
    ivec2 LOW_SPACE = {2, 2};
    //ivec2 MID_SPACE = {4, 4};
    //ivec2 BIG_SPACE = {8, 8};

    /* Configure default text cursor sub-style */
    gui_default_elem_style.cursor.shape = ATRA_GUI_CURSOR_BAR;
    gui_default_elem_style.cursor.blink_rate = 500;
    gui_default_elem_style.cursor.bg_normal = DARK_GREY;
    gui_default_elem_style.cursor.bg_hover = DARK_GREY;
    gui_default_elem_style.cursor.fg_normal = WHITE;
    gui_default_elem_style.cursor.fg_hover = WHITE;

    /* Configure default scrollbar style (multi-element) */
    atra_gui_configbasestyle(&gui_default_elem_style.scrollbar.body,
        BLACK_GREY, BLACK_GREY, BLACK_GREY, BLACK_GREY,
        BLACK_GREY, 0, LOW_SPACE, NO_SPACE, 4);

    atra_gui_configbasestyle(&gui_default_elem_style.scrollbar.bar,
        DARK_GREY, MID_GREY, LIGHT_GREY, DARK_GREY,
        BLACK_GREY, 0, LOW_SPACE, NO_SPACE, 4);

    atra_gui_configbasestyle(&gui_default_elem_style.scrollbar.button_inc,
        BLACK_GREY, BLACK_GREY, BLACK_GREY, BLACK_GREY,
        BLACK_GREY, 0, LOW_SPACE, NO_SPACE, 0);

    atra_gui_configbasestyle(&gui_default_elem_style.scrollbar.button_dec,
        BLACK_GREY, BLACK_GREY, BLACK_GREY, BLACK_GREY,
        BLACK_GREY, 0, LOW_SPACE, NO_SPACE, 0);

    gui_default_elem_style.scrollbar.has_buttons = 0;

    /* Configure default slider style (multi-element) */
    atra_gui_configbasestyle(&gui_default_elem_style.slider.body,
        BLACK_GREY, BLACK_GREY, BLACK_GREY, BLACK_GREY,
        BLACK_GREY, 0, NO_SPACE, NO_SPACE, 4);

    atra_gui_configbasestyle(&gui_default_elem_style.slider.fill,
        LIGHT_BLUE, LIGHT_BLUE, LIGHT_BLUE, LIGHT_BLUE,
        BLACK_GREY, 0, NO_SPACE, NO_SPACE, 4);

    atra_gui_configbasestyle(&gui_default_elem_style.slider.bar,
        LIGHT_GREY, LIGHT_GREY, LIGHT_GREY, LIGHT_GREY,
        LIGHT_GREY, 0, NO_SPACE, NO_SPACE, 4);

    

    /* TODO: Configure default progress bar style (multi-element) */

    /* TODO: Configure default combo box style (multi-element) */

    /* TODO: Configure default menu style (multi-element) */


    text_cursor.threshold = 500;
    text_cursor.ticks = 0;
    text_cursor.visible = 1;

    atra_gui_init_none();
    atra_gui_init_panel();
    atra_gui_init_label();
    atra_gui_init_button();
    atra_gui_init_toggle();
    atra_gui_init_slider();
    atra_gui_init_progressbar();
    atra_gui_init_scrollbar();
    atra_gui_init_textbox();
    atra_gui_init_textfield();
    atra_gui_init_combobox();
    atra_gui_init_menu();
    atra_gui_init_tooltip();
}

ATRA_INLINE
void
atra_gui_configbasestyle(GUIBaseStyle *style, Color normal, Color hover, Color pressed,
        Color disabled, Color border_color, int border_size,
        int *padding, int *margin, int rounding)
{
    SET_STYLE_COLOR_V(style->normal, normal);
    SET_STYLE_COLOR_V(style->hover, hover);
    SET_STYLE_COLOR_V(style->pressed, pressed);
    SET_STYLE_COLOR_V(style->disabled, disabled);
    style->border.color = border_color;
    style->border.size = border_size;
    SET_VEC2(style->padding, padding[0], padding[1]);
    SET_VEC2(style->margin, margin[0], margin[1]);
    style->rounding = rounding;
}

ATRA_INLINE
void
atra_gui_init_none(void)
{
    SET_STYLE_COLOR(gui_base_defaults.none.normal, 0x30, 0x30, 0x30);
    SET_STYLE_COLOR(gui_base_defaults.none.hover, 0x30, 0x30, 0x30);
    SET_STYLE_COLOR(gui_base_defaults.none.pressed, 0x30, 0x30, 0x30);
    SET_STYLE_COLOR(gui_base_defaults.none.disabled, 0x30, 0x30, 0x30);
    SET_COLOR(gui_base_defaults.none.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.none.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.none.padding, 0.0f, 0.0f);
    SET_VEC2(gui_base_defaults.none.margin, 4.0f, 4.0f);
    gui_base_defaults.none.rounding = 4.0f;

    gui_styles.none.base = &gui_base_defaults.none;
    gui_styles.none.text = &gui_default_elem_style.text; 
    gui_styles.none.edit = &gui_default_elem_style.edit;
    gui_styles.none.selection = &gui_default_elem_style.selection;
    gui_styles.none.cursor = &gui_default_elem_style.cursor;
    gui_styles.none.scrollbar = &gui_default_elem_style.scrollbar;
}

ATRA_INLINE
void
atra_gui_init_panel(void)
{
    SET_STYLE_COLOR(gui_base_defaults.panel.normal, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.panel.hover, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.panel.pressed, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.panel.disabled, 0x40, 0x40, 0x40);
    SET_COLOR(gui_base_defaults.panel.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.panel.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.panel.padding, 0.0f, 0.0f);
    SET_VEC2(gui_base_defaults.panel.margin, 4.0f, 4.0f);
    gui_base_defaults.panel.rounding = 4.0f;

    gui_styles.panel.base = &gui_base_defaults.panel;
    gui_styles.panel.text = &gui_default_elem_style.text; 
    gui_styles.panel.edit = &gui_default_elem_style.edit;
    gui_styles.panel.selection = &gui_default_elem_style.selection;
    gui_styles.panel.cursor = &gui_default_elem_style.cursor;
    gui_styles.panel.scrollbar = &gui_default_elem_style.scrollbar;
}

ATRA_INLINE
void
atra_gui_init_label(void)
{
    SET_STYLE_COLOR(gui_base_defaults.label.normal, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.label.hover, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.label.pressed, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.label.disabled, 0x40, 0x40, 0x40);
    SET_COLOR(gui_base_defaults.label.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.label.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.label.padding, 0.0f, 0.0f);
    SET_VEC2(gui_base_defaults.label.margin, 4.0f, 4.0f);
    gui_base_defaults.label.rounding = 4.0f;

    gui_styles.label.base = &gui_base_defaults.label;
    gui_styles.label.text = &gui_default_elem_style.text; 
    gui_styles.label.edit = &gui_default_elem_style.edit;
    gui_styles.label.selection = &gui_default_elem_style.selection;
    gui_styles.label.cursor = &gui_default_elem_style.cursor;
    gui_styles.label.scrollbar = &gui_default_elem_style.scrollbar;
}

ATRA_INLINE
void
atra_gui_init_button(void)
{
    SET_STYLE_COLOR(gui_base_defaults.button.normal, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.button.hover, 0x80, 0x80, 0x80);
    SET_STYLE_COLOR(gui_base_defaults.button.pressed, 0xC0, 0xC0, 0xC0);
    SET_STYLE_COLOR(gui_base_defaults.button.disabled, 0x20, 0x20, 0x20);
    SET_COLOR(gui_base_defaults.button.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.button.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.button.padding, 0.0f, 0.0f);
    SET_VEC2(gui_base_defaults.button.margin, 4.0f, 4.0f);
    gui_base_defaults.button.rounding = 4.0f;

    gui_styles.button.base = &gui_base_defaults.button;
    gui_styles.button.text = &gui_default_elem_style.text; 
    gui_styles.button.edit = &gui_default_elem_style.edit;
    gui_styles.button.selection = &gui_default_elem_style.selection;
    gui_styles.button.cursor = &gui_default_elem_style.cursor;
    gui_styles.button.scrollbar = &gui_default_elem_style.scrollbar;
}

ATRA_INLINE
void
atra_gui_init_toggle(void)
{
    SET_STYLE_COLOR(gui_base_defaults.toggle.normal, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.toggle.hover, 0x80, 0x80, 0x80);
    SET_STYLE_COLOR(gui_base_defaults.toggle.pressed, 0x60, 0x60, 0x60);
    SET_STYLE_COLOR(gui_base_defaults.toggle.disabled, 0x20, 0x20, 0x20);
    SET_COLOR(gui_base_defaults.toggle.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.toggle.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.toggle.padding, 0.0f, 0.0f);
    SET_VEC2(gui_base_defaults.toggle.margin, 4.0f, 4.0f);
    gui_base_defaults.toggle.rounding = 4.0f;

    gui_styles.toggle.base = &gui_base_defaults.toggle;
    gui_styles.toggle.text = &gui_default_elem_style.text; 
    gui_styles.toggle.edit = &gui_default_elem_style.edit;
    gui_styles.toggle.selection = &gui_default_elem_style.selection;
    gui_styles.toggle.cursor = &gui_default_elem_style.cursor;
    gui_styles.toggle.scrollbar = &gui_default_elem_style.scrollbar;
}

ATRA_INLINE
void
atra_gui_init_slider(void)
{
    SET_STYLE_COLOR(gui_base_defaults.slider.normal, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.slider.hover, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.slider.pressed, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.slider.disabled, 0x40, 0x40, 0x40);
    SET_COLOR(gui_base_defaults.slider.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.slider.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.slider.padding, 0.0f, 0.0f);
    SET_VEC2(gui_base_defaults.slider.margin, 4.0f, 4.0f);
    gui_base_defaults.slider.rounding = 4.0f;

    gui_styles.slider.base = &gui_base_defaults.slider;
    gui_styles.slider.text = &gui_default_elem_style.text; 
    gui_styles.slider.edit = &gui_default_elem_style.edit;
    gui_styles.slider.selection = &gui_default_elem_style.selection;
    gui_styles.slider.cursor = &gui_default_elem_style.cursor;
    gui_styles.slider.scrollbar = &gui_default_elem_style.scrollbar;
}

ATRA_INLINE
void
atra_gui_init_progressbar(void)
{
    SET_STYLE_COLOR(gui_base_defaults.progressbar.normal, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.progressbar.hover, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.progressbar.pressed, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.progressbar.disabled, 0x40, 0x40, 0x40);
    SET_COLOR(gui_base_defaults.progressbar.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.progressbar.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.progressbar.padding, 0.0f, 0.0f);
    SET_VEC2(gui_base_defaults.progressbar.margin, 4.0f, 4.0f);
    gui_base_defaults.progressbar.rounding = 4.0f;

    gui_styles.progressbar.base = &gui_base_defaults.progressbar;
    gui_styles.progressbar.text = &gui_default_elem_style.text; 
    gui_styles.progressbar.edit = &gui_default_elem_style.edit;
    gui_styles.progressbar.selection = &gui_default_elem_style.selection;
    gui_styles.progressbar.cursor = &gui_default_elem_style.cursor;
    gui_styles.progressbar.scrollbar = &gui_default_elem_style.scrollbar;
}

ATRA_INLINE
void
atra_gui_init_scrollbar(void)
{
    SET_STYLE_COLOR(gui_base_defaults.scrollbar.normal, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.scrollbar.hover, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.scrollbar.pressed, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.scrollbar.disabled, 0x40, 0x40, 0x40);
    SET_COLOR(gui_base_defaults.scrollbar.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.scrollbar.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.scrollbar.padding, 0.0f, 0.0f);
    SET_VEC2(gui_base_defaults.scrollbar.margin, 4.0f, 4.0f);
    gui_base_defaults.scrollbar.rounding = 4.0f;

    gui_styles.scrollbar.base = &gui_base_defaults.scrollbar;
    gui_styles.scrollbar.text = &gui_default_elem_style.text; 
    gui_styles.scrollbar.edit = &gui_default_elem_style.edit;
    gui_styles.scrollbar.selection = &gui_default_elem_style.selection;
    gui_styles.scrollbar.cursor = &gui_default_elem_style.cursor;
    gui_styles.scrollbar.scrollbar = &gui_default_elem_style.scrollbar;
}

ATRA_INLINE
void
atra_gui_init_textbox(void)
{
    SET_STYLE_COLOR(gui_base_defaults.textbox.normal, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.textbox.hover, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.textbox.pressed, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.textbox.disabled, 0x40, 0x40, 0x40);
    SET_COLOR(gui_base_defaults.textbox.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.textbox.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.textbox.padding, 4.0f, 4.0f);
    SET_VEC2(gui_base_defaults.textbox.margin, 4.0f, 4.0f);
    gui_base_defaults.textbox.rounding = 4.0f;

    gui_styles.textbox.base = &gui_base_defaults.textbox;
    gui_styles.textbox.text = &gui_default_elem_style.text; 
    gui_styles.textbox.edit = &gui_default_elem_style.edit;
    gui_styles.textbox.selection = &gui_default_elem_style.selection;
    gui_styles.textbox.cursor = &gui_default_elem_style.cursor;
    gui_styles.textbox.scrollbar = &gui_default_elem_style.scrollbar;
}

ATRA_INLINE
void
atra_gui_init_textfield(void)
{
    SET_STYLE_COLOR(gui_base_defaults.textfield.normal, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.textfield.hover, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.textfield.pressed, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.textfield.disabled, 0x40, 0x40, 0x40);
    SET_COLOR(gui_base_defaults.textfield.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.textfield.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.textfield.padding, 4.0f, 4.0f);
    SET_VEC2(gui_base_defaults.textfield.margin, 4.0f, 4.0f);
    gui_base_defaults.textfield.rounding = 4.0f;

    gui_styles.textfield.base = &gui_base_defaults.textfield;
    gui_styles.textfield.text = &gui_default_elem_style.text; 
    gui_styles.textfield.edit = &gui_default_elem_style.edit;
    gui_styles.textfield.selection = &gui_default_elem_style.selection;
    gui_styles.textfield.cursor = &gui_default_elem_style.cursor;
    gui_styles.textfield.scrollbar = &gui_default_elem_style.scrollbar;
}

ATRA_INLINE
void atra_gui_init_combobox(void)
{
    SET_STYLE_COLOR(gui_base_defaults.combobox.normal, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.combobox.hover, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.combobox.pressed, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.combobox.disabled, 0x40, 0x40, 0x40);
    SET_COLOR(gui_base_defaults.combobox.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.combobox.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.combobox.padding, 0.0f, 0.0f);
    SET_VEC2(gui_base_defaults.combobox.margin, 4.0f, 4.0f);
    gui_base_defaults.combobox.rounding = 4.0f;

    gui_styles.combobox.base = &gui_base_defaults.combobox;
    gui_styles.combobox.text = &gui_default_elem_style.text; 
    gui_styles.combobox.edit = &gui_default_elem_style.edit;
    gui_styles.combobox.selection = &gui_default_elem_style.selection;
    gui_styles.combobox.cursor = &gui_default_elem_style.cursor;
    gui_styles.combobox.scrollbar = &gui_default_elem_style.scrollbar;
}

ATRA_INLINE
void
atra_gui_init_menu(void)
{
    SET_STYLE_COLOR(gui_base_defaults.menu.normal, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.menu.hover, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.menu.pressed, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.menu.disabled, 0x40, 0x40, 0x40);
    SET_COLOR(gui_base_defaults.menu.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.menu.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.menu.padding, 0.0f, 0.0f);
    SET_VEC2(gui_base_defaults.menu.margin, 4.0f, 4.0f);
    gui_base_defaults.menu.rounding = 4.0f;

    gui_styles.menu.base = &gui_base_defaults.menu;
    gui_styles.menu.text = &gui_default_elem_style.text; 
    gui_styles.menu.edit = &gui_default_elem_style.edit;
    gui_styles.menu.selection = &gui_default_elem_style.selection;
    gui_styles.menu.cursor = &gui_default_elem_style.cursor;
    gui_styles.menu.scrollbar = &gui_default_elem_style.scrollbar;
}

ATRA_INLINE
void
atra_gui_init_tooltip(void)
{
    SET_STYLE_COLOR(gui_base_defaults.tooltip.normal, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.tooltip.hover, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.tooltip.pressed, 0x40, 0x40, 0x40);
    SET_STYLE_COLOR(gui_base_defaults.tooltip.disabled, 0x40, 0x40, 0x40);
    SET_COLOR(gui_base_defaults.tooltip.border.color, 0x80, 0x80, 0x80);
    gui_base_defaults.tooltip.border.size = 2.0f;
    SET_VEC2(gui_base_defaults.tooltip.padding, 0.0f, 0.0f);
    SET_VEC2(gui_base_defaults.tooltip.margin, 4.0f, 4.0f);
    gui_base_defaults.tooltip.rounding = 4.0f;

    gui_styles.tooltip.base = &gui_base_defaults.tooltip;
    gui_styles.tooltip.text = &gui_default_elem_style.text; 
    gui_styles.tooltip.edit = &gui_default_elem_style.edit;
    gui_styles.tooltip.selection = &gui_default_elem_style.selection;
    gui_styles.tooltip.cursor = &gui_default_elem_style.cursor;
    gui_styles.tooltip.scrollbar = &gui_default_elem_style.scrollbar;
}


/* Shutdown the GUI subsystem */
ATRA_API
void
atra_gui_stop(void)
{
    const uint list_size = atra_list_length(&gui.window_stack);

    GUIWindow *window;
    uint i;

    for (i = 0; i < list_size; i++) {
        window = *(GUIWindow **) atra_list_get(&gui.window_stack, i);
        atra_gui_window_delete(window);
    }
    atra_shader_destroy(&gui.material.shader);
    atra_font_delete(&gui.font);
    atra_list_delete(&gui.window_stack);
    atra_hashmap_delete(&gui.window_map, MAP_DELFLAG_WINDOW);
    atra_hashmap_delete(&gui.elem_map, MAP_DELFLAG_ELEM);
}

/* Handles any text-related keyboard events for a GUI element */
ATRA_INLINE
void
atra_gui_elem_handletextinput(GUIElem *elem)
{
    i32 keypress;
    
    while ((keypress = atra_key_getpressed())) {
        /* Handle deletion keys */
        switch (keypress) {
        case ATRA_KEY_HOME:
            /* Find beginning of line and set cursor position to that */
            atra_log_print(LOG_INFO, "Detected key: HOME");
            atra_gui_text_home(elem);
            atra_gui_text_resetcursorblink();
            break;
        case ATRA_KEY_END:
            /* Find end of line and set cursor position to that */
            atra_log_print(LOG_INFO, "Detected key: END");
            atra_gui_text_end(elem);
            atra_gui_text_resetcursorblink();
            break;
        case ATRA_KEY_DELETE:
            /* Delete selection, if any, otherwise delete character at current position */
            /* Does nothing at end of text if no selection */
            atra_log_print(LOG_INFO, "Detected key: DELETE");
            atra_gui_text_delete(elem);
            atra_gui_text_resetcursorblink();
            break;
        case ATRA_KEY_INSERT:
            /* DO NOT IMPLEMENT */
            break;
        case ATRA_KEY_BACKSPACE:
            /* Delete selection, if any, otherwise delete character before current position */
            /* Does nothing at beginning of text if no selection */
            atra_log_print(LOG_INFO, "Detected key: BACKSPACE");
            atra_gui_text_backspace(elem);
            atra_gui_text_resetcursorblink();
            break;
        case ATRA_KEY_PAGEUP:
            /* Find the size of a "page" and subtract that from cursor pos */
            atra_log_print(LOG_INFO, "Detected key: PAGEUP");
            atra_gui_text_pageup(elem);
            atra_gui_text_resetcursorblink();
            break;
        case ATRA_KEY_PAGEDOWN:
            /* Find the size of a "page" and add that to cursor pos */
            atra_log_print(LOG_INFO, "Detected key: PAGEDOWN");
            atra_gui_text_pagedown(elem);
            atra_gui_text_resetcursorblink();
            break;
        case ATRA_KEY_RETURN:
            /* Insert newline character (text-input may already handle this) */
            atra_log_print(LOG_INFO, "Detected key: RETURN");
            atra_gui_text_enter(elem);
            atra_gui_text_resetcursorblink();
            break;
        case ATRA_KEY_UP:
            atra_log_print(LOG_INFO, "Detected key: UP");
            atra_gui_text_up(elem);
            atra_gui_text_resetcursorblink();
            break;
        case ATRA_KEY_DOWN:
            atra_log_print(LOG_INFO, "Detected key: DOWN");
            atra_gui_text_down(elem);
            atra_gui_text_resetcursorblink();
            break;
        case ATRA_KEY_LEFT:
            atra_log_print(LOG_INFO, "Detected key: LEFT");
            atra_gui_text_left(elem);
            atra_gui_text_resetcursorblink();
            break;
        case ATRA_KEY_RIGHT:
            atra_log_print(LOG_INFO, "Detected key: RIGHT");
            atra_gui_text_right(elem);
            atra_gui_text_resetcursorblink();
            break;
        }
    }

    /* TODO: Handle special modifier key combo functions (copy, cut, paste) */

}

/* Handles user input events on the GUI elements.
 * Returns true if a mouse event was captured by a window, otherwise false. */
ATRA_API
int
atra_gui_handle(void)
{
    /* TODO: Add style or flag for elements which hold their PRESS state (tickbox, etc.) */

    int mx, my;

    static GUIElem *prev_hover = NULL;
    static GUIElem *prev_focus = NULL;

    if (atra_mouse_islocked()) {
        return 0;
    }
    atra_mouse_getpos(&mx, &my);
    gui.hover = atra_gui_findmousehover(mx, my);

    /* Set the current focus */
    if (atra_mouse_ispressed(ATRA_MOUSE_LEFT)) {
        atra_log_print(LOG_INFO, "Mouse: LEFT_BUTTON at (%d, %d)", mx, my);
        gui.focus = gui.hover;

        if (gui.focus) {
            atra_gui_window_movetotop(gui.focus->window);
        }
    } else if (atra_mouse_ispressed(ATRA_MOUSE_RIGHT) ||
            atra_mouse_ispressed(ATRA_MOUSE_MIDDLE) ||
            atra_mouse_ispressed(ATRA_MOUSE_X1) ||
            atra_mouse_ispressed(ATRA_MOUSE_X2)) {
        /* Drop focus if not left-click */
        if (gui.hover != gui.focus) {
            gui.focus = NULL;
        }
    }

    /* Update old element states */
    if (prev_hover != NULL && prev_hover != gui.hover) {
        atra_gui_elem_callback(prev_hover, ATRA_GUI_EVENT_MOUSELEAVE, 0);

        if (prev_hover != gui.focus) {
            prev_hover->state = ATRA_GUI_ELEM_NORMAL;
        } else if (atra_mouse_isdown(ATRA_MOUSE_LEFT) == 0) {
            prev_hover->state = ATRA_GUI_ELEM_NORMAL;
        }
    }

    if (gui.hover != NULL && gui.hover != prev_hover) {
        atra_gui_elem_callback(gui.hover, ATRA_GUI_EVENT_MOUSEOVER, 0);
    }

    if (prev_focus != NULL && prev_focus != gui.focus) {
        if (prev_focus == gui.hover) {
            /* Set to hover mode */
            prev_focus->state = ATRA_GUI_ELEM_HOVER;
        } else {
            /* Reset to normal mode */
            prev_focus->state = ATRA_GUI_ELEM_NORMAL;
        }
        atra_gui_elem_callback(prev_focus, ATRA_GUI_EVENT_LOSEFOCUS, 0);
    }
    
    if (gui.focus != NULL && gui.focus != prev_focus) {
        atra_gui_elem_callback(gui.focus, ATRA_GUI_EVENT_GAINFOCUS, 0);
    }

    /* Toggle text mode based on element attribute flags */
    const int curr_hastext = (gui.focus != NULL) && BIT_CHK(gui.focus->attrs, ATRA_GUI_ELEM_TEXTINPUT);
    const int prev_hastext = (prev_focus != NULL) && BIT_CHK(prev_focus->attrs, ATRA_GUI_ELEM_TEXTINPUT);
    
    if (curr_hastext && !prev_hastext) {
        atra_key_settextinputmode(1);
    } else if (!curr_hastext && prev_hastext) {
        atra_key_settextinputmode(0);
    }

    if (curr_hastext) {
        atra_gui_elem_handletextinput(gui.focus);
    }

    /* Flag all candidate windows for remesh */
    if (gui.hover != NULL) {
        const int wheel_move = atra_mouse_getwheelmovement();

        atra_gui_elem_handle(gui.hover);

        if (wheel_move && BIT_CHK(gui.hover->attrs, ATRA_GUI_ELEM_SCROLLABLE)) {
            if (gui.hover->smooth_scroll) {
                gui.hover->scroll_amount_y -= wheel_move * ATRA_GUI_SCROLL_PIXEL_COUNT;
            } else {
                gui.hover->scroll_y -= wheel_move * ATRA_GUI_SCROLL_PIXEL_COUNT;
            }
            gui.hover->scroll_y = MAX(gui.hover->scroll_y, 0);

            if (gui.hover->scroll_y_max.value.pixels > 0) {
                const float padding = gui.hover->style.base->padding[1];
				const float border = BIT_CHK(gui.hover->attrs, ATRA_GUI_ELEM_HASBORDER) ?
						gui.hover->style.base->border.size : 0;
                const float y_offset = (border + padding) * 2;
                const float elem_h = gui.hover->real_quad.h * atra_window_getheight() - y_offset;

                const int max_pix = DECIDE(
                    gui.hover->scroll_y_max.data_type == ATRA_GUI_CONS_PIXEL,
                    gui.hover->scroll_y_max.value.pixels,
                    gui.hover->scroll_y_max.value.percent * elem_h);

                gui.hover->scroll_y = MIN(gui.hover->scroll_y, max_pix);
            }
            atra_log_print(LOG_INFO, "Scroll: %d", gui.hover->scroll_y);
            /* 
                TODO:
                    Update scrollbar element (if any)
                    Update smooth scroll variables with new wheel move amount
            */
            atra_gui_elem_flagremesh(gui.hover);
        }

        if (BIT_CHK(gui.hover->attrs, ATRA_GUI_ELEM_HASTEXT)) {
            atra_mouse_setcursor(ATRA_CURSOR_IBEAM);
        } else {
            atra_mouse_setcursor(ATRA_CURSOR_ARROW);
        }
    }
    if (gui.focus != NULL && gui.focus != gui.hover) {
        atra_gui_elem_handle(gui.focus);
    }
    if (prev_hover != NULL && prev_hover != gui.hover) {
        atra_gui_elem_flagremesh(prev_hover);
    }
    if (prev_focus != NULL && prev_focus != gui.focus) {
        atra_gui_elem_flagremesh(prev_focus);
    }
    prev_hover = gui.hover;
    prev_focus = gui.focus;

    return (gui.focus != NULL);
}

/* Updates GUI elements for the current frame */
ATRA_API
void
atra_gui_update(uint dt)
{
    /*
     * 1. Update animations for each window, altering its transform
     * 2. Run all element update callbacks
     */

    const int stack_size = atra_list_length(&gui.window_stack);

    int i;
    GUIWindow *window;

    for (i = 0; i < stack_size; i++) {
        window = *(GUIWindow **) atra_list_get(&gui.window_stack, i);
        atra_gui_window_update(window, dt);
    }
}

/* Draws all windows, and their elements, in the window stack */
ATRA_API
void
atra_gui_render(void)
{
    /*
     *  1. Remesh all flagged windows
     *  2. Call each window in the stack to draw in reverse stack order
     */

    const size_t stack_size = atra_list_length(&gui.window_stack);

    size_t i;
    GUIWindow *window;

    atra_gl_disabledepthtest();

    /* Mesh any windows which require meshing */
    for (i = 0; i < stack_size; i++) {
        window = *(GUIWindow **) atra_list_get(&gui.window_stack, i);

        if (window->dirty) {
            atra_gui_window_mesh(window);
        }
    }

    /* Draw windows */
    for (i = 0; i < stack_size; i++) {
        window = *(GUIWindow **) atra_list_get(&gui.window_stack, i);
        
        if (BIT_CHK(window->states, ATRA_GUI_WINDOW_HIDDEN) == 0) {
            atra_gui_window_render(window);
        }
    }
    atra_gl_disablescissortest();
    atra_gl_enabledepthtest();
}

/* Signals a remesh on all windows and elements */
ATRA_API
void
atra_gui_fullremesh(void)
{
    const int stack_size = atra_list_length(&gui.window_stack);

    int i;
    GUIWindow *window;

    for (i = 0; i < stack_size; i++) {
        window = *(GUIWindow **) atra_list_get(&gui.window_stack, i);
        window->dirty = 1;
        
        if (window->elem) {
            window->elem->dirty = 1;
        }
    }
}

/* Sets the GUI texture atlas for all elements */
ATRA_API
void
atra_gui_settextureatlas(Texture *tex)
{
	gui.material.maps[ATRA_TEXTURE_ALBEDO].tex = *tex;
	gui.material.maps[ATRA_TEXTURE_ALBEDO].color = (Color){0xFF, 0xFF, 0xFF, 0xFF};
}

/* Toggles the visibility of the text cursor */
ATRA_INLINE
void
atra_gui_text_togglecursorblink(void)
{
    text_cursor.visible = !text_cursor.visible;
    text_cursor.ticks = 0;
}

/* Resets the text cursor to be visible */
ATRA_INLINE
void
atra_gui_text_resetcursorblink(void)
{
    text_cursor.visible = 1;
    text_cursor.ticks = 0;
}


/*
 *  GUI Window Functions
 */

/* Creates a new window, automatically adding it to the window stack.
 * Returns a pointer to the new window. */
ATRA_API
GUIWindow *
atra_gui_window_create(u32 attrib, u32 state, const char *id_str)
{
    GUIWindow *window = calloc(1, sizeof(GUIWindow));
    char *str = window->id_str;
    
    strncpy(window->id_str, id_str, ATRA_GUI_MAX_ID_LEN);
    window->attrs = attrib;
    window->states = state;
    window->dirty = 1;
    window->meshes = atra_list_create(sizeof(Mesh));
    window->materials = atra_list_create(sizeof(Material *));
    window->transforms = atra_list_create(sizeof(mat4));
    window->scissors = atra_list_create(sizeof(IQuad));

    atra_list_add(&gui.window_stack, &window);
    atra_hashmap_add(&gui.window_map, &str, &window);

    return window;
}

/* Deallocates the given window, automatically removing it from the window stack. */
ATRA_API
void
atra_gui_window_delete(GUIWindow *window)
{
    const int list_size = atra_list_length(&gui.window_stack);
    const int mesh_count = atra_list_length(&window->meshes);
    char * const str = window->id_str;

    GUIWindow *list_win;
    int found = 0;
    int i, j;

    for (i = 0; i < list_size; i++) {
        list_win = *(GUIWindow **) atra_list_get(&gui.window_stack, i);
        
        if (list_win == window) {
            found = 1;
            break;
        }
    }

    if (!found) {
        atra_log_print(LOG_WARN, "gui: Unable to delete window: No such window in stack");
        return;
    }

    if (window->elem != NULL) {
        atra_gui_elem_delete(window->elem);
    }

    for (j = 0; j < mesh_count; j++) {
        atra_mesh_delete(atra_list_get(&window->meshes, j));
    }
    atra_list_delete(&window->meshes);
    atra_list_delete(&window->materials);
    atra_list_delete(&window->transforms);
    atra_list_delete(&window->scissors);

    atra_hashmap_remove(&gui.window_map, &str, MAP_DELFLAG_WINDOW);
    atra_list_remove_f(&gui.window_stack, i);
}

/* Runs updates on all of a window's elements */
ATRA_API
void
atra_gui_window_update(GUIWindow *window, uint dt)
{
    if (window->elem) {
        atra_gui_elem_update(window->elem, dt);
    } else {
        atra_log_print(LOG_WARN, "gui: Attempted to update a window with no child element");
    }
}

ATRA_INLINE
void
atra_gui_text_meshselectionquad(float depth, const Color *color,
        float x0, float x1, float y0, float y1)
{
    atra_mesh_vertex3f(x0, y0, depth);
    atra_mesh_vertex3f(x0, y1, depth);
    atra_mesh_vertex3f(x1, y1, depth);
    atra_mesh_vertex3f(x1, y0, depth);

    atra_mesh_texcoord2f(0.0f, 0.0f);
    atra_mesh_texcoord2f(0.0f, 0.0f);
    atra_mesh_texcoord2f(0.0f, 0.0f);
    atra_mesh_texcoord2f(0.0f, 0.0f);

    atra_mesh_colorv((u8 *) color);
    atra_mesh_colorv((u8 *) color);
    atra_mesh_colorv((u8 *) color);
    atra_mesh_colorv((u8 *) color);

    atra_mesh_texcoord22f(1.0f, 1.0f);
    atra_mesh_texcoord22f(1.0f, 1.0f);
    atra_mesh_texcoord22f(1.0f, 1.0f);
    atra_mesh_texcoord22f(1.0f, 1.0f);
}

/* Calculates the text cursor quad and updates element quad scroll in case of
   text cursor */
//ATRA_INLINE
void
atra_gui_text_calccursorquad(GUIElem *elem)
{
    const Font * const font = elem->style.text->font;
    //const u8 * const cursor_color = (u8 *) &elem->style.cursor->bg_normal;

    const float font_size = elem->style.text->size;
    const float font_spacing = elem->style.text->spacing;
    const float font_border = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASBORDER) ? elem->style.base->border.size : 0;
    const float * const font_padding = elem->style.base->padding;

    const float x_offset = font_border + font_padding[0];
    const float y_offset = font_border + font_padding[1];
    
    const float screen_width = atra_window_getwidth();
    const float screen_height = atra_window_getheight();
    
    const float scale = font_size / font->size;
    const float skip_extra = font->size * 0.25f;
    const float row_skip = (font->size + skip_extra) * scale;
    const int code_count = elem->code_count;
    
    const int centre = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_CENTRE);
    const int right = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_RIGHT);
    const int middle = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_MIDDLE);
    const int bottom = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_BOTTOM);

    const int row_count = atra_list_length(&elem->rows);
    const float x_factor = DECIDE(centre, 0.5f, 1.0f);
    const float y_factor = DECIDE(middle, 0.5f, 1.0f);
    const float total_height = (row_count * row_skip) - (skip_extra * scale);
    
    const Quad quad = {
        .x = elem->real_quad.x * screen_width + x_offset - elem->scroll_x,
        .y = elem->real_quad.y * screen_height + y_offset - elem->scroll_y,
        .w = elem->real_quad.w * screen_width - x_offset * 2,
        .h = elem->real_quad.h * screen_height - y_offset * 2
    };

    int index, i, j;
    int curr_code;
    int row_length;
    int found;
    float x0, x1, y0, y1;
    float *code_widths;

    const GUILineText *row;

    code_widths = malloc(code_count * sizeof(float));

    /* Get the number, size, width and index of each codepoint */
    for (i = 0; i < code_count; i++) {
        index = atra_font_getglyphindex(font, elem->codepoints[i]);
        code_widths[i] = font_spacing + scale * 
                DECIDE(font->glyphs[index].advance != 0,
                font->glyphs[index].advance, font->quads[index].w);
    }
    curr_code = 0;
    found = 0;

    /* Get the row which holds the text cursor */
    for (i = 0; i < row_count; i++) {
        row = atra_list_get(&elem->rows, i);
        row_length = row->length;

        if (curr_code + row_length <= (int)text_cursor.pos) {
            curr_code += row_length;
            continue;
        }
        x0 = quad.x;
        y0 = quad.y + (row_skip * i);

        /* Calculate alignment offsets */
        if (total_height < quad.h) {
            y0 += DECIDE(middle || bottom, (quad.h - total_height) * y_factor, 0.0f);
        }
        x0 += DECIDE(centre || right, (quad.w - row->width) * x_factor, 0.0f);
        
        for (j = 0; j < row_length; j++) {
            if (curr_code + j == (int)text_cursor.pos) {
                x1 = x0 + 1;
                x0 -= 1;
                y1 = y0 + font_size;

                text_cursor.quad = (IQuad){x0, y0, x1 - x0, y1 - y0};
                text_cursor.row_num = i;
                found = 1;

                break;
            }
            x0 += code_widths[curr_code + j];
        }

        if (found) {
            break;
        }
    }

    /* Adjust scroll of element if cursor is out of bounds */
    if (text_cursor.moved) {
        x0 = text_cursor.quad.x - elem->scroll_x;
        y0 = text_cursor.quad.y - elem->scroll_y;
        x1 = x0 + text_cursor.quad.w;
        y1 = y0 + text_cursor.quad.h;

        if (x0 < quad.x) {
            elem->scroll_x -= quad.x - x0;
            //text_cursor.quad.x += quad.x - x0;
        } else if (x1 > quad.x + quad.w) {
            elem->scroll_x += x1 - (quad.x + quad.w);
            //text_cursor.quad.x -= x1 - (quad.x + quad.w);
        }

        if (y0 < quad.y) {
            elem->scroll_y -= quad.y - y0;
            //text_cursor.quad.y += quad.y - y0;
        } else if (y1 > quad.y + quad.h) {
            elem->scroll_y += y1 - (quad.y + quad.h);
            //text_cursor.quad.y -= y1 - (quad.y + quad.h);
        }
        elem->scroll_x = MAX(elem->scroll_x, 0);
        elem->scroll_y = MAX(elem->scroll_y, 0);

        if (elem->scroll_x_max.value.pixels > 0) {
            const int max_pix = DECIDE(
                elem->scroll_x_max.data_type == ATRA_GUI_CONS_PIXEL,
                elem->scroll_x_max.value.pixels,
                elem->scroll_x_max.value.percent * quad.w);

            elem->scroll_x = MIN(elem->scroll_x, max_pix);
        }
        
        if (elem->scroll_y_max.value.pixels > 0) {
            const int max_pix = DECIDE(
                elem->scroll_y_max.data_type == ATRA_GUI_CONS_PIXEL,
                elem->scroll_y_max.value.pixels,
                elem->scroll_y_max.value.percent * quad.h);

            elem->scroll_y = MIN(elem->scroll_y, max_pix);
        }
        text_cursor.moved = 0;
        atra_gui_text_calccursorquad(elem);
    }
    free(code_widths);
}


/* Builds a text cursor mesh and pushes it to the window's mesh list */
ATRA_INLINE
void
atra_gui_text_drawtextcursor(GUIWindow *window, GUIElem *elem, IQuad scissor, float *depth)
{
    const u8 * const cursor_color = (u8 *) &elem->style.text->normal;
    //const u8 * const cursor_color = (u8 *) &elem->style.cursor->bg_normal;

    Mesh mesh;
    Material *material = &gui.material;
    mat4 transform;
    
    const float x0 = text_cursor.quad.x;
    const float y0 = text_cursor.quad.y;
    const float x1 = text_cursor.quad.x + text_cursor.quad.w;
    const float y1 = text_cursor.quad.y + text_cursor.quad.h;
  
    glm_mat4_identity(transform);

    atra_mesh_setactive(&mesh);

    mesh = atra_mesh_begin(ATRA_QUADS, 
            ATRA_MESH_BUFFER_POSITION |
            ATRA_MESH_BUFFER_TEXCOORD |
            ATRA_MESH_BUFFER_COLORS |
            ATRA_MESH_BUFFER_TEXCOORD2);
    {
        atra_mesh_vertex3f(x0, y0, *depth);
        atra_mesh_vertex3f(x0, y1, *depth);
        atra_mesh_vertex3f(x1, y1, *depth);
        atra_mesh_vertex3f(x1, y0, *depth);

        atra_mesh_texcoord2f(0.0f, 0.0f);
        atra_mesh_texcoord2f(0.0f, 0.0f);
        atra_mesh_texcoord2f(0.0f, 0.0f);
        atra_mesh_texcoord2f(0.0f, 0.0f);

        atra_mesh_colorv(cursor_color);
        atra_mesh_colorv(cursor_color);
        atra_mesh_colorv(cursor_color);
        atra_mesh_colorv(cursor_color);

        atra_mesh_texcoord22f(1.0f, 1.0f);
        atra_mesh_texcoord22f(1.0f, 1.0f);
        atra_mesh_texcoord22f(1.0f, 1.0f);
        atra_mesh_texcoord22f(1.0f, 1.0f);
    }
    atra_mesh_end(FALSE);

    atra_list_add(&window->meshes, &mesh);
    atra_list_add(&window->materials, &material);
    atra_list_add(&window->transforms, &transform);
    atra_list_add(&window->scissors, &scissor);
    
    *depth += (1.0f / 20000.0f);
}

/* Builds a text selection highlight mesh and pushes it to the window's mesh list */
ATRA_INLINE
void
atra_gui_text_drawtextselection(GUIWindow *window, GUIElem *elem, IQuad scissor, float *depth)
{
    const Font * const font = elem->style.text->font;
    const Color sel_color = elem->style.selection->background;

    const float font_size = elem->style.text->size;
    const float font_spacing = elem->style.text->spacing;
    const float font_border = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASBORDER) ? elem->style.base->border.size : 0;
    const float * const font_padding = elem->style.base->padding;

    const float x_offset = font_border + font_padding[0];
    const float y_offset = font_border + font_padding[1];
    
    const float screen_width = atra_window_getwidth();
    const float screen_height = atra_window_getheight();
    
    const float scale = font_size / font->size;
    const float skip_extra = font->size * 0.25f;
    const float row_skip = (font->size + skip_extra) * scale;
    const int code_count = elem->code_count;
    
    const int centre = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_CENTRE);
    const int right = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_RIGHT);
    const int middle = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_MIDDLE);
    const int bottom = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_BOTTOM);

    const int row_count = atra_list_length(&elem->rows);
    const float x_factor = DECIDE(centre, 0.5f, 1.0f);
    const float y_factor = DECIDE(middle, 0.5f, 1.0f);
    const float total_height = (row_count * row_skip) - (skip_extra * scale);
    
    const Quad quad = {
        .x = elem->real_quad.x * screen_width + x_offset - elem->scroll_x,
        .y = elem->real_quad.y * screen_height + y_offset - elem->scroll_y,
        .w = elem->real_quad.w * screen_width - x_offset * 2,
        .h = elem->real_quad.h * screen_height - y_offset * 2
    };

    int curr_code;
    int row_length;
    int index, i, j;
    float *code_widths;

    int should_draw;
    float x0, x1, y0, y1;

    Mesh mesh;
    Material *material = &gui.material;
    mat4 transform;
    
    const GUILineText *row;

    glm_mat4_identity(transform);

    code_widths = malloc(code_count * sizeof(float));

    mesh = atra_mesh_begin(ATRA_QUADS,
                ATRA_MESH_BUFFER_POSITION |
                ATRA_MESH_BUFFER_TEXCOORD |
                ATRA_MESH_BUFFER_COLORS |
                ATRA_MESH_BUFFER_TEXCOORD2);
        
    atra_mesh_setactive(&mesh);

    /* Get the number, size, width and index of each codepoint */
    for (i = 0; i < code_count; i++) {
        index = atra_font_getglyphindex(font, elem->codepoints[i]);
        code_widths[i] = font_spacing + scale * 
                DECIDE(font->glyphs[index].advance != 0,
                font->glyphs[index].advance, font->quads[index].w);
    }
    should_draw = 0;
    curr_code = 0;

    for (i = 0; i < row_count; i++) {
        row = atra_list_get(&elem->rows, i);
        row_length = row->length;

        if (curr_code + row_length <= (int)text_cursor.start) {
            curr_code += row_length;
            continue;
        }
        x0 = quad.x;
        y0 = quad.y + (row_skip * i);

        /* Calculate alignment offsets */
        if (total_height < quad.h) {
            y0 += DECIDE(middle || bottom, (quad.h - total_height) * y_factor, 0.0f);
        }
        x0 += DECIDE(centre || right, (quad.w - row->width) * x_factor, 0.0f);
        x1 = x0;
        y1 = y0 + font_size;
        
        for (j = 0; j < row_length; j++) {
            
            if (curr_code + j == (int)text_cursor.start) {
                x1 = x0;
                should_draw = 1;
            }
            
            if (should_draw == 0) {
                x0 += code_widths[curr_code + j];
            }

            if (curr_code + j == (int)text_cursor.end) {
                atra_gui_text_meshselectionquad(*depth, &sel_color, x0, x1, y0, y1);
                goto end;
            }
            
            if (should_draw == 1) {
                x1 += code_widths[curr_code + j];
            }
            
            if (j == row_length - 1 && should_draw) {
                atra_gui_text_meshselectionquad(*depth, &sel_color, x0, x1, y0, y1);
            }
        }
        curr_code += row_length;
    }
end:
    *depth += (1.0f / 20000.0f);
    free(code_widths);
    atra_mesh_end(FALSE);

    atra_list_add(&window->meshes, &mesh);
    atra_list_add(&window->materials, &material);
    atra_list_add(&window->transforms, &transform);
    atra_list_add(&window->scissors, &scissor);
}

/* Populates the window's mesh list with data from all contained elements */
ATRA_API
void
atra_gui_window_mesh(GUIWindow *window)
{
    if (window->elem == NULL) {
        atra_log_print(LOG_WARN, "gui: Attempted to mesh a window with no child element");
        return;
    }

    const float screen_width = atra_window_getwidth();
    const float screen_height = atra_window_getheight();
    const int elem_count = atra_gui_elem_treesize(window->elem);

    GUIElem **agenda;
    GUIElem *elem;
    float depth;
    int agenda_size;
    int found_mesh;
    int i, j;

    int mesh_count;
    int mesh_index;

    Mesh *mesh;
    Material *elem_material;
    mat4 elem_transform;
    IQuad elem_scissor;
    int s_x0, s_x1, s_y0, s_y1;

    atra_gui_window_clearmeshdata(window);
    
    agenda = calloc(elem_count, sizeof(GUIElem *));
    mesh_count = 0;
    agenda_size = 1;
    agenda[0] = window->elem;
    depth = -1.0f;

    for (i = 0; i < agenda_size; i++) {
        const int child_count = atra_list_length(&agenda[i]->children);

        /* Add children to the agenda */
        for (j = 0; j < child_count; j++) {
            elem = *(GUIElem **) atra_list_get(&agenda[i]->children, j);

            if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HIDEALL) == 0) {
                agenda[agenda_size + j] = elem;
            }
        }
        agenda_size += child_count;
        elem = agenda[i];


        if (elem->dirty) {
            atra_gui_elem_calcquad(elem);
        }
        
        if (elem->rebuild_rows && BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASTEXT)) {
            atra_gui_elem_calcrowdata(elem);
        }
        
        if (elem == gui.focus && BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASTEXT)) {
            atra_gui_text_calccursorquad(elem);
        }

        if (elem->material) {
            elem_material = elem->material;
        } else {
            elem_material = &gui.material;
        }

        /* Calculate the mesh transform (accumulate all parent tranforms) */
        glm_mat4_identity(elem_transform);

        while (elem) {
            glm_mat4_mul(elem_transform, elem->transform, elem_transform);
            elem = elem->parent;
        }

        /* Calculate the mesh scissor */
        elem = agenda[i]->parent;
        s_x0 = 0;
        s_x1 = 0;
        s_y0 = 0;
        s_y1 = 0;

        while (elem != NULL) {
            if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_SCISSORTEST | ATRA_GUI_ELEM_SCROLLABLE)) {
				const uint border_size = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASBORDER) ? elem->style.base->border.size : 0;
                const int q_x0 = elem->real_quad.x * screen_width + border_size;
                const int q_y0 = elem->real_quad.y * screen_height + border_size;
                const int q_x1 = q_x0 + elem->real_quad.w * screen_width - border_size * 2;
                const int q_y1 = q_y0 + elem->real_quad.h * screen_height - border_size * 2;

                if (s_x1 == 0 || s_y1 == 0) {
                    s_x0 = q_x0; s_x1 = q_x1;
                    s_y0 = q_y0; s_y1 = q_y1;
                } else {
                    s_x0 = MAX(s_x0, q_x0); s_x1 = MIN(s_x1, q_x1);
                    s_y0 = MAX(s_y0, q_y0); s_y1 = MIN(s_y1, q_y1);
                }
            }
            elem = elem->parent;
        }
        found_mesh = 0;
        elem = agenda[i];

        if (s_x1 != 0 || s_y1 != 0) {
            elem_scissor.x = s_x0;
            elem_scissor.y = screen_height - s_y1;
            elem_scissor.w = s_x1 - s_x0;
            elem_scissor.h = s_y1 - s_y0;
        } else {
            elem_scissor.x = 0;
            elem_scissor.y = 0;
            elem_scissor.w = 0;
            elem_scissor.h = 0;
        }
        
        /* Mesh element into selected mesh */
        if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HIDDEN) == 0) {
            const uint border_size = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASBORDER) ? elem->style.base->border.size : 0;

            int x0 = elem->real_quad.x * screen_width;
            int y0 = elem->real_quad.y * screen_height;
            int x1 = x0 + elem->real_quad.w * screen_width;
            int y1 = y0 + elem->real_quad.h * screen_height;

            GUIStyleEntry *style;

            Color color;
            Color border_color;
            vec2 uvs[4];
            int white_level;

            /* Create new mesh if none found */
            if (!found_mesh) {
                Mesh mesh_data;

                /* Uses texcoord2 as a whiteness mask */
                mesh_data = atra_mesh_begin(ATRA_QUADS,
                        ATRA_MESH_BUFFER_POSITION |
                        ATRA_MESH_BUFFER_TEXCOORD |
                        ATRA_MESH_BUFFER_COLORS |
                        ATRA_MESH_BUFFER_TEXCOORD2);

                atra_list_add(&window->meshes, &mesh_data);
                atra_list_add(&window->materials, &elem_material);
                atra_list_add(&window->transforms, &elem_transform);
                atra_list_add(&window->scissors, &elem_scissor);
            
                mesh_index = mesh_count;
                mesh_count++;
                mesh = atra_list_get(&window->meshes, mesh_index);
            }

            switch (elem->state) {
            case ATRA_GUI_ELEM_NORMAL:
                style = &elem->style.base->normal;
                break;
            case ATRA_GUI_ELEM_HOVER:
                style = &elem->style.base->hover;
                break;
            case ATRA_GUI_ELEM_PRESS:
            case ATRA_GUI_ELEM_ACTIVE:
                style = &elem->style.base->pressed;
                break;
            case ATRA_GUI_ELEM_DISABLED:
                style = &elem->style.base->disabled;
                break;
            default:
                style = &elem->style.base->normal;
                break;
            }
            atra_mesh_setactive(mesh);
            border_color = elem->style.base->border.color;
            white_level = 1.0f;

            /* Draw border */
            if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASBORDER)) {
                atra_mesh_vertex3f(x0, y0, depth);
                atra_mesh_vertex3f(x0, y1, depth);
                atra_mesh_vertex3f(x1, y1, depth);
                atra_mesh_vertex3f(x1, y0, depth);

                atra_mesh_texcoord2f(0.0f, 0.0f);
                atra_mesh_texcoord2f(0.0f, 0.0f);
                atra_mesh_texcoord2f(0.0f, 0.0f);
                atra_mesh_texcoord2f(0.0f, 0.0f);

                atra_mesh_colorv((u8 *)&border_color);
                atra_mesh_colorv((u8 *)&border_color);
                atra_mesh_colorv((u8 *)&border_color);
                atra_mesh_colorv((u8 *)&border_color);

                atra_mesh_texcoord22f(white_level, white_level);
                atra_mesh_texcoord22f(white_level, white_level);
                atra_mesh_texcoord22f(white_level, white_level);
                atra_mesh_texcoord22f(white_level, white_level);

                depth += (1.0f / 20000.0f);
            }
            /* Draw mesh body */
            x0 += border_size;
            y0 += border_size;
            x1 -= border_size;
            y1 -= border_size;

            if (style->type == ATRA_GUI_STYLE_IMAGE) {
                memcpy(uvs, style->data.tex_uvs, 4 * sizeof(vec2));
                memset(&color, 0xFF, sizeof(Color));
                white_level = 0.0f;
            } else {
                memcpy(&color, &style->data.color, sizeof(Color));
                memset(uvs, 0x00, 4 * sizeof(vec2));
                white_level = 1.0f;
            }

            atra_mesh_vertex3f(x0, y0, depth);
            atra_mesh_vertex3f(x0, y1, depth);
            atra_mesh_vertex3f(x1, y1, depth);
            atra_mesh_vertex3f(x1, y0, depth);

            atra_mesh_texcoordv(uvs[0]);
            atra_mesh_texcoordv(uvs[1]);
            atra_mesh_texcoordv(uvs[2]);
            atra_mesh_texcoordv(uvs[3]);

            atra_mesh_colorv((u8 *) &color);
            atra_mesh_colorv((u8 *) &color);
            atra_mesh_colorv((u8 *) &color);
            atra_mesh_colorv((u8 *) &color);

            atra_mesh_texcoord22f(white_level, white_level);
            atra_mesh_texcoord22f(white_level, white_level);
            atra_mesh_texcoord22f(white_level, white_level);
            atra_mesh_texcoord22f(white_level, white_level);

            depth += (1.0f / 20000.0f);

            /* TODO: remove this mesh ender if doing mesh grouping again */
            atra_mesh_end(FALSE);

            /* Pop out an extra mesh if text is used */
            /* Be sure to use styles to render the font, cursor and selection */
            if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASTEXT)) {
                Mesh font_mesh;
                Material *font_mat = calloc(1, sizeof(Material));
                Font *font = elem->style.text->font;
                
                if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_SCISSORTEST | ATRA_GUI_ELEM_SCROLLABLE)) {
                    const int r_x0 = elem->real_quad.x * screen_width + border_size;
                    const int r_y0 = elem->real_quad.y * screen_height + border_size;
                    const int r_x1 = r_x0 + elem->real_quad.w * screen_width - border_size * 2;
                    const int r_y1 = r_y0 + elem->real_quad.h * screen_height - border_size * 2;
                    
                    if (s_x1 == 0 || s_y1 == 0) {
                        s_x0 = r_x0;
                        s_x1 = r_x1;
                        s_y0 = r_y0;
                        s_y1 = r_y1;
                    } else {
                        s_x0 = MAX(s_x0, r_x0);
                        s_x1 = MIN(s_x1, r_x1);
                        s_y0 = MAX(s_y0, r_y0);
                        s_y1 = MIN(s_y1, r_y1);
                    }

                    /* TODO: Maybe remove zeroing, since this branch might never
                       actually be triggered
                    */
                    if (s_x1 != 0 || s_y1 != 0) {
                        elem_scissor.x = s_x0;
                        elem_scissor.y = screen_height - s_y1;
                        elem_scissor.w = s_x1 - s_x0;
                        elem_scissor.h = s_y1 - s_y0;
                    } else {
                        elem_scissor.x = 0;
                        elem_scissor.y = 0;
                        elem_scissor.w = 0;
                        elem_scissor.h = 0;
                    }
                }

                /* Draw text selection highlight background quads */
                if (gui.focus == elem && BIT_CHK(elem->attrs, ATRA_GUI_ELEM_TEXTINPUT)) {
                    if (text_cursor.start != text_cursor.end) {
                        atra_gui_text_drawtextselection(window, elem, elem_scissor, &depth);
                        mesh_index = mesh_count;
                        mesh_count++;
                    }

                    if ((int)text_cursor.pos < elem->code_count && text_cursor.visible) {
                        atra_gui_text_drawtextcursor(window, elem, elem_scissor, &depth);
                        mesh_index = mesh_count;
                        mesh_count++;
                    }
                }

                /* TODO: Maybe turn depth into reference so that this function can modify
                        it properly preventing undesireable overlaps */
                atra_gui_elem_drawtext(elem, &font_mesh, depth);
                depth += (1.0f / 20000.0f); // * strlen(elem->text);

                /* TODO: FONT ELEMENT IS NOT FREED */

                font_mat->maps[0].tex = font->texture;
                font_mat->maps[0].color = (Color){0xFF, 0xFF, 0xFF, 0xFF};
                /* TODO: font_mat.maps[1] = International font units */
                font_mat->shader.prog_id = atra_gl_shader_getdefaultid();
                font_mat->shader.locs = atra_gl_shader_getdefaultlocs();

                atra_list_add(&window->meshes, &font_mesh);
                atra_list_add(&window->materials, &font_mat);
                atra_list_add(&window->transforms, &elem_transform);
                atra_list_add(&window->scissors, &elem_scissor);

                mesh_index = mesh_count;
                mesh_count++;

                /* TODO: Free font_mat on material destruction */

                //free(font_mat);
            }
        }
    }

    /* Finalise all meshes */
    /*for (i = 0; i < mesh_count; i++) {
        atra_mesh_setactive(atra_list_get(&window->meshes, i));
        atra_mesh_end(FALSE);
    }*/
    window->dirty = 0;
    free(agenda);
}

/* Deletes all mesh data from the window, leaving the relevant lists empty */
ATRA_API
void
atra_gui_window_clearmeshdata(GUIWindow *window)
{
    const int mesh_count = atra_list_length(&window->meshes);

    int i;
    Mesh *mesh;

    for (i = 0; i < mesh_count; i++) {
        mesh = atra_list_get(&window->meshes, i);
        
        /* Delete material of font meshes (font meshes have no UV2 attribute) */
        if (mesh->uvs2 == NULL) {
            Material **mat = atra_list_get(&window->materials, i);
            free(*mat);
        }
        /* Delete mesh */
        atra_mesh_delete(mesh);
    }
    atra_list_clear(&window->meshes);
    atra_list_clear(&window->materials);
    atra_list_clear(&window->transforms);
    atra_list_clear(&window->scissors);
}

/* Draws the window's meshes to the screen */
ATRA_API
void
atra_gui_window_render(GUIWindow *window)
{
    const int mesh_count = atra_list_length(&window->meshes);
    const IQuad scissor_zero = {0, 0, 0, 0};

    int i;

    Mesh *mesh;
    Material *material;
    mat4 *transform;
    IQuad *scissor;

    for (i = 0; i < mesh_count; i++) {
        mesh = atra_list_get(&window->meshes, i);
        material = *(Material **) atra_list_get(&window->materials, i);
        transform = atra_list_get(&window->transforms, i);
        scissor = atra_list_get(&window->scissors, i);
/*
        if (material == NULL) {
            material = &gui.material;
        }
*/
        if (memcmp(scissor, &scissor_zero, sizeof(IQuad)) == 0) {
            atra_gl_disablescissortest();
        } else {
            atra_gl_enablescissortest();
            atra_gl_scissor(scissor->x, scissor->y, scissor->w, scissor->h);
        }
        atra_mesh_draw(mesh, material, *transform);
    }
}

/* Returns a pointer to a window with a matching ID string.
 * Returns NULL if no such window exists */
ATRA_API
GUIWindow *
atra_gui_window_find(const char *id)
{
    return *(GUIWindow **) atra_hashmap_get(&gui.window_map, &id);
}

/* Inserts an element into the window's base element */
ATRA_API
void
atra_gui_window_setelem(GUIWindow *window, GUIElem *elem)
{
    window->elem = elem;
    elem->window = window;
    atra_gui_elem_flagremesh(elem);
}

/* Sets the window title string */
ATRA_API
void
atra_gui_window_settitle(GUIWindow *window, const char *title)
{
    strncpy(window->title, title, ATRA_GUI_MAX_TITLE_LEN);
    window->dirty = 1;

    if (window->elem) {
        window->elem->dirty = 1;
    }
}

ATRA_INLINE
void
atra_gui_window_movetotop(GUIWindow *window)
{
    const uint stack_size = atra_list_length(&gui.window_stack);

    GUIWindow *curr_win;
    uint i;

    for (i = 0; i < stack_size; i++) {
        curr_win = *(GUIWindow **) atra_list_get(&gui.window_stack, i);

        if (curr_win == window) {
            atra_list_remove(&gui.window_stack, i);
            atra_list_add(&gui.window_stack, &window);
        }
    }
}

/*
 *  GUI Element Functions
 */

/* Creates a new, custom GUI element with no specialised behaviors */
ATRA_API
GUIElem *
atra_gui_elem_create(const char *id, u32 attrib, GUIQuad area)
{
    GUIElem *elem = calloc(1, sizeof(GUIElem));
    char *str = elem->id_str;

    strncpy(elem->id_str, id, ATRA_GUI_MAX_ID_LEN);
    elem->attrs = attrib;
    elem->state = ATRA_GUI_ELEM_NORMAL;
    elem->quad = area;
    elem->style = gui_styles.none;
    elem->ratio = area.w / area.h;
    elem->children = atra_list_create(sizeof(GUIElem *));

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASTEXT)) {
        elem->rows = atra_list_create(sizeof(GUILineText));
    }

    atra_hashmap_add(&gui.elem_map, &str, &elem);
    glm_mat4_identity(elem->transform);
    atra_gui_elem_flagremesh(elem);

    return elem;
}

/* Deletes the given element, along with all its children */
ATRA_API
void
atra_gui_elem_delete(GUIElem *elem)
{
    const size_t list_size = atra_list_length(&elem->children);
    size_t i;
    GUIElem *child;
    char *id_str;

    for (i = 0; i < list_size; i++) {
        child = *(GUIElem **) atra_list_get(&elem->children, i);
        atra_gui_elem_delete(child);
    }
    id_str = elem->id_str;

    atra_list_delete(&elem->children);
    atra_hashmap_remove(&gui.elem_map, &id_str, MAP_DELFLAG_ELEM);

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASTEXT)) {
        atra_list_delete(&elem->rows);
    }

    if (elem->text) {
        free(elem->text);
        free(elem->codepoints);
    }
}

/* Removes element text which is currently selected */
ATRA_INLINE
void
atra_gui_elem_removetextselection(GUIElem *elem)
{
    int start, end;

    start = text_cursor.start;
    end = text_cursor.end;

    if (start == end) {
        return;
    }

    for (; end < elem->code_count; end++, start++) {
        elem->codepoints[start] = elem->codepoints[end];
    }
    elem->code_count -= end - start;

    free(elem->text);
    elem->text = atra_utf_fromcodepointstr(elem->codepoints, elem->code_count);
    elem->rebuild_rows = 1;

    text_cursor.pos = text_cursor.start;
    text_cursor.end = text_cursor.start;
    text_cursor.moved = 1;
}

/* Inserts given text string at cursor position */
ATRA_INLINE
void
atra_gui_elem_inserttext(GUIElem *elem, const char *str)
{
    int code_count;
    int *codepoints;
    int new_codecount;

    atra_log_print(LOG_INFO, "Inserting text: %s", str);

    if (*str == '\0' || str == NULL) {
        return;
    }
    codepoints = atra_utf_tocodepoints(str, &code_count);
    new_codecount = code_count + elem->code_count;
    elem->codepoints = realloc(elem->codepoints, new_codecount * sizeof(int));

    memmove(elem->codepoints + text_cursor.pos + code_count,
            elem->codepoints + text_cursor.pos,
            (elem->code_count - text_cursor.pos) * sizeof(int));

    memcpy(elem->codepoints + text_cursor.pos, codepoints, code_count * sizeof(int));
    
    free(codepoints);
    free(elem->text);

    text_cursor.pos += code_count;
    text_cursor.moved = 1;
    elem->code_count = new_codecount;
    elem->text = atra_utf_fromcodepointstr(elem->codepoints, elem->code_count);
    elem->rebuild_rows = 1;
    atra_gui_elem_flagremesh(elem);
}

/* Determines the codepoint position of the text cursor based on mouse position */
ATRA_INLINE
uint
atra_gui_elem_calctextcursorpos(GUIElem *elem)
{
    const Font * const font = elem->style.text->font;

    const float font_size = elem->style.text->size;
    const float font_spacing = elem->style.text->spacing;
    const float font_border = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASBORDER) ? elem->style.base->border.size : 0;
    const float * const font_padding = elem->style.base->padding;

    const float x_offset = font_border + font_padding[0];
    const float y_offset = font_border + font_padding[1];

    const float screen_width = atra_window_getwidth();
    const float screen_height = atra_window_getheight();
    
    const float scale = font_size / font->size;
    const float skip_extra = font->size * 0.25f;
    const float row_skip = (font->size + skip_extra) * scale;
    const uint code_count = elem->code_count;

    const int centre = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_CENTRE);
    const int right = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_RIGHT);
    const int middle = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_MIDDLE);
    const int bottom = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_BOTTOM);

    const uint row_count = atra_list_length(&elem->rows);
    const float x_factor = DECIDE(centre, 0.5f, 1.0f);
    const float y_factor = DECIDE(middle, 0.5f, 1.0f);
    const float total_height = (row_count * row_skip) - (skip_extra * scale);

    const Quad quad = {
        .x = elem->real_quad.x * screen_width + x_offset - elem->scroll_x,
        .y = elem->real_quad.y * screen_height + y_offset - elem->scroll_y,
        .w = elem->real_quad.w * screen_width - x_offset * 2,
        .h = elem->real_quad.h * screen_height - y_offset * 2
    };

    uint index, i, j;
    int mx, my;
    float x_pos, y_pos;
    
    uint code_pos;
    uint row_length;
    float *code_widths;

    const GUILineText *row;

    atra_mouse_getpos(&mx, &my);

    code_widths = malloc(code_count * sizeof(float));
    code_pos = 0;

    /* Get the number, size, width and index of each codepoint */
    for (i = 0; i < code_count; i++) {
        index = atra_font_getglyphindex(font, elem->codepoints[i]);
        code_widths[i] = font_spacing + scale * 
                DECIDE(font->glyphs[index].advance != 0,
                font->glyphs[index].advance, font->quads[index].w);
    }

    for (i = 0; i < row_count; i++) {
        row = atra_list_get(&elem->rows, i);
        row_length = row->length;
        x_pos = quad.x;
        y_pos = quad.y + (row_skip * i);

        /* Calculate alignment offsets */
        if (total_height < quad.h) {
            y_pos += DECIDE(middle || bottom, (quad.h - total_height) * y_factor, 0.0f);
        }
        x_pos += DECIDE(centre || right, (quad.w - row->width) * x_factor, 0.0f);

        /* Decide which row the mouse is selecting */
        if (my < y_pos + row_skip) {
            /* Found line, now find character */
            for (j = 0; j < row_length; j++) {
                const float code_width = code_widths[row->codes - elem->codepoints + j];

                if (mx < x_pos + code_width) {
                    if (mx > x_pos + code_width * 0.5f) {
                        /* Select next char if codepoint is not at end of line */
                        free(code_widths);
                        return code_pos + j + DECIDE(j < row_length - 1, 1, 0);
                    } else {
                        /* Select current char */
                        free(code_widths);
                        return code_pos + j;
                    }
                } else if (j == row_length - 1) {
                    /* Return last codepoint in row */
                    free(code_widths);
                    return code_pos + row_length - 1;
                }
                x_pos += code_width;
            }
        } else if (i == row_count - 1) {
            /* Final row, force cursor to final codepoint */
            free(code_widths);
            return elem->code_count - 1;
        }
        code_pos += row_length;
    }
    free(code_widths);

    return code_pos;
}

/* Handle user input for the given element */
ATRA_API
int
atra_gui_elem_handle(GUIElem *elem)
{
    /* Change state based on input events */
    int pressed[ATRA_MOUSE_TOTAL];
    int released[ATRA_MOUSE_TOTAL];

    int any_pressed = 0;
    int i;

    for (i = 0; i < ATRA_MOUSE_TOTAL; i++) {
        pressed[i] = atra_mouse_ispressed(i);
        released[i] = atra_mouse_isreleased(i);
        any_pressed = any_pressed || pressed[i];
    }

    if (elem == gui.focus) {
        /* Left mouse button has been pressed down on this element at some point */
        if (elem == gui.hover) {
            /* Mouse is hovering over previously pressed element */
            if (elem->state != ATRA_GUI_ELEM_HOVER) {
                elem->state = ATRA_GUI_ELEM_HOVER;
                atra_gui_elem_flagremesh(elem);
            }

            if (atra_mouse_isdown(ATRA_MOUSE_LEFT) && elem->state != ATRA_GUI_ELEM_PRESS) {
                elem->state = ATRA_GUI_ELEM_PRESS;
                atra_gui_elem_flagremesh(elem);
            }

            for (i = 0; i < ATRA_MOUSE_TOTAL; i++) {
                if (pressed[i]) {
                    atra_gui_elem_callback(elem, ATRA_GUI_EVENT_MOUSEDOWN, i);
                    atra_gui_elem_flagremesh(elem);
                }
                if (released[i]) {
                    /* Trigger mouse full-click event (press and release) */
                    atra_log_print(LOG_INFO, "Mouse full-click event: #%d", i);
                    atra_gui_elem_callback(elem, ATRA_GUI_EVENT_MOUSECLICK, i);
                    atra_gui_elem_flagremesh(elem);
                    elem->state = ATRA_GUI_ELEM_HOVER;
                }
            }
        } else {
            /* Mouse moved away from focus since last left mouse button press */
            if (released[ATRA_MOUSE_LEFT]) {
                elem->state = ATRA_GUI_ELEM_NORMAL;
                atra_gui_elem_flagremesh(elem);
            }
        }

        /* Handle text input if the element is a text field/box */
        if (atra_key_textinputenabled() &&
                BIT_CHK(elem->attrs, ATRA_GUI_ELEM_TEXTINPUT)) {
            /* TODO: Handle text (look at breadboard for help) */
            /* Handle click-drag text selection */
            const char *text;
            static size_t drag_origin;

            /* Clear selected text and insert new text */
            if ((text = atra_key_gettext())) {
                atra_gui_elem_removetextselection(elem);

                /* Actuate text input */
                do {
                    atra_gui_elem_inserttext(elem, text);
                } while ((text = atra_key_gettext()));

                atra_gui_text_resetcursorblink();
                atra_gui_elem_flagremesh(elem);
            }

            /* Initialise selection range */
            if (atra_mouse_ispressed(ATRA_MOUSE_LEFT)) {
                text_cursor.pos = atra_gui_elem_calctextcursorpos(elem);
                text_cursor.start = text_cursor.pos;
                text_cursor.end = text_cursor.pos;
                text_cursor.moved = 1;
                drag_origin = text_cursor.pos;
                atra_gui_text_resetcursorblink();
                atra_gui_elem_flagremesh(elem);
            }

            /* Set cursor position and update current mouse text selection */
            if (atra_mouse_isdown(ATRA_MOUSE_LEFT)) {
                text_cursor.pos = atra_gui_elem_calctextcursorpos(elem);
                /* Use new cursor position to recalculate the start and length of text selection */

                if (text_cursor.pos <= drag_origin) {
                    text_cursor.start = text_cursor.pos;
                    text_cursor.end = drag_origin;
                } else {
                    text_cursor.end = text_cursor.pos;
                    text_cursor.start = drag_origin;
                }
                atra_gui_text_resetcursorblink();
                atra_gui_elem_flagremesh(elem);
                text_cursor.moved = 1;
            }
        }
    } else if (elem == gui.hover) {
        /* Mouse is currently hovering over this element. 
         * As in, mouse button has not been pressed down on this element */

        if (elem->state != ATRA_GUI_ELEM_HOVER) {
            elem->state = ATRA_GUI_ELEM_HOVER;
            atra_gui_elem_flagremesh(elem);
        }
        
        for (i = 0; i < ATRA_MOUSE_TOTAL; i++) {
            if (pressed[i]) {
                atra_gui_elem_callback(elem, ATRA_GUI_EVENT_MOUSEDOWN, i);
            }
            if (released[i]) {
                atra_gui_elem_callback(elem, ATRA_GUI_EVENT_MOUSEUP, i);
            }
        }
    }

    return 0;
}

/* Updates the given element for the current frame */
ATRA_API
void
atra_gui_elem_update(GUIElem *elem, uint dt)
{
    const int child_count = atra_list_length(&elem->children);

    GUIElem *child;
    int scrolled, i;

    /* Execute update function */
    if (elem->update) {
        elem->update(elem, dt);
    }

    if (elem == gui.focus && BIT_CHK(elem->attrs, ATRA_GUI_ELEM_TEXTINPUT)) {
        text_cursor.ticks += dt;
        
        if (text_cursor.ticks >= text_cursor.threshold) {
            atra_gui_text_togglecursorblink();
            atra_gui_elem_flagremesh(elem);
        }
    }

    /* Process smooth scrolling */
    scrolled = 0;

    if (elem->smooth_scroll) {
        if (elem->scroll_amount_x < 0) {
            elem->scroll_x -= ATRA_GUI_SCROLL_SPEED;
            elem->scroll_amount_x += ATRA_GUI_SCROLL_SPEED;
            scrolled = 1;
        } else if (elem->scroll_amount_x > 0) {
            elem->scroll_x += ATRA_GUI_SCROLL_SPEED;
            elem->scroll_amount_x -= ATRA_GUI_SCROLL_SPEED;
            scrolled = 1;
        }
        
        if (elem->scroll_amount_y < 0) {
            elem->scroll_y -= ATRA_GUI_SCROLL_SPEED;
            elem->scroll_amount_y += ATRA_GUI_SCROLL_SPEED;
            scrolled = 1;
        } else if (elem->scroll_amount_y > 0) {
            elem->scroll_y += ATRA_GUI_SCROLL_SPEED;
            elem->scroll_amount_y -= ATRA_GUI_SCROLL_SPEED;
            scrolled = 1;
        }

        if (scrolled) {
            elem->scroll_x = MAX(elem->scroll_x, 0);
            elem->scroll_y = MAX(elem->scroll_y, 0);

            if (elem->scroll_x_max.value.pixels > 0) {
                const float padding = gui.hover->style.base->padding[0];
                const float border = BIT_CHK(gui.hover->attrs, ATRA_GUI_ELEM_HASBORDER) ?
					gui.hover->style.base->border.size : 0;
                const float x_offset = (border + padding) * 2;
                const float elem_w = gui.hover->real_quad.w * 
                        atra_window_getwidth() - x_offset;

                const int max_pix = DECIDE(
                    elem->scroll_x_max.data_type == ATRA_GUI_CONS_PIXEL,
                    elem->scroll_x_max.value.pixels,
                    elem->scroll_x_max.value.percent * elem_w);

                elem->scroll_x = MIN(elem->scroll_x, max_pix);
            }
            
            if (elem->scroll_y_max.value.pixels > 0) {
                const float padding = gui.hover->style.base->padding[1];
                const float border = BIT_CHK(gui.hover->attrs, ATRA_GUI_ELEM_HASBORDER) ?
					gui.hover->style.base->border.size : 0;
                const float y_offset = (border + padding) * 2;
                const float elem_h = gui.hover->real_quad.h *
                        atra_window_getheight() - y_offset;

                const int max_pix = DECIDE(
                    elem->scroll_y_max.data_type == ATRA_GUI_CONS_PIXEL,
                    elem->scroll_y_max.value.pixels,
                    elem->scroll_y_max.value.percent * elem_h);

                elem->scroll_y = MIN(elem->scroll_y, max_pix);
            }
            atra_gui_elem_flagremesh(elem);
        }
    }

    /* Call children to update */
    for (i = 0; i < child_count; i++) {
        child = *(GUIElem **) atra_list_get(&elem->children, i);

        atra_gui_elem_update(child, dt);
    }
}

/* Returns the number of elements in an element's tree, including itself */
ATRA_API
int
atra_gui_elem_treesize(GUIElem *elem)
{
    const int list_size = atra_list_length(&elem->children);

    GUIElem *child;
    int child_count, i;

    child_count = 1;

    for (i = 0; i < list_size; i++) {
        child = *(GUIElem **) atra_list_get(&elem->children, i);
        child_count += atra_gui_elem_treesize(child);
    }

    return child_count;
}

/* Sets the X position constraint of the element */
ATRA_API
void
atra_gui_elem_setpos_x(GUIElem *elem, int type, float data)
{
    elem->quad.types[0] = type;
    elem->quad.x = data;

    atra_gui_elem_flagremesh(elem);
    atra_gui_elem_calcquad(elem);
}

/* Sets the Y position constraint of the element */
ATRA_API
void
atra_gui_elem_setpos_y(GUIElem *elem, int type, float data)
{
    elem->quad.types[1] = type;
    elem->quad.h = data;

    atra_gui_elem_flagremesh(elem);
    atra_gui_elem_calcquad(elem);
}

/* Sets the width constraint of the element */
ATRA_API
void
atra_gui_elem_setwidth(GUIElem *elem, int type, float data)
{
    elem->quad.types[2] = type;
    elem->quad.w = data;

    atra_gui_elem_flagremesh(elem);
    atra_gui_elem_calcratio(elem);
    atra_gui_elem_calcquad(elem);
}

/* Sets the height constraint of the element */
ATRA_API
void
atra_gui_elem_setheight(GUIElem *elem, int type, float data)
{
    elem->quad.types[3] = type;
    elem->quad.h = data;

    atra_gui_elem_flagremesh(elem);
    atra_gui_elem_calcratio(elem);
    atra_gui_elem_calcquad(elem);
}

/* Sets the maximum scroll amount of the element on the x axis */
ATRA_API
void
atra_gui_elem_setmaxscroll_x(GUIElem *elem, int type, float data)
{
    if (type == ATRA_GUI_CONS_PIXEL) {
        elem->scroll_x_max.value.pixels = data;
    } else {
        elem->scroll_x_max.value.percent = data;
    }
    elem->scroll_x_max.data_type = type;
}

/* Sets the maximum scroll amount of the element on the y axis */
ATRA_API
void
atra_gui_elem_setmaxscroll_y(GUIElem *elem, int type, float data)
{
    if (type == ATRA_GUI_CONS_PIXEL) {
        elem->scroll_y_max.value.pixels = data;
    } else {
        elem->scroll_y_max.value.percent = data;
    }
    elem->scroll_y_max.data_type = type;
}

/* Sets the alignment constraint flags of the element */
ATRA_API
void
atra_gui_elem_setalignment(GUIElem *elem, u32 flags)
{
	elem->align = flags;
}

/* Sets the pointer to a specific style subtype */
ATRA_API
void
atra_gui_elem_setstyle(GUIElem *elem, int type, void *style)
{
    switch (type) {
    case ATRA_GUI_STYLE_BASE:
        elem->style.base = style; break;
    case ATRA_GUI_STYLE_TEXT:
        elem->style.text = style; break;
    case ATRA_GUI_STYLE_TEXTEDIT:
        elem->style.edit = style; break;
    case ATRA_GUI_STYLE_TEXTSELECTION:
        elem->style.selection = style; break;
    case ATRA_GUI_STYLE_TEXTCURSOR:
        elem->style.cursor = style; break;
    case ATRA_GUI_STYLE_SCROLLBAR:
        elem->style.scrollbar = style; break;
    default:
        break;
    }
    atra_gui_elem_flagremesh(elem);
}

/* Set the element's contained text data, additionally constructs codepoint array */
ATRA_API
void
atra_gui_elem_settext(GUIElem *elem, const char *text)
{
    if (elem->text) {
        free(elem->text);
        free(elem->codepoints);
    }
    elem->text = atra_util_strdup(text);
    elem->codepoints = atra_utf_tocodepointsterm(text, &elem->code_count);
    elem->rebuild_rows = 1;
    atra_gui_elem_flagremesh(elem);
}

/* Set the element's text alignment mode */
ATRA_API
void
atra_gui_elem_settextalign(GUIElem *elem, int alignment)
{
    elem->text_align = alignment;
    elem->rebuild_rows = 1;
    atra_gui_elem_flagremesh(elem);
}

/* Alters the element's style to hide everything except its child elements.
 * Useful for having a group of floating elements without a background. */
ATRA_API
void
atra_gui_elem_setinvisible(GUIElem *elem)
{
    /* TODO: Make some flag that simply skips meshing for the element, but DOES mesh it's children */

    atra_gui_elem_flagremesh(elem);
}

/* Flags both an element and it's window for remeshing */
ATRA_API
void
atra_gui_elem_flagremesh(GUIElem *elem)
{
    elem->dirty = 1;

    if (elem->window) {
        elem->window->dirty = 1;
    }

    if (elem->parent && BIT_CHK(elem->parent->attrs, ATRA_GUI_ELEM_AUTOALIGN)) {
        elem->parent->dirty = 1;
    }
}

/* Calculates the element's internal screen-space quad and that of all of it's children */
ATRA_API
void
atra_gui_elem_calcquad(GUIElem *elem)
{
    const int has_parent = elem->parent != NULL;
    const int child_count = atra_list_length(&elem->children);
    
    GUIElem *child;
    

    /* Skip to next quad if this element is already auto-aligned */
    if (has_parent && BIT_CHK(elem->parent->attrs, ATRA_GUI_ELEM_AUTOALIGN)) {
        if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_AUTOALIGN)) {
            atra_gui_elem_calcquad_auto(elem);
        }

        for (int i = 0; i < child_count; i++) {
            child = *(GUIElem **) atra_list_get(&elem->children, i);
            atra_gui_elem_calcquad(child);
        }

        return;
    }

    const float screen_width = atra_window_getwidth();
    const float screen_height = atra_window_getheight();

    const float screen_width_inv = 1.0 / screen_width;
    const float screen_height_inv = 1.0 / screen_height;

    const float aspect_ratio_w = screen_width * screen_height_inv;
    const float aspect_ratio_h = screen_height * screen_width_inv;

    const int elem_keep_w = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_KEEPRATIO_W); 
    const int elem_keep_h = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_KEEPRATIO_H);

    const int ispix_x = elem->quad.types[0] == ATRA_GUI_CONS_PIXEL;
    const int ispix_y = elem->quad.types[1] == ATRA_GUI_CONS_PIXEL;
    const int ispix_w = elem->quad.types[2] == ATRA_GUI_CONS_PIXEL;
    const int ispix_h = elem->quad.types[3] == ATRA_GUI_CONS_PIXEL;

    /* Calculate parent quad */
    GUIElem * restrict parent = elem->parent;
    Quad parent_quad;
    Quad result;
    int i;

    if (has_parent) {
        const float border_size_w = BIT_CHK(parent->attrs, ATRA_GUI_ELEM_HASBORDER) ?
				parent->style.base->border.size * screen_width_inv : 0;
        const float border_size_h = BIT_CHK(parent->attrs, ATRA_GUI_ELEM_HASBORDER) ?
				parent->style.base->border.size * screen_height_inv : 0;
        const float padding_w = parent->style.base->padding[0] * screen_width_inv;
        const float padding_h = parent->style.base->padding[1] * screen_height_inv;

        parent_quad.x = parent->real_quad.x + border_size_w + padding_w;
        parent_quad.y = parent->real_quad.y + border_size_h + padding_h;
        parent_quad.w = parent->real_quad.w - (border_size_w + padding_w) * 2;
        parent_quad.h = parent->real_quad.h - (border_size_h + padding_h) * 2;
    } else {
        parent_quad.x = 0.0;
        parent_quad.y = 0.0;
        parent_quad.w = 1.0;
        parent_quad.h = 1.0;
    }

    /* Pixel/relative positions and dimensions */
    float parent_ratio_w = 1.0f;
    float parent_ratio_h = 1.0f;

    if (has_parent) {
        parent_ratio_w = parent_quad.w / parent_quad.h;
        parent_ratio_h = 1.0f / parent_ratio_w;
    }

    const float elem_ratio_w = parent_ratio_h * aspect_ratio_h * DECIDE(ispix_h, screen_width_inv, parent_quad.w);
    const float elem_ratio_h = parent_ratio_w * aspect_ratio_w * DECIDE(ispix_w, screen_height_inv, parent_quad.h);

    const float elem_mult_w = DECIDE(ispix_w, screen_width_inv, parent_quad.w);
    const float elem_mult_h = DECIDE(ispix_h, screen_height_inv, parent_quad.h);

	/* Calculate screen-space width and height of the quad */
	result.w = DECIDE(elem_keep_h,
			elem->quad.h * elem->ratio * elem_ratio_w,
			elem->quad.w * elem_mult_w);
	result.h = DECIDE(elem_keep_w,
			elem->quad.w * elem->ratio * elem_ratio_h,
			elem->quad.h * elem_mult_h);

	/* Calculate screen-space X coordinate of the quad */
	if (BIT_CHK(elem->align, ATRA_GUI_CONS_MIDDLE_X)) {
		result.x = DECIDE(ispix_x,
				elem->quad.x * screen_width_inv + parent_quad.x + (parent_quad.w - result.w) / 2.0f,
				elem->quad.x * parent_quad.w + parent_quad.x + (parent_quad.w - result.w) / 2.0f);
		//result.x += (screen_width_inv / 8.0);
		//result.w += (screen_width_inv / 8.0);
	} else if (BIT_CHK(elem->align, ATRA_GUI_CONS_DIST_RIGHT)) {
		result.x = DECIDE(ispix_x,
				-elem->quad.x * screen_width_inv + parent_quad.x + parent_quad.w - result.w,
				-elem->quad.x * parent_quad.w + parent_quad.x + parent_quad.w - result.w);
		//result.x -= (screen_width_inv / 8.0);
		//result.w -= (screen_width_inv / 8.0);
	} else {
		result.x = DECIDE(ispix_x,
				elem->quad.x * screen_width_inv + parent_quad.x,
				elem->quad.x * parent_quad.w + parent_quad.x);
		//result.x += (screen_width_inv / 8.0);
		//result.w += (screen_width_inv / 8.0);
	}
 
	/* Calculate screen-space Y coordinate of the quad */
	if (BIT_CHK(elem->align, ATRA_GUI_CONS_MIDDLE_Y)) {
		result.y = DECIDE(ispix_y,
				elem->quad.y * screen_height_inv + parent_quad.y + (parent_quad.h - result.h) / 2.0f,
				elem->quad.y * parent_quad.h + parent_quad.y + (parent_quad.h - result.h) / 2.0f);
		//result.y += (screen_height_inv / 8.0);
		//result.h += (screen_height_inv / 8.0);
	} else if (BIT_CHK(elem->align, ATRA_GUI_CONS_DIST_BOT)) {
		result.y = DECIDE(ispix_y,
				-elem->quad.y * screen_height_inv + parent_quad.y + parent_quad.h - result.h,
				-elem->quad.y * parent_quad.h + parent_quad.y + parent_quad.h - result.h);
		//result.y -= (screen_height_inv / 8.0);
		//result.h -= (screen_height_inv / 8.0);
	} else {
		result.y = DECIDE(ispix_y,
				elem->quad.y * screen_height_inv + parent_quad.y,
				elem->quad.y * parent_quad.h + parent_quad.y);
		//result.y += (screen_height_inv / 8.0);
		//result.h += (screen_height_inv / 8.0);
	}
	result.x += (screen_width_inv / 8.0);
	result.y += (screen_height_inv / 8.0);
	result.w += (screen_width_inv / 8.0);
	result.h += (screen_height_inv / 8.0);

    if (has_parent && BIT_CHK(parent->attrs, ATRA_GUI_ELEM_SCROLLABLE | ATRA_GUI_ELEM_SCISSORTEST)) {
        result.x -= parent->scroll_x * screen_width_inv;
        result.y -= parent->scroll_y * screen_height_inv;
    }
    elem->real_quad = result;

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_AUTOALIGN)) {
        atra_gui_elem_calcquad_auto(elem);
    }

    for (i = 0; i < child_count; i++) {
        child = *(GUIElem **) atra_list_get(&elem->children, i);
        atra_gui_elem_calcquad(child);
    }
    elem->dirty = 0;
}

/* Calculates the quads for an element's children in auto-align mode */
ATRA_API
void
atra_gui_elem_calcquad_auto(GUIElem *elem)
{
    /*  Auto-align elements */

    const float screen_width = atra_window_getwidth();
    const float screen_height = atra_window_getheight();

    const float screen_width_inv = 1.0 / screen_width;
    const float screen_height_inv = 1.0 / screen_height;

    const float aspect_ratio_w = screen_width * screen_height_inv;
    //const float aspect_ratio_h = screen_height * screen_width_inv;

    const int child_count = atra_list_length(&elem->children);

    int i, j;

    int row_size = 0;
    float row_height;
    vec2 align_point;

    Quad elem_quad;
    Quad outer_quad;
    Quad inner_quad;
    
    GUIElem *child;
    GUIElem *prev = NULL;

    /* Calculate the quad of the parent element */
    const float border_size_w = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASBORDER) ?
		elem->style.base->border.size * screen_width_inv : 0;
    const float border_size_h = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASBORDER) ?
		elem->style.base->border.size * screen_height_inv : 0;
    const float padding_w = elem->style.base->padding[0] * screen_width_inv;
    const float padding_h = elem->style.base->padding[1] * screen_height_inv;

    elem_quad.x = elem->real_quad.x + border_size_w + padding_w;
    elem_quad.y = elem->real_quad.y + border_size_h + padding_h;
    elem_quad.w = elem->real_quad.w - (border_size_w + padding_w) * 2;
    elem_quad.h = elem->real_quad.h - (border_size_h + padding_h) * 2;

    align_point[0] = elem_quad.x;
    align_point[1] = elem_quad.y;
    
    for (i = 0; i < child_count; i += row_size) {
        prev = NULL;

        /* Get number of elements in row */
        for (j = i; j < child_count; j++) {
            child = *(GUIElem **) atra_list_get(&elem->children, j);

            if (prev != NULL &&
                    (child->quad.y < prev->quad.y) &&
                    (child->quad.y > prev->quad.y)) {
                break;
            }
        }
        row_size = j - i;

        /* Calculate the full widths and heights (includes margins) */
        row_height = 0.0f;

        for (j = 0; j < row_size; j++) {
            child = *(GUIElem **) atra_list_get(&elem->children, j + i);

            /* Calculate width */
            //const int keep_w = BIT_CHK(child->attrs, ATRA_GUI_ELEM_KEEPRATIO_W);
            const int keep_h = BIT_CHK(child->attrs, ATRA_GUI_ELEM_KEEPRATIO_H);

            const int ispix_w = child->quad.types[2] == ATRA_GUI_CONS_PIXEL;
            const int ispix_h = child->quad.types[3] == ATRA_GUI_CONS_PIXEL;

            const float margin_x = child->style.base->margin[0] * screen_width_inv;
            const float margin_y = child->style.base->margin[1] * screen_height_inv;

            const float elem_ratio_w = elem_quad.w / elem_quad.h;
            //const float elem_ratio_h = 1.0f / elem_ratio_w;

            /*  TODO: Getting the auto-align to handle user-defined widths will require:
                    A separate loop to calculate the total width of all non-auto width children
                    Taking the remainder of the free space and splitting it between auto elements
             */
            outer_quad.w = 1.0f / row_size;
            inner_quad.w = outer_quad.w - margin_x * 2;

            /* Calculate height */

            //const float ratio_w = aspect_ratio_h * DECIDE(ispix_h, screen_width_inv, elem_quad.w);
            const float ratio_h = elem_ratio_w * aspect_ratio_w * DECIDE(ispix_w, screen_height_inv, elem_quad.h);

            const float mult_w = DECIDE(ispix_w, screen_width_inv, elem_quad.w);
            const float mult_h = DECIDE(ispix_h, screen_height_inv, elem_quad.h);

            inner_quad.h = DECIDE(keep_h, inner_quad.w * child->ratio * ratio_h, child->quad.h * mult_h);
            outer_quad.h = inner_quad.h + margin_y * 2;

            outer_quad.w *= mult_w;
            inner_quad.w *= mult_w;

            row_height = MAX(row_height, outer_quad.h);
        }
        
        /* Set positions and calculate real quad for each element in row */
        for (j = 0; j < row_size; j++) {
            child = *(GUIElem **) atra_list_get(&elem->children, j + i);
            
            const float margin_x = child->style.base->margin[0] * screen_width_inv;
            const float margin_y = child->style.base->margin[1] * screen_height_inv;

            child->real_quad.x = align_point[0] + margin_x;
            child->real_quad.y = align_point[1] + margin_y;
            child->real_quad.w = inner_quad.w;
            child->real_quad.h = inner_quad.h;

            child->real_quad.x -= elem->scroll_x * screen_width_inv;
            child->real_quad.y -= elem->scroll_y * screen_height_inv;

            align_point[0] += outer_quad.w;
        }
        align_point[1] += row_height;
    }

    /* TODO: Handle alignment? */
    /* Problem, since width is currently fully occupied, height is the only aligment
       that can be calculated for, however this may cause issues in scrolling.
    */

    elem->dirty = 0;
}

/* Returns a quad containing the screen-space pixel data */
ATRA_API
IQuad
atra_gui_elem_getquad_pix(GUIElem *elem)
{
    const int screen_w = atra_window_getwidth();
    const int screen_h = atra_window_getheight();

    IQuad quad;

    quad.x = elem->real_quad.x * screen_w;
    quad.y = elem->real_quad.y * screen_h;
    quad.w = elem->real_quad.w * screen_w;
    quad.h = elem->real_quad.h * screen_h;

    return quad;
}

/* Returns a quad containting the screen-space normalised [0, 1] data */
ATRA_API
Quad
atra_gui_elem_getquad_norm(GUIElem *elem)
{
    return elem->real_quad;
}

/* Returns the position of the element in normalised coordinates.
 * Will take parent element position into account when `relative` flag is set */
ATRA_API
void
atra_gui_elem_getpos_norm(GUIElem *elem, float * restrict x, float * restrict y)
{
    const int x_pix = elem->quad.types[0] == ATRA_GUI_CONS_PIXEL;
    const int y_pix = elem->quad.types[1] == ATRA_GUI_CONS_PIXEL;

    *x = elem->quad.x / DECIDE(x_pix, atra_window_getwidth(), 1);
    *y = elem->quad.y / DECIDE(y_pix, atra_window_getheight(), 1);
}

/* Returns the width of the element in normalised coordinates. 
 * Will take parent element dimensions into account when `relative` flag is set */
ATRA_API
float
atra_gui_elem_getwidth_norm(GUIElem *elem)
{
    const int ispix = elem->quad.types[2] == ATRA_GUI_CONS_PIXEL;
    
    return elem->quad.w / DECIDE(ispix, atra_window_getwidth(), 1);
}

/* Returns the height of the element in normalised coordinates.
 * Will take parent element dimensions into account when `relative` flag is set */
ATRA_API
float
atra_gui_elem_getheight_norm(GUIElem *elem)
{
    const int ispix = elem->quad.types[3] == ATRA_GUI_CONS_PIXEL;
    
    return elem->quad.h / DECIDE(ispix, atra_window_getheight(), 1);
}

/* Composes an element's codepoints into row data for further processing */
ATRA_API
void
atra_gui_elem_calcrowdata(GUIElem *elem)
{
    const Font * const font = elem->style.text->font;

    const float font_size = elem->style.text->size;
    const float font_spacing = elem->style.text->spacing;
    const float font_border = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASBORDER) ? elem->style.base->border.size : 0;
    const float * const font_padding = elem->style.base->padding;

    const float x_offset = font_border + font_padding[0];
    const float y_offset = font_border + font_padding[1];

    const float screen_width = atra_window_getwidth();
    const float screen_height = atra_window_getheight();

    const float scale = font_size / font->size;
    const int code_count = elem->code_count;
    const int should_wrap = (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_NOWRAP) == 0);
    const int multiline = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_MULTILINE);

    const Quad quad = {
        .x = elem->real_quad.x * screen_width + x_offset,
        .y = elem->real_quad.y * screen_height + y_offset,
        .w = elem->real_quad.w * screen_width - x_offset * 2,
        .h = elem->real_quad.h * screen_height - y_offset * 2
    };
    
    GUILineText row = {0};

    int index, i;
    int row_count;
    int row_length;
    int word_length;
    float word_width;
    float row_width;
    float *code_widths;

    atra_list_clear(&elem->rows);

    code_widths = malloc(code_count * sizeof(float));

    /* Get the number, size, width and index of each codepoint */
    for (i = 0; i < code_count; i++) {
        index = atra_font_getglyphindex(font, elem->codepoints[i]);
        code_widths[i] = font_spacing + scale * 
                DECIDE(font->glyphs[index].advance != 0,
                font->glyphs[index].advance, font->quads[index].w);
    }
    word_width = 0.0f;
    row_width = 0.0f;
    row_count = 1;
    row_length = 0;
    word_length = 0;
    row.codes = elem->codepoints;

    /* Calculate text row layout information */
    for (i = 0; i < code_count; i++) {
        word_length++;

        /* Handle newline character */
        if (elem->codepoints[i] == '\n' && multiline) {
            /* Ensure line wrapping is actioned before processing newline */
            if (row_width + word_width - code_widths[i] > quad.w && should_wrap && multiline) {
                /* Record row data and progress to next row */
                row.length = row_length;
                row.width = row_width;
                atra_list_add(&elem->rows, &row);

                row_width = word_width;
                row_length = word_length;
                word_width = 0.0f;
                word_length = 0;
                row.codes += row.length;
                row_count++;
            }

            /* Record row data and progress to next row */
            row.length = row_length + word_length;
            row.width = row_width + word_width;
            atra_list_add(&elem->rows, &row);

            row_width = 0.0f;
            row_length = 0;
            word_width = 0.0f;
            word_length = 0;
            row.codes += row.length;
            row_count++;
            
            continue;
        }
        word_width += code_widths[i];
        
        /* Force character wrap if word is longer than line */
        if (word_width > quad.w && should_wrap && multiline) {
            /* Ensure text is already on it's own line */
            if (row_length > 0) {
                row.length = row_length;
                row.width = row_width;
                atra_list_add(&elem->rows, &row);

                row_width = word_width;
                row_length = word_length;
                word_width = 0.0f;
                word_length = 0;
                row.codes += row.length;
                row_count++;
            }
            /* Record row data and progress to next row */
            row.length = row_length + word_length - 1;
            row.width = row_width + word_width - code_widths[i];
            atra_list_add(&elem->rows, &row);

            row_width = 0.0f;
            row_length = 0;
            word_width = code_widths[i];
            word_length = 1;
            row.codes += row.length;
            row_count++;
        }

        /* Finalise rows if this is the final codepoint */
        if (i == code_count - 1) {
            /* Check for wrapping on the current row */
            if (row_width + word_width > quad.w && should_wrap && multiline) {
                /* Record row data and progress to next row */
                row.length = row_length;
                row.width = row_width;
                atra_list_add(&elem->rows, &row);

                row_width = word_width;
                row_length = word_length;
                word_width = 0.0f;
                word_length = 0;
                row.codes += row.length;
                row_count++;
            }
            /* Calculate final row */
            row.length = row_length + word_length;
            row.width = row_width + word_width;
            atra_list_add(&elem->rows, &row);

            break;
        }

        /* Add word to row if breaking character is found */
        if (atra_utf_iscodepointbreaking(elem->codepoints[i])) {
            /* Check that word fits in quad (row_width + word_width) */
            if (row_width + word_width - code_widths[i] > quad.w && should_wrap && multiline) {
                /* Record row data and progress to next row */
                row.length = row_length;
                row.width = row_width;
                atra_list_add(&elem->rows, &row);

                row_width = word_width;
                row_length = word_length;
                word_width = 0.0f;
                word_length = 0;
                row.codes += row.length;
                row_count++;
            } else {
                row_width += word_width;
                row_length += word_length;
                word_width = 0.0f;
                word_length = 0;
            }
        }
    }
    free(code_widths);
}

/* Draws a string inside a given rectangular bound */
ATRA_API
void
atra_gui_elem_drawtext(GUIElem *elem, Mesh *mesh, float depth)
{   
    const Font * const font = elem->style.text->font;

    const float font_size = elem->style.text->size;
    const float font_spacing = elem->style.text->spacing;
    const float font_border = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASBORDER) ? elem->style.base->border.size : 0;
    const float * const font_padding = elem->style.base->padding;

    const float x_offset = font_border + font_padding[0];
    const float y_offset = font_border + font_padding[1];
    
    const float screen_width = atra_window_getwidth();
    const float screen_height = atra_window_getheight();
    
    const float scale = font_size / font->size;
    const float row_skip = (font->size + font->size * 0.25f) * scale;
    const int code_count = elem->code_count;
    
    const int centre = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_CENTRE);
    const int right = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_RIGHT);
    const int middle = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_MIDDLE);
    const int bottom = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_BOTTOM);

    const int row_count = atra_list_length(&elem->rows);
    const float x_factor = DECIDE(centre, 0.5f, 1.0f);
    const float y_factor = DECIDE(middle, 0.5f, 1.0f);
    const float total_height = (row_count * row_skip) - (font->size * 0.25f * scale);

    const Quad quad = {
        .x = elem->real_quad.x * screen_width + x_offset - elem->scroll_x,
        .y = elem->real_quad.y * screen_height + y_offset - elem->scroll_y,
        .w = elem->real_quad.w * screen_width - x_offset * 2,
        .h = elem->real_quad.h * screen_height - y_offset * 2
    };

    *mesh = atra_mesh_begin(ATRA_QUADS,
            ATRA_MESH_BUFFER_POSITION |
            ATRA_MESH_BUFFER_TEXCOORD |
            ATRA_MESH_BUFFER_COLORS);
    atra_mesh_setactive(mesh);

    if (elem->code_count == 0) {
        atra_mesh_end(FALSE);
        return;
    }

    if (font->texture.id == 0) {
        atra_log_print(LOG_WARN, "font: Attempted to draw with an incomplete font");
        atra_mesh_end(FALSE);
        return;
    }
    
    int index, i, j;
    float x_pos, y_pos;
    float *code_widths;
    Color color;
    const GUILineText *row;
    
    code_widths = malloc(code_count * sizeof(float));

    switch (elem->state) {
    case ATRA_GUI_ELEM_NORMAL:
        color = elem->style.text->normal;
        break;
    case ATRA_GUI_ELEM_HOVER:
        color = elem->style.text->hover;
        break;
    case ATRA_GUI_ELEM_PRESS:
    case ATRA_GUI_ELEM_ACTIVE:
        color = elem->style.text->pressed;
        break;
    case ATRA_GUI_ELEM_DISABLED:
        color = elem->style.text->disabled;
        break;
    }

    /* Get the number, size, width and index of each codepoint */
    for (i = 0; i < code_count; i++) {
        index = atra_font_getglyphindex(font, elem->codepoints[i]);
        code_widths[i] = font_spacing + scale * 
                DECIDE(font->glyphs[index].advance != 0,
                font->glyphs[index].advance, font->quads[index].w);
    }
 
    /* Draw row data */
    for (i = 0; i < row_count; i++) {
        row = atra_list_get(&elem->rows, i);
        x_pos = quad.x;
        y_pos = quad.y + (row_skip * i);

        /* Calculate alignment offsets */
        if (total_height < quad.h) {
            y_pos += DECIDE(middle || bottom, (quad.h - total_height) * y_factor, 0.0f);
        }
        x_pos += DECIDE(centre || right, (quad.w - row->width) * x_factor, 0.0f);

        /* Draw row characters */
        for (j = 0; j < row->length; j++) {
            atra_font_drawcodepoint_mesh(font, row->codes[j],
                    x_pos, y_pos, depth, font_size, color);
            x_pos += code_widths[row->codes - elem->codepoints + j];
        }
    }
    atra_mesh_end(FALSE);

    free(code_widths);
}

/* Plays an animation for a GUI element with an optional delay (in milliseconds) */
ATRA_API
void
atra_gui_elem_playanim(GUIElem *elem, GUIAnim *anim, uint delay)
{
    elem->anim = anim;
    elem->anim_time = -delay;
}

/* Returns the element with the matching string identifier */
ATRA_API
GUIElem *
atra_gui_elem_find(const char *str_id)
{
    return *(GUIElem **)atra_hashmap_get(&gui.elem_map, &str_id);
}

/* Adds an element as a child to an element */
ATRA_API
void
atra_gui_elem_addelem(GUIElem *parent, GUIElem *child)
{
    if (parent->window != NULL) {
        child->window = parent->window;
        child->window->dirty = 1;
    }
    child->parent = parent;
    child->dirty = 1;
    parent->dirty = 1;

    atra_list_add(&parent->children, &child);
    atra_gui_elem_calcquad(child);
}

/* Translates the element transform by the given x and y */
ATRA_API
void
atra_gui_elem_translate(GUIElem *elem, float x, float y)
{
    vec3 trans_vec = {x, y, 0.0f};
    glm_translate(elem->transform, trans_vec);
}

/* Rotates the element transform by the given angle (radians) around the axis (x, y, z) */
ATRA_API
void
atra_gui_elem_rotate(GUIElem *elem, float x, float y, float z, float angle)
{
    vec3 rot_vec = {x, y, z};
    glm_rotate(elem->transform, angle, rot_vec);
}

/* Scales the element transform by the given x and y */
ATRA_API
void
atra_gui_elem_scale(GUIElem *elem, float x, float y)
{
    vec3 scale_vec = {x, y, 1.0f};
    glm_scale(elem->transform, scale_vec);
}


/*
 *  Specialised Element Creation Functions
 */

ATRA_API
GUIElem *
atra_gui_elem_create_panel(const char *id, int attrib, GUIQuad quad)
{
    GUIElem *elem = calloc(1, sizeof(GUIElem));
    char *str = elem->id_str;

    strncpy(elem->id_str, id, ATRA_GUI_MAX_ID_LEN);
    elem->attrs = attrib;
    elem->state = ATRA_GUI_ELEM_NORMAL;
    elem->quad = quad;
    elem->style = gui_styles.panel;
    elem->ratio = quad.w / quad.h;
    elem->children = atra_list_create(sizeof(GUIElem *));

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASTEXT)) {
        elem->rows = atra_list_create(sizeof(GUILineText));
    }

    atra_hashmap_add(&gui.elem_map, &str, &elem);
    glm_mat4_identity(elem->transform);
    atra_gui_elem_flagremesh(elem);

    return elem;
}

ATRA_API
GUIElem *
atra_gui_elem_create_label(const char *id, int attrib, GUIQuad quad)
{
    GUIElem *elem = calloc(1, sizeof(GUIElem));
    char *str = elem->id_str;

    strncpy(elem->id_str, id, ATRA_GUI_MAX_ID_LEN);
    elem->attrs = attrib | ATRA_GUI_ELEM_HASTEXT;
    elem->state = ATRA_GUI_ELEM_NORMAL;
    elem->quad = quad;
    elem->style = gui_styles.label;
    elem->ratio = quad.w / quad.h;
    elem->children = atra_list_create(sizeof(GUIElem *));

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASTEXT)) {
        elem->rows = atra_list_create(sizeof(GUILineText));
    }

    atra_hashmap_add(&gui.elem_map, &str, &elem);
    glm_mat4_identity(elem->transform);
    atra_gui_elem_flagremesh(elem);

    return elem;
}

ATRA_API
GUIElem *
atra_gui_elem_create_button(const char *id, int attrib, GUIQuad quad)
{
    GUIElem *elem = calloc(1, sizeof(GUIElem));
    char *str = elem->id_str;

    strncpy(elem->id_str, id, ATRA_GUI_MAX_ID_LEN);
    elem->attrs = attrib | ATRA_GUI_ELEM_CLICKABLE;
    elem->state = ATRA_GUI_ELEM_NORMAL;
    elem->quad = quad;
    elem->style = gui_styles.button;
    elem->ratio = quad.w / quad.h;
    elem->children = atra_list_create(sizeof(GUIElem *));

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASTEXT)) {
        elem->rows = atra_list_create(sizeof(GUILineText));
    }

    atra_hashmap_add(&gui.elem_map, &str, &elem);
    glm_mat4_identity(elem->transform);
    atra_gui_elem_flagremesh(elem);

    return elem;
}

ATRA_API
GUIElem *
atra_gui_elem_create_toggle(const char *id, int attrib, GUIQuad quad)
{
    GUIElem *elem = calloc(1, sizeof(GUIElem));
    char *str = elem->id_str;

    strncpy(elem->id_str, id, ATRA_GUI_MAX_ID_LEN);
    elem->attrs = attrib | ATRA_GUI_ELEM_CLICKABLE | ATRA_GUI_ELEM_TOGGLING;
    elem->state = ATRA_GUI_ELEM_NORMAL;
    elem->quad = quad;
    elem->style = gui_styles.toggle;
    elem->ratio = quad.w / quad.h;
    elem->children = atra_list_create(sizeof(GUIElem *));

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASTEXT)) {
        elem->rows = atra_list_create(sizeof(GUILineText));
    }

    atra_hashmap_add(&gui.elem_map, &str, &elem);
    glm_mat4_identity(elem->transform);
    atra_gui_elem_flagremesh(elem);

    return elem;
}

ATRA_API
GUIElem *
atra_gui_elem_create_slider(const char *id, int attrib, GUIQuad quad)
{
    /* Needs ability to display numeric values */
    /* Slider controlled by mouse */
    /* Fill color for bar */

    /*  Some default design choices:
            - Bar should highlight when moused over
            - Bar should highlight further when pressed
            - Update function should use mouse input and respond to relative x/y displacement.
            - Displacement of bar (in pixels) with mouse should be translated
              to some numerical value.
            - Scrolling on the parent should also adjust scrollbar
    */

    ATRA_UNUSED(id);
    ATRA_UNUSED(attrib);
    ATRA_UNUSED(quad);

    return NULL;
}

ATRA_API
GUIElem *
atra_gui_elem_create_progressbar(const char *id, int attrib, GUIQuad quad)
{
    /* Two elements, one for bar frame, one for fill */
    /* Maybe have auxiliary functions to set progress amount */

    GUIElem *base = calloc(1, sizeof(GUIElem));
    GUIElem *bar = calloc(1, sizeof(GUIElem));
    GUIElem *label = calloc(1, sizeof(GUIElem));
    char *str = base->id_str;

    strncpy(base->id_str, id, ATRA_GUI_MAX_ID_LEN);
    base->attrs = attrib;
    base->state = ATRA_GUI_ELEM_NORMAL;
    base->quad = quad;
    base->style = gui_styles.progressbar;
    base->ratio = quad.w / quad.h;
    base->children = atra_list_create(sizeof(GUIElem *));

    if (BIT_CHK(base->attrs, ATRA_GUI_ELEM_HASTEXT)) {
        base->rows = atra_list_create(sizeof(GUILineText));
    }
    atra_hashmap_add(&gui.elem_map, &str, &base);
    glm_mat4_identity(base->transform);
    atra_gui_elem_flagremesh(base);

    

    atra_gui_elem_addelem(base, bar);
    atra_gui_elem_addelem(bar, label);

    return base;
}

ATRA_API
GUIElem *
atra_gui_elem_create_scrollbar(const char *id, int attrib, GUIQuad quad)
{
    /*  Four elements: 
            - Background
            - Slider (shrinks/grows based on parent's scroll amount)
            - Up/Down buttons (optional)
    */
    
    /*  Some default design choices:
            - Bar should highlight when moused over
            - Bar should highlight further when pressed
            - Update function should use mouse input and respond to relative vertical displacement.
            - Displacement of bar (in pixels) with mouse should be translated to
              displacement of the parent element's scroll value.
            - Scrolling on the parent should also adjust scrollbar 
    */
    ATRA_UNUSED(id);
    ATRA_UNUSED(attrib);
    ATRA_UNUSED(quad);

    return NULL;
}

ATRA_API
GUIElem *
atra_gui_elem_create_textbox(const char *id, int attrib, GUIQuad quad)
{
    GUIElem *elem = calloc(1, sizeof(GUIElem));
    char *str = elem->id_str;

    strncpy(elem->id_str, id, ATRA_GUI_MAX_ID_LEN);
    elem->attrs = attrib | ATRA_GUI_ELEM_HASTEXT | ATRA_GUI_ELEM_TEXTINPUT |
            ATRA_GUI_ELEM_MULTILINE | ATRA_GUI_ELEM_SCISSORTEST;
    elem->state = ATRA_GUI_ELEM_NORMAL;
    elem->quad = quad;
    elem->style = gui_styles.textbox;
    elem->ratio = quad.w / quad.h;
    elem->children = atra_list_create(sizeof(GUIElem *));

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASTEXT)) {
        elem->rows = atra_list_create(sizeof(GUILineText));
    }

    atra_hashmap_add(&gui.elem_map, &str, &elem);
    glm_mat4_identity(elem->transform);
    atra_gui_elem_flagremesh(elem);

    return elem;
}

ATRA_API
GUIElem *
atra_gui_elem_create_textfield(const char *id, int attrib, GUIQuad quad)
{
    GUIElem *elem = calloc(1, sizeof(GUIElem));
    char *str = elem->id_str;

    strncpy(elem->id_str, id, ATRA_GUI_MAX_ID_LEN);
    elem->attrs = attrib | ATRA_GUI_ELEM_HASTEXT | ATRA_GUI_ELEM_TEXTINPUT |
            ATRA_GUI_ELEM_SCISSORTEST;
    elem->state = ATRA_GUI_ELEM_NORMAL;
    elem->quad = quad;
    elem->style = gui_styles.textfield;
    elem->ratio = quad.w / quad.h;
    elem->children = atra_list_create(sizeof(GUIElem *));

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASTEXT)) {
        elem->rows = atra_list_create(sizeof(GUILineText));
    }

    atra_hashmap_add(&gui.elem_map, &str, &elem);
    glm_mat4_identity(elem->transform);
    atra_gui_elem_flagremesh(elem);

    return elem;
}

ATRA_API
GUIElem *
atra_gui_elem_create_combobox(const char *id, int attrib, GUIQuad quad)
{
    /*
        Select from a menu of buttons, the active selection being displayed in the main box
        Will require auxiliary functions for adding/removing menu items.
    */

    /*  Design choices:
         - Elements:
            - Box displaying current selection
            - Box displaying a 'down' arrow to the right of the current selection
         - Clicking either element opens a drop-down menu of selections to make (buttons)
    */

    ATRA_UNUSED(id);
    ATRA_UNUSED(attrib);
    ATRA_UNUSED(quad);

    return NULL;

}

ATRA_API
GUIElem *
atra_gui_elem_create_menu(const char *id, int attrib, GUIQuad quad)
{
    /*
        A drop down menu containing a list of buttons.
        Will require auxiliary functions for adding/removing menu items.
    */
    ATRA_UNUSED(id);
    ATRA_UNUSED(attrib);
    ATRA_UNUSED(quad);

    return NULL;
}

ATRA_API
GUIElem *
atra_gui_elem_create_tooltip(const char *id, int attrib, GUIQuad quad)
{
    GUIElem *elem = calloc(1, sizeof(GUIElem));
    char *str = elem->id_str;

    strncpy(elem->id_str, id, ATRA_GUI_MAX_ID_LEN);
    elem->attrs = attrib | ATRA_GUI_ELEM_FALLTHROUGH;
    elem->state = ATRA_GUI_ELEM_NORMAL;
    elem->quad = quad;
    elem->style = gui_styles.tooltip;
    elem->ratio = quad.w / quad.h;
    elem->children = atra_list_create(sizeof(GUIElem *));

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASTEXT)) {
        elem->rows = atra_list_create(sizeof(GUILineText));
    }

    atra_hashmap_add(&gui.elem_map, &str, &elem);
    glm_mat4_identity(elem->transform);
    atra_gui_elem_flagremesh(elem);

    return elem;
}

/* Returns 1 if the given element intersects with the mouse cursor.
 * Mouse coordinates must be normalised to screen-space. */
ATRA_INLINE
int
atra_gui_elem_mouseintersects(GUIElem *elem, float mx, float my)
{
    //IQuad quad = atra_gui_elem_getquad_pix(elem);
    Quad quad = elem->real_quad;

    return (mx >= quad.x) && (mx <= quad.x + quad.w) &&
            (my >= quad.y) && (my <= quad.y + quad.h);
}

/* Returns a pointer to the topmost element in an element tree which the mouse
 * should interact with */
static GUIElem *
atra_gui_elem_findmousehover(GUIElem *elem, float mx_norm, float my_norm)
{
    GUIElem *child;
    GUIElem *focus = NULL;
    size_t i;

    if (!atra_gui_elem_mouseintersects(elem, mx_norm, my_norm)) {
        return NULL;
    }

    for (i = 0; i < atra_list_length(&elem->children); i++) {
        child = *(GUIElem **) atra_list_get(&elem->children, i);

        if (BIT_CHK(child->attrs, ATRA_GUI_ELEM_FALLTHROUGH) ||
                child->state == ATRA_GUI_ELEM_HIDDEN) {
            continue;
        }
        focus = atra_gui_elem_findmousehover(child, mx_norm, my_norm);

        if (focus) {
            return focus;
        }
    }

    if (focus == NULL) {
        focus = elem;
    }

    return focus;
}

/* Returns a pointer to the topmost element which the mouse should interact with */
ATRA_INLINE
GUIElem *
atra_gui_findmousehover(int mx, int my)
{
    GUIWindow *window;
    GUIElem *focus = NULL;
    float mx_norm, my_norm;
    int i;

    mx_norm = (float)mx / atra_window_getwidth();
    my_norm = (float)my / atra_window_getheight();

    for (i = atra_list_length(&gui.window_stack) - 1; i >= 0; i--) {
        window = *(GUIWindow **) atra_list_get(&gui.window_stack, i);

        if (BIT_CHK(window->states, ATRA_GUI_WINDOW_HIDDEN) ||
                BIT_CHK(window->attrs, ATRA_GUI_WINDOW_FALLTHROUGH)) {
            continue;
        } 
        focus = atra_gui_elem_findmousehover(window->elem, mx_norm, my_norm);

        if (focus) {
            break;
        }
    }

    return focus;
}

/* Checks if a callback is non-null, and calls it */
ATRA_INLINE
void
atra_gui_elem_callback(GUIElem *elem, int callback_type, int arg)
{
    if (elem->callbacks[callback_type] != NULL) {
        elem->callbacks[callback_type](elem, arg);
    }
}

/* Calculates the dimensional ratio of a GUI element */
ATRA_INLINE
void
atra_gui_elem_calcratio(GUIElem *elem)
{
    float width, height;

    if (elem->quad.types[2] == ATRA_GUI_CONS_PERCENT) {
        width = elem->quad.w * atra_window_getwidth();
    } else {
        width = elem->quad.w;
    }
    
    if (elem->quad.types[3] == ATRA_GUI_CONS_PERCENT) {
        height = elem->quad.h * atra_window_getheight();
    } else {
        height = elem->quad.h;
    }

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_KEEPRATIO_H)) {
        elem->ratio = height / width;
    } else {
        elem->ratio = width / height;
    }
}

ATRA_INLINE
void
atra_gui_text_home(GUIElem *elem)
{
    atra_gui_elem_flagremesh(elem);
    text_cursor.moved = 1;

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_MULTILINE) == 0) {
        text_cursor.pos = 0;
        return;
    }

    const int row_count = atra_list_length(&elem->rows);

    GUILineText *row;
    int curr_code = 0;
    int curr_row;

    for (curr_row = 0; curr_row < row_count; curr_row++) {
        row = atra_list_get(&elem->rows, curr_row);

        if (curr_code + row->length > (int)text_cursor.pos) {
            text_cursor.pos = curr_code;
            return;
        }
        curr_code += row->length;
    }
}

/* Shift text cursor to end of current row */
ATRA_INLINE
void
atra_gui_text_end(GUIElem *elem)
{
    atra_gui_elem_flagremesh(elem);
    text_cursor.moved = 1;

    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_MULTILINE) == 0) {
        text_cursor.pos = elem->code_count - 1;
        return;
    }

    const int row_count = atra_list_length(&elem->rows);

    GUILineText *row;
    int curr_code = 0;
    int curr_row;

    for (curr_row = 0; curr_row < row_count; curr_row++) {
        row = atra_list_get(&elem->rows, curr_row);
        curr_code += row->length;

        if (curr_code > (int)text_cursor.pos) {
            text_cursor.pos = curr_code - 1;
            return;
        }
    }
}

ATRA_INLINE
void
atra_gui_text_delete(GUIElem *elem)
{
    atra_gui_elem_flagremesh(elem);

    if (text_cursor.start == text_cursor.end) {
        if ((int)text_cursor.pos < elem->code_count - 1) {
            /* Delete current codepoint */
            atra_utf_removecodepoints(elem->codepoints, &elem->code_count,
                    text_cursor.pos, text_cursor.pos + 1);
        }
    } else {
        /* Delete codepoints in range [start, end) */
        atra_utf_removecodepoints(elem->codepoints, &elem->code_count,
                text_cursor.start, text_cursor.end);
    }
    elem->text = atra_utf_fromcodepointstr(elem->codepoints, elem->code_count);
    elem->rebuild_rows = 1;
    text_cursor.moved = 1;
}

ATRA_INLINE
void
atra_gui_text_backspace(GUIElem *elem)
{
    atra_gui_elem_flagremesh(elem);

    if (text_cursor.start == text_cursor.end) {
        if (text_cursor.pos > 0) {
            /* Delete previous character */
            atra_utf_removecodepoints(elem->codepoints, &elem->code_count,
                    text_cursor.pos - 1, text_cursor.pos);
            text_cursor.pos--;
        }
    } else {
        /* Delete codepoints in range [start, end) */
        atra_utf_removecodepoints(elem->codepoints, &elem->code_count,
                text_cursor.start, text_cursor.end);
        text_cursor.pos = text_cursor.start;
        text_cursor.end = text_cursor.start;
    }
    elem->text = atra_utf_fromcodepointstr(elem->codepoints, elem->code_count);
    elem->rebuild_rows = 1;
    text_cursor.moved = 1;
}

ATRA_INLINE
void
atra_gui_text_pageup(GUIElem *elem)
{
    /*  Find size of 'page' using text size and line skip size. (Elem height / line size)
        Get current cursor line
        Set target line as (cursor_line - page_size)
        Get target line first character position
        Set cursor to this character position
    */
    atra_gui_elem_flagremesh(elem);
    text_cursor.moved = 1;
}

ATRA_INLINE
void
atra_gui_text_pagedown(GUIElem *elem)
{
    /*  Find size of 'page' using text size and line skip size. (Elem height / line size)
        Get current cursor line
        Set target line as (cursor_line - page_size)
        Get target line first character position
        Set cursor to this character position
    */
    atra_gui_elem_flagremesh(elem);
    text_cursor.moved = 1;
}

ATRA_INLINE
void
atra_gui_text_enter(GUIElem *elem)
{
    /*  Insert newline character at current position (multi-line)
        OR
        Trigger some kind of special confirmation, if any (single-line)
    */
    if (BIT_CHK(elem->attrs, ATRA_GUI_ELEM_MULTILINE)) {
        /* Insert newline character */
        atra_gui_elem_inserttext(elem, "\n");
    } else {
        /* Run the element's special text confirmation callback, if any */ 
        if (elem->callbacks[ATRA_GUI_EVENT_TEXTENTERKEY]) {
            atra_gui_elem_callback(elem, ATRA_GUI_EVENT_TEXTENTERKEY, 0);
        }
    }
}

ATRA_INLINE
void
atra_gui_text_up(GUIElem *elem)
{
    atra_gui_elem_flagremesh(elem);
    text_cursor.moved = 1;

    if (text_cursor.row_num == 0) {
        return;
    }

    const Font * const font = elem->style.text->font;

    const float font_size = elem->style.text->size;
    const float font_spacing = elem->style.text->spacing;
    const float font_border = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASBORDER) ? elem->style.base->border.size : 0;
    const float * const font_padding = elem->style.base->padding;

    const float x_offset = font_border + font_padding[0];
    const float y_offset = font_border + font_padding[1];
    
    const float screen_width = atra_window_getwidth();
    const float screen_height = atra_window_getheight();
    
    const float scale = font_size / font->size;
    
    const int centre = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_CENTRE);
    const int right = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_RIGHT);

    const float x_factor = DECIDE(centre, 0.5f, 1.0f);
    
    const Quad quad = {
        .x = elem->real_quad.x * screen_width + x_offset,
        .y = elem->real_quad.y * screen_height + y_offset,
        .w = elem->real_quad.w * screen_width - x_offset * 2,
        .h = elem->real_quad.h * screen_height - y_offset * 2
    };

    int index, found, i;
    int curr_code;
    int row_length;
    float x_pos;
    float *code_widths;

    const GUILineText *row;

    curr_code = 0;

    /* Get character count up until the cursor's row */
    for (i = 0; i < text_cursor.row_num - 1; i++) {
        row = atra_list_get(&elem->rows, i);
        row_length = row->length;
        curr_code += row->length;
    }
    row = atra_list_get(&elem->rows, text_cursor.row_num - 1);
    row_length = row->length;
    code_widths = malloc(row_length * sizeof(float));

    /* Get the number, size, width and index of each codepoint */
    for (i = 0; i < row_length; i++) {
        index = atra_font_getglyphindex(font, row->codes[i]);
        code_widths[i] = font_spacing + scale * 
                DECIDE(font->glyphs[index].advance != 0,
                font->glyphs[index].advance, font->quads[index].w);
    }
    found = 0;
    x_pos = quad.x;
    x_pos += DECIDE(centre || right, (quad.w - row->width) * x_factor, 0.0f);
        
    for (i = 0; i < row_length; i++) {
        if (x_pos + 2 >= text_cursor.quad.x) {
            text_cursor.pos = curr_code + i;
            found = 1;
            break;
        }
        x_pos += code_widths[i];
    }
    
    if (found == 0) {
        /* Could not find suitable position, default to end of row */
        text_cursor.pos = curr_code + row_length - 1;
    }
    free(code_widths);
}

ATRA_INLINE
void
atra_gui_text_down(GUIElem *elem)
{
    const int row_count = atra_list_length(&elem->rows);
    
    atra_gui_elem_flagremesh(elem);
    text_cursor.moved = 1;

    if (text_cursor.row_num == row_count - 1) {
        return;
    }

    const Font * const font = elem->style.text->font;

    const float font_size = elem->style.text->size;
    const float font_spacing = elem->style.text->spacing;
    const float font_border = BIT_CHK(elem->attrs, ATRA_GUI_ELEM_HASBORDER) ? elem->style.base->border.size : 0;
    const float * const font_padding = elem->style.base->padding;

    const float x_offset = font_border + font_padding[0];
    const float y_offset = font_border + font_padding[1];
    
    const float screen_width = atra_window_getwidth();
    const float screen_height = atra_window_getheight();
    
    const float scale = font_size / font->size;
    
    const int centre = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_CENTRE);
    const int right = BIT_CHK(elem->text_align, ATRA_GUI_TEXTALIGN_RIGHT);

    const float x_factor = DECIDE(centre, 0.5f, 1.0f);
    
    const Quad quad = {
        .x = elem->real_quad.x * screen_width + x_offset,
        .y = elem->real_quad.y * screen_height + y_offset,
        .w = elem->real_quad.w * screen_width - x_offset * 2,
        .h = elem->real_quad.h * screen_height - y_offset * 2
    };

    int index, found, i;
    int curr_code;
    int row_length;
    float x_pos;
    float *code_widths;

    const GUILineText *row;

    curr_code = 0;

    /* Get character count up until the cursor's row */
    for (i = 0; i < text_cursor.row_num + 1; i++) {
        row = atra_list_get(&elem->rows, i);
        row_length = row->length;
        curr_code += row->length;
    }
    row = atra_list_get(&elem->rows, text_cursor.row_num + 1);
    row_length = row->length;
    code_widths = malloc(row_length * sizeof(float));

    /* Get the number, size, width and index of each codepoint */
    for (i = 0; i < row_length; i++) {
        index = atra_font_getglyphindex(font, row->codes[i]);
        code_widths[i] = font_spacing + scale * 
                DECIDE(font->glyphs[index].advance != 0,
                font->glyphs[index].advance, font->quads[index].w);
    }
    found = 0;
    x_pos = quad.x;
    x_pos += DECIDE(centre || right, (quad.w - row->width) * x_factor, 0.0f);
        
    for (i = 0; i < row_length; i++) {
        if (x_pos + 2 >= text_cursor.quad.x) {
            text_cursor.pos = curr_code + i;
            found = 1;
            break;
        }
        x_pos += code_widths[i];
    }
    
    if (found == 0) {
        /* Could not find suitable position, default to end of row */
        text_cursor.pos = curr_code + row_length - 1;
    }
    free(code_widths);
}

ATRA_INLINE
void
atra_gui_text_left(GUIElem *elem)
{
    if (text_cursor.pos > 0) {
        text_cursor.pos--;
    }
    text_cursor.start = text_cursor.pos;
    text_cursor.end = text_cursor.pos;
    text_cursor.moved = 1;
    atra_gui_elem_flagremesh(elem);
}

ATRA_INLINE
void
atra_gui_text_right(GUIElem *elem)
{
    if ((int)text_cursor.pos < elem->code_count - 1) {
        text_cursor.pos++;
    }
    text_cursor.start = text_cursor.pos;
    text_cursor.end = text_cursor.pos;
    text_cursor.moved = 1;
    atra_gui_elem_flagremesh(elem);
}
