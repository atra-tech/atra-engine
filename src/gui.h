/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved
 *
 *  gui.h - An immediate mode GUI module integrated with the engine.
 *
 * */

#include "common.h"

#include "texture.h"



enum gui_style_item_type {
    ATRA_GUI_STYLE_COLOR,   /* Drawn with a flat color */
    ATRA_GUI_STYLE_IMAGE,   /* Drawn using an image */
    ATRA_GUI_STYLE_GRID     /* Drawn as a grid of nine quads using an image */
};

union GUIStyleItemData {
    Color color;
    Image image;

};
