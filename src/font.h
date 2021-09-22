/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved
 *
 *  font.h - Tools for loading and drawing TrueType fonts.
 *
 * */

#ifndef ATRA_FONT_H
#define ATRA_FONT_H

#include "common.h"
#include "texture.h"

#define ATRA_FONT_DEFAULT_SIZE              32  /* 32px-high characters */
#define ATRA_FONT_DEFAULT_START_GLYPH       32  /* Skips control characters */
#define ATRA_FONT_DEFAULT_GLYPHCOUNT        95  /* ASCII character set */
#define ATRA_FONT_DEFAULT_PADDING           2   /* Default character padding */

#define ATRA_FONT_SDF_PADDING               4
#define ATRA_FONT_SDF_ON_EDGE_VALUE         128
#define ATRA_FONT_SDF_PIXEL_DIST_SCALE      64.0f
#define ATRA_FONT_BITMAP_ALPHA_THRESHOLD    80

enum font_type {
    ATRA_FONT_BITMAP,
    ATRA_FONT_SDF
};

enum font_packing_method {
    ATRA_FONT_PACK_DEFAULT,
    ATRA_FONT_PACK_SKYLINE
};

/* Font character information */
typedef struct Glyph {
    int value;          /* UTF-8 character value */
    int offset_x;       /* Glyph x-offset from origin */
    int offset_y;       /* Glyph y-offset from origin */
    int advance;        /* Post-glyph x-spacing */
    Image image;        /* Associated character image data */
} Glyph;

/* Font data structure */
typedef struct Font {
    int size;           /* Default character height */
    int glyph_count;    /* Total number of characters */
    int padding;        /* Omnidirectional spacing */
    Texture texture;    /* Font texture atlas */
    Quad *quads;        /* Character bounding boxes in texture */
    Glyph *glyphs;      /* Character information array */
} Font;

/* Loads a font from a TTF file and uploads it to the GPU */
ATRA_API Font atra_font_loadfile(const char *path, int font_size, int *chars, int count);

/* Loads a font from an image and uploads it to the GPU */
ATRA_API Font atra_font_loadimage(Image *image, Color key, int first);

/* Loads a font from a buffer containing TTF data and uploads it to the GPU */
ATRA_API Font atra_font_loadbuffer(const u8 *buf, size_t buf_size,
        int font_size, int *chars, int count);

/* Loads font glyph data from a TTF data buffer */
ATRA_API Glyph * atra_font_loadglyphs(const u8 *buf, int buf_size,
        int font_size, int *chars, int count, int type);

/* Deletes a font's glyph data from memory */
ATRA_API void atra_font_deleteglyphs(Font *font, int count);

/* Deletes a font, removing it from the GPU */
ATRA_API void atra_font_delete(Font *font);

/* Returns the resulting size of a string constructed using a particular font */
ATRA_API void atra_font_calctextsize(Font *font, const char *str, float font_size,
        float spacing, vec2 *result);

/* Returns the index position of a UTF-8 character in the font */
ATRA_API int atra_font_getglyphindex(Font *font, int codepoint);

/* Returns an image containing an atlas of a given font's glyphs */
ATRA_API Image atra_font_generateatlas(Font *font, int packing_method);

/* Draws a string to the screen using a given font */
ATRA_API void atra_font_drawtext(Font *font, const char *str, float x, float y,
        float font_size, float spacing, Color color);

/* Draws a string inside the a given rectangular bound (supports line wrapping) */
ATRA_API void atra_font_drawtextrect(Font *font, const char *str, Quad quad,
        float font_size, float spacing, int word_wrap, Color color);

/* Draws a single codepoint character to the screen */
ATRA_API void atra_font_drawcodepoint(Font *font, int codepoint, float x, float y,
        float font_size, Color color);

#endif /* ATRA_FONT_H */
