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

#include "texture.h"

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
    int char_count;     /* Total number of characters */
    int padding;        /* Omnidirectional spacing */
    Texture texture;    /* Font texture atlas */
    Quad *quads;        /* Character bounding boxes in texture */
    Glyph *chars;       /* Character information array */
} Font;

#endif /* ATRA_FONT_H */
