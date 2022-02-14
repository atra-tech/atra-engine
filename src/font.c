/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved
 *
 *  font.c - Tools for loading and drawing TrueType fonts.
 *
 * */

#include <math.h>

#include "atra.h"
#include "stb/stb_truetype.h"

/* Common UTF-8 character ranges */
/*static const u32 ranges_ascii[] = {
    0x0020, 0x00FF,
    0
};

static const u32 ranges_cyrillic[] = {
    0x0020, 0x00FF,
    0x0400, 0x052F,
    0x2DE0, 0x2DFF,
    0xA640, 0xA69F,
    0
};

static const u32 ranges_chinese[] = {
    0x0020, 0x00FF,
    0x3000, 0x30FF,
    0x31F0, 0x31FF,
    0xFF00, 0xFFEF,
    0x4e00, 0x9FAF,
    0 
};

static const u32 ranges_korean[] = {
    0x0020, 0x00FF,
    0x3131, 0x3163,
    0xAC00, 0xD79D,
    0
};
*/

/* Loads a font from a TTF file and uploads it to the GPU */
ATRA_API
Font
atra_font_loadfile(const char *path, int font_size, int *chars, int count)
{
    Font font = {0};

    if (atra_util_filehasext(path, ".ttf;.otf")) {
        /* Load TTF file */
        size_t size = 0;
        u8 *data = atra_util_loadfile(path, &size);

        font = atra_font_loadbuffer(data, size, font_size, chars, count);
        free(data);
    } else {
        /* Load image file */
        const Color key_color = {1.0f, 1.0f, 1.0f, 1.0f};

        Image atlas = atra_image_loadfile(path);

        if (atlas.data != NULL) {
            font = atra_font_loadimage(&atlas, key_color, ATRA_FONT_DEFAULT_GLYPHCOUNT);
            atra_image_destroy(&atlas);
        }
    }

    if (font.texture.id == 0) {
        atra_log_print(LOG_WARN, "font: Failed to load font texture: %s", path);
    } else {
        atra_texture_samplebase(&font.texture, ATRA_SAMPLE_NEAREST);
    }

    return font;
}

/* Loads a font from an image and uploads it to the GPU */
ATRA_API
Font
atra_font_loadimage(Image *image, Color key, int first)
{
    ATRA_UNUSED(image);
    ATRA_UNUSED(key);
    ATRA_UNUSED(first);

    Font font = {0};

    atra_log_print(LOG_WARN, "font: Image font loading not implemented");

    return font;
}

/* Loads a font from a buffer containing TTF data and uploads it to the GPU */
ATRA_API
Font
atra_font_loadbuffer(const u8 *buf, size_t buf_size, int font_size, int *chars, int count)
{
    Font font = {0};
    Image atlas;
    int i;

    font.size = font_size;
    font.glyph_count = (count > 0) ? count : ATRA_FONT_DEFAULT_GLYPHCOUNT;
    font.padding = 0;
    font.glyphs = atra_font_loadglyphs(buf, buf_size, font_size, chars, count, ATRA_FONT_SDF);

    if (font.glyphs != NULL) {
        font.padding = ATRA_FONT_DEFAULT_PADDING;

        atlas = atra_font_generateatlas(&font, ATRA_FONT_PACK_DEFAULT);
        font.texture = atra_texture_loadimage(&atlas);

        for (i = 0; i < font.glyph_count; i++) {
            const IQuad img_quad = {
                font.quads[i].x,
                font.quads[i].y,
                font.quads[i].w,
                font.quads[i].h};

            atra_image_destroy(&font.glyphs[i].image);
            font.glyphs[i].image = atra_image_subcopy(&atlas, img_quad);
        }
        atra_image_destroy(&atlas);
    } else {
        atra_log_print(LOG_WARN, "font: Unable to load font: No glyphs loaded");
    }

    return font;
}

/* Loads font glyph data from a TTF data buffer */
ATRA_API
Glyph *
atra_font_loadglyphs(const u8 *buf, int buf_size,
        int font_size, int *chars, int count, int type)
{
    ATRA_UNUSED(buf_size);

    Glyph *result = NULL;
    stbtt_fontinfo font_info = {0};
    int genchars = 0;
    int ascent, descent, line_gap;
    int width, height, chr;
    float scale;
    int i, j;

    if (buf == NULL) {
        atra_log_print(LOG_WARN, "font: Failed to process TTF data: NULL buffer");
        return NULL;
    }

    if (stbtt_InitFont(&font_info, buf, 0)) {
        /* Calculate font scale factor */
        scale = stbtt_ScaleForPixelHeight(&font_info, font_size);

        /* Calculate font metrics */
        stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);

        count = (count > 0) ? count : ATRA_FONT_DEFAULT_GLYPHCOUNT;
        result = malloc(count * sizeof(Glyph));

        if (chars == NULL) {
            chars = malloc(count * sizeof(int));
            genchars = 1;

            for (i = 0; i < count; i++) {
                chars[i] = i + 32;
            }
        }

        for (i = 0; i < count; i++) {
            width = 0;
            height = 0;
            chr = chars[i];
            result->value = chr;

            if (type != ATRA_FONT_SDF) {
                result[i].image.data = stbtt_GetCodepointBitmap(
                        &font_info, scale, scale, chr,
                        &width, &height,
                        &result[i].offset_x,
                        &result[i].offset_y);
            } else if (chr != 32) {
                result[i].image.data = stbtt_GetCodepointSDF(
                        &font_info, scale, chr,
                        ATRA_FONT_SDF_PADDING,
                        ATRA_FONT_SDF_ON_EDGE_VALUE,
                        ATRA_FONT_SDF_PIXEL_DIST_SCALE,
                        &width, &height,
                        &result[i].offset_x,
                        &result[i].offset_y);
            } else {
                result[i].image.data = NULL;
            }
            stbtt_GetCodepointHMetrics(&font_info, chr, &result[i].advance, NULL);
            result[i].advance = result[i].advance * scale;
            result[i].offset_y += ascent * scale;

            if (chr == 32) {
                /* Generate empty image for space character */
                result[i].image.data = calloc(result[i].advance * font_size, 2);
                result[i].image.width = result[i].advance;
                result[i].image.height = font_size;
                result[i].image.mipmaps = 1;
            } else {
                result[i].image.width = width;
                result[i].image.height = height;
                result[i].image.mipmaps = 1; 
            }

            if (type == ATRA_FONT_BITMAP) {
                u8 *const pix = result[i].image.data;

                for (j = 0; j < width * height; j++) {
                    if (pix[j] < ATRA_FONT_BITMAP_ALPHA_THRESHOLD) {
                        pix[j] = 0;
                    } else {
                        pix[j] = 255;
                    }
                }
            }
        }
    }

    if (genchars) {
        free(chars);
    }

    return result;
}

/* Deletes a font's glyph data from memory */
ATRA_API
void
atra_font_deleteglyphs(Font *font, int count)
{
    int i;

    for (i = 0; i < count; i++) {
        atra_image_destroy(&font->glyphs[i].image);
    }
    free(font->glyphs);
}

/* Deletes a font, removing it from the GPU */
ATRA_API
void
atra_font_delete(Font *font)
{
    atra_font_deleteglyphs(font, font->glyph_count);
    atra_texture_destroy(&font->texture);
    free(font->quads);
}

/* Returns the dimensions of a string constructed using a particular font through `result` */
ATRA_API
void
atra_font_calctextsize(Font *font, const char *str, float font_size, float spacing, vec2 *result)
{
    int str_len = strlen(str);
    int temp_counter = 0;
    int byte_counter = 0;

    float text_width = 0.0f;
    float temp_width = 0.0f;

    float text_height = font->size;
    float scale = font_size / font->size;

    int letter = 0;
    int index = 0;
    int next, i;


    for (i = 0; i < str_len; i++) {
        byte_counter++;

        next = 0;
        letter = atra_utf_getcodepoint(str + i, &next);
        index = atra_font_getglyphindex(font, letter);
    }

    if (letter == 0x3f) {
        next = 1;
    }
    i += next - 1;

    if (letter != '\n') {
        if (font->glyphs[index].advance != 0) {
            text_width += font->glyphs[index].advance;
        } else {
            text_width += font->quads[index].w + font->glyphs[index].offset_x;
        }
    } else {
        if (temp_width < text_width) {
            temp_width = text_width;
        }
        byte_counter = 0;
        text_width = 0;
        text_height += font->size * 1.5f;   /* 1.5 line spacing */
    }

    if (temp_counter < byte_counter) {
        temp_counter = byte_counter;
    }

    *result[0] = temp_width * scale + (temp_counter - 1) * spacing;
    *result[1] = text_height * scale;
}

/* Returns the index position of a UTF-8 character in the font */
ATRA_API
int
atra_font_getglyphindex(const Font *font, int codepoint)
{
    ATRA_UNUSED(font);

    int index = 0x3f;

    /*
    int i;

    for (i = 0; i < font->glyph_count; i++) {
        if (font->glyphs[i].value == codepoint) {
            index = i;
            break;
        }
    }
    */

    index = MAX(codepoint - 32, 0);

    return index;
}

/* Returns an image containing an atlas of a given font's glyphs */
ATRA_API
Image
atra_font_generateatlas(Font *font, int packing_method)
{
    Image atlas = {0};
    Quad *quads;
    float req_area, guess_size;
    int image_size;
    int i, j, k;

    if (font->glyphs == NULL) {
        atra_log_print(LOG_WARN, "font: Provided glyph info not valid, returning empty atlas");
        return atlas;
    }
    font->quads = NULL;

    font->glyph_count = (font->glyph_count > 0) ? font->glyph_count : ATRA_FONT_DEFAULT_GLYPHCOUNT;
    quads = malloc(font->glyph_count * sizeof(Quad));
    req_area = 0;

    for (i = 0; i < font->glyph_count; i++) {
        const int req_w = font->glyphs[i].image.width + 2 * font->padding;
        const int req_h = font->glyphs[i].image.height + 2 * font->padding;
        req_area += req_w * req_h;
    }
    guess_size = sqrtf(req_area) * 1.3f;
    image_size = powf(2, ceilf(logf(guess_size) / logf(2)));

    atlas.width = image_size;
    atlas.height = image_size;
    atlas.data = calloc(1, atlas.width * atlas.height);
    atlas.mipmaps = 1;

    /* TODO: Split packing algorithms into static functions */
    if (packing_method == ATRA_FONT_PACK_DEFAULT) {

        int offset_x = font->padding;
        int offset_y = font->padding;

        for (i = 0; i < font->glyph_count; i++) {
            /* Copy pixels from the font glyphs to the atlas */
            u8 *const src = font->glyphs[i].image.data;
            u8 *const dest = atlas.data;

            for (j = 0; j < font->glyphs[i].image.height; j++) {
                const int src_y_index = j * font->glyphs[i].image.width;
                const int dest_y_index = (offset_y + j) * atlas.width;

                for (k = 0; k < font->glyphs[i].image.width; k++) {
                    const int src_index = src_y_index + k;
                    const int dest_index = dest_y_index + (offset_x + k);

                    dest[dest_index] = src[src_index];
                }
            }

            /* Fill glyph quads with atlas info */
            quads[i].x = offset_x;
            quads[i].y = offset_y;
            quads[i].w = font->glyphs[i].image.width;
            quads[i].h = font->glyphs[i].image.height;

            /* Adjust offsets to the next position in the atlas */
            offset_x += font->glyphs[i].image.width + 2 * font->padding;

            if (offset_x >= atlas.width - font->glyphs[i].image.width + 2 * font->padding) {
                offset_x = font->padding;
                offset_y += font->size + 2 * font->padding;

                if (offset_y > (atlas.height - font->size - font->padding)) {
                    break;
                }
            }
        }
    } else if (packing_method == ATRA_FONT_PACK_SKYLINE) {
        atra_log_print(LOG_WARN, "font: Skyline font packing not implemented");
    }

    /* Convert atlas to RGBA8 */
    u8 *rgb_img = malloc(atlas.width * atlas.height * sizeof(u8) * 4);

    for (i = 0, j = 0; i < atlas.width * atlas.height; i++, j += 4) {
        rgb_img[j + 0] = 255;
        rgb_img[j + 1] = 255;
        rgb_img[j + 2] = 255;
        rgb_img[j + 3] = ((u8 *)atlas.data)[i];
    }
    free(atlas.data);
    atlas.data = rgb_img;
    font->quads = quads;

    return atlas;
}

/* Draws a string to the screen using a given font */
ATRA_API
void
atra_font_drawtext(Font *font, const char *str, float x, float y,
        float font_size, float spacing, Color color)
{
    if (str == NULL) {
        return;
    }

    if (font->texture.id == 0) {
        atra_log_print(LOG_WARN, "font: Attempted to draw with an incomplete font");
    }

    int str_len = strlen(str);
    int text_offset_y = 0;
    float text_offset_x = 0.0f;
    float scale = font_size / font->size;
    int codepoint_byte_count, codepoint;
    int index, i;

    for (i = 0; i < str_len;) {
        codepoint_byte_count = 0;
        codepoint = atra_utf_getcodepoint(str + i, &codepoint_byte_count);
        index = atra_font_getglyphindex(font, codepoint);

        if (codepoint == 0x3f) codepoint_byte_count = 1;

        if (codepoint == '\n') {
            text_offset_y += (font->size + font->size / 2.0) * scale;
            text_offset_x = 0.0f;
        } else {
            if (codepoint != ' ' && codepoint != '\t') {
                const int x_pos = x + text_offset_x;
                const int y_pos = y + text_offset_y;

                atra_font_drawcodepoint(font, codepoint, x_pos, y_pos, font_size, color);
            }

            if (font->glyphs[index].advance == 0) {
                text_offset_x += font->quads[index].w * scale + spacing;
            } else {
                text_offset_x += font->glyphs[index].advance * scale + spacing;
            }
        }
        i += codepoint_byte_count;
    }
}

/* Draws a string to the screen using a given font (mesh version) */
ATRA_API
void
atra_font_drawtext_mesh(Mesh *mesh, Font *font, const char *str, float x, float y,
        float depth, float font_size, float spacing, Color color)
{
    if (str == NULL) {
        return;
    }

    if (font->texture.id == 0) {
        atra_log_print(LOG_WARN, "font: Attempted to draw with an incomplete font");
    }

    int str_len = strlen(str);
    int text_offset_y = 0;
    float text_offset_x = 0.0f;
    float scale = font_size / font->size;
    int codepoint_byte_count, codepoint;
    int index, i;

    atra_mesh_setactive(mesh);
    *mesh = atra_mesh_begin(ATRA_QUADS,
            ATRA_MESH_BUFFER_POSITION |
            ATRA_MESH_BUFFER_TEXCOORD |
            ATRA_MESH_BUFFER_COLORS);

    for (i = 0; i < str_len;) {
        codepoint_byte_count = 0;
        codepoint = atra_utf_getcodepoint(str + i, &codepoint_byte_count);
        index = atra_font_getglyphindex(font, codepoint);

        if (codepoint == 0x3f) codepoint_byte_count = 1;

        if (codepoint == '\n') {
            text_offset_y += (font->size + font->size / 2.0) * scale;
            text_offset_x = 0.0f;
        } else {
            if (codepoint != ' ' && codepoint != '\t') {
                const int x_pos = x + text_offset_x;
                const int y_pos = y + text_offset_y;

                atra_font_drawcodepoint_mesh(font, codepoint, x_pos, y_pos,
                        depth, font_size, color);
            }

            if (font->glyphs[index].advance == 0) {
                text_offset_x += font->quads[index].w * scale + spacing;
            } else {
                text_offset_x += font->glyphs[index].advance * scale + spacing;
            }
        }
        i += codepoint_byte_count;
    }
    atra_mesh_end(0);
}

/* TODO: Test functionality and add branch for line/character wrapping */
/* TODO: Add centre and right alignment */
/* Draws a string inside the a given rectangular bound (supports line wrapping) */
ATRA_API
void
atra_font_drawtextrect(Font *font, const char *str, Quad quad,
        float font_size, float spacing, int word_wrap, Color color)
{
    ATRA_UNUSED(word_wrap);

    if (str == NULL) {
        return;
    }

    const int str_len = strlen(str);
    const float scale = font_size / font->size;
    const float row_skip = (font->size + font->size / 4.0f) * scale;
    
    int codepoint_byte_count, codepoint;
    int x_pos, y_pos;
    int index, i;

    int word_start;
    float word_width;
    int row_start;
    float row_width;

    int code_count;
    int *codes;
    float *code_widths;

    code_count = 0;
    codes = malloc((str_len + 1) * sizeof(int));
    code_widths = malloc((str_len + 1) * sizeof(float));

    /* Get the number, size, width and index of each codepoint */
    for (i = 0; i < str_len;) {
        codepoint_byte_count = 0;
        codepoint = atra_utf_getcodepoint(str + i, &codepoint_byte_count);
        index = atra_font_getglyphindex(font, codepoint);

        if (codepoint == 0x3f) {
            codepoint_byte_count = 1;
        }
        codes[code_count] = codepoint;
        code_widths[code_count] = spacing + scale * 
                DECIDE(font->glyphs[index].advance != 0,
                    font->glyphs[index].advance, font->quads[index].w);

        code_count++;
        i += codepoint_byte_count;
    }
    word_start = 0;
    word_width = 0.0f;
    row_start = 0;
    row_width = 0.0f;
    x_pos = quad.x;
    y_pos = quad.y;
    
    for (i = 0; i < code_count; i++) {
        
        /* Handle newline character */
        if (codes[i] == '\n') {
            /* Draw current row */
            x_pos = quad.x;

            for (; row_start < i; row_start++) {
                if (!atra_utf_iscodepointbreaking(codes[row_start]) &&
                        codes[row_start] != '\t') {
                    atra_font_drawcodepoint(font,
                            codes[row_start], x_pos, y_pos, font_size, color);
                }
                x_pos += code_widths[row_start];
            }

            /* Move to next row */
            y_pos += row_skip;
            word_width = 0.0f;
            row_width = 0.0f;
            row_start = i + 1;
            continue;
        } else if (codes[i] < 0x20) {
            /* TODO: Handle other undrawable crap like '\t' */
        }
        word_width += code_widths[i];

        /* Force character wrap if word is longer than line */
        if ((word_width > quad.w)) {
            /* Draw current row */
            x_pos = quad.x;

            for (; row_start < i; row_start++) {
                if (!atra_utf_iscodepointbreaking(codes[row_start]) &&
                        codes[row_start] != '\t') {
                    atra_font_drawcodepoint(font,
                            codes[row_start], x_pos, y_pos, font_size, color);
                }
                x_pos += code_widths[row_start];
            }

            /* Move to next row */
            y_pos += row_skip;
            word_width = code_widths[i];
            row_width = 0.0f;
        }

        /* Draw the rest of the text if this is the last codepoint */
        if (i == code_count - 1) {
            /* Draw current row */
            x_pos = quad.x;

            for (; row_start <= i; row_start++) {
                if (!atra_utf_iscodepointbreaking(codes[row_start]) &&
                        codes[row_start] != '\t') {
                    atra_font_drawcodepoint(font,
                            codes[row_start], x_pos, y_pos, font_size, color);
                }
                x_pos += code_widths[row_start];
            }
            break;
        }

        /* Add word to row if breaking character is found */
        if (atra_utf_iscodepointbreaking(codes[i])) {
            /* Check that word fits in quad (row_width + width) */
            if ((row_width + word_width > quad.w)) {
                /* Draw current row */
                x_pos = quad.x;

                for (; row_start <= word_start; row_start++) {
                    if (!atra_utf_iscodepointbreaking(codes[row_start]) &&
                            codes[row_start] != '\t') {
                        atra_font_drawcodepoint(font,
                                codes[row_start], x_pos, y_pos, font_size, color);
                    }
                    x_pos += code_widths[row_start];
                }

                /* Move to next row */
                y_pos += row_skip;
                word_start = i;
                row_width = word_width;
                word_width = 0.0f;

            } else {
                row_width += word_width;
                word_start = i;
                word_width = 0.0f;
            }
        }
    }
    free(codes);
    free(code_widths);
}

/* Draws a string inside the a given rectangular bound (mesh version) */
ATRA_API
void
atra_font_drawtextrect_mesh(Mesh *mesh, Font *font, const char *str, Quad quad,
        float depth, float font_size, float spacing, int word_wrap, Color color)
{
    ATRA_UNUSED(word_wrap);

    *mesh = atra_mesh_begin(ATRA_QUADS,
            ATRA_MESH_BUFFER_POSITION |
            ATRA_MESH_BUFFER_TEXCOORD |
            ATRA_MESH_BUFFER_COLORS);
    atra_mesh_setactive(mesh);

    if (str == NULL) {
        atra_mesh_end(FALSE);
        return;
    }

    if (font->texture.id == 0) {
        atra_log_print(LOG_WARN, "font: Attempted to draw with an incomplete font");
        return;
    }
    
    const int str_len = strlen(str);
    const float scale = font_size / font->size;
    const float row_skip = (font->size + font->size * 0.25f) * scale;

    int codepoint_byte_count, codepoint;
    int x_pos, y_pos;
    int index, i;

    int word_start;
    float word_width;
    int row_start;
    float row_width;

    int code_count;
    int *codes;
    float *code_widths;

    code_count = 0;
    codes = malloc((str_len + 1) * sizeof(int));
    code_widths = malloc((str_len + 1) * sizeof(float));
 
    /* Get the number, size, width and index of each codepoint */
    for (i = 0; i < str_len;) {
        codepoint_byte_count = 0;
        codepoint = atra_utf_getcodepoint(str + i, &codepoint_byte_count);
        index = atra_font_getglyphindex(font, codepoint);

        if (codepoint == 0x3f) {
            codepoint_byte_count = 1;
        }
        codes[code_count] = codepoint;
        code_widths[code_count] = spacing + scale * 
                DECIDE(font->glyphs[index].advance != 0,
                    font->glyphs[index].advance, font->quads[index].w);

        code_count++;
        i += codepoint_byte_count;
    }
    word_start = 0;
    word_width = 0.0f;
    row_start = 0;
    row_width = 0.0f;
    x_pos = quad.x;
    y_pos = quad.y;
    
    for (i = 0; i < code_count; i++) {
        
        /* Handle newline character */
        if (codes[i] == '\n') {
            /* Draw current row */
            x_pos = quad.x;

            for (; row_start < i; row_start++) {
                if (!atra_utf_iscodepointbreaking(codes[row_start]) &&
                        codes[row_start] != '\t') {
                    atra_font_drawcodepoint_mesh(font,
                            codes[row_start], x_pos, y_pos, depth, font_size, color);
                }
                x_pos += code_widths[row_start];
            }
            /* Move to next row */
            y_pos += row_skip;
            word_width = 0.0f;
            row_width = 0.0f;
            row_start = i + 1;

            continue;
        } else if (codes[i] < 0x20) {
            /* TODO: Handle other undrawable crap like '\t' */
        }
        word_width += code_widths[i];

        /* Force character wrap if word is longer than line */
        if ((word_width > quad.w)) {
            /* Draw current row */
            x_pos = quad.x;

            for (; row_start < i; row_start++) {
                if (!atra_utf_iscodepointbreaking(codes[row_start]) &&
                        codes[row_start] != '\t') {
                    atra_font_drawcodepoint_mesh(font,
                            codes[row_start], x_pos, y_pos, depth, font_size, color);
                }
                x_pos += code_widths[row_start];
            }
            /* Move to next row */
            y_pos += row_skip;
            word_width = code_widths[i];
            row_width = 0.0f;
        }

        /* Draw the rest of the text if this is the last codepoint */
        if (i == code_count - 1) {
            /* Check for wrapping on the current row */
            if (row_width + word_width > quad.w) {
                x_pos = quad.x;

                for (; row_start <= word_start; row_start++) {
                    if (!atra_utf_iscodepointbreaking(codes[row_start]) &&
                            codes[row_start] != '\t') {
                        atra_font_drawcodepoint_mesh(font,
                                codes[row_start], x_pos, y_pos, depth, font_size, color);
                    }
                    x_pos += code_widths[row_start];
                }
                /* Move to next row */
                y_pos += row_skip;
                word_start = i;
                row_width = word_width;
                word_width = 0.0f;
            }
            /* Draw any remaining codepoints */
            x_pos = quad.x;

            for (; row_start <= i; row_start++) {
                if (!atra_utf_iscodepointbreaking(codes[row_start]) &&
                        codes[row_start] != '\t') {
                    atra_font_drawcodepoint_mesh(font,
                            codes[row_start], x_pos, y_pos, depth, font_size, color);
                }
                x_pos += code_widths[row_start];
            }
            break;
        }

        /* Add word to row if breaking character is found */
        if (atra_utf_iscodepointbreaking(codes[i])) {
            /* Check that word fits in quad (row_width + width) */
            if (row_width + word_width - code_widths[i] > quad.w) {
                /* Draw current row */
                x_pos = quad.x;

                for (; row_start <= word_start; row_start++) {
                    if (!atra_utf_iscodepointbreaking(codes[row_start]) &&
                            codes[row_start] != '\t') {
                        atra_font_drawcodepoint_mesh(font,
                                codes[row_start], x_pos, y_pos, depth, font_size, color);
                    }
                    x_pos += code_widths[row_start];
                }
                /* Move to next row */
                y_pos += row_skip;
                word_start = i;
                row_width = word_width;
                word_width = 0.0f;
            } else {
                row_width += word_width;
                word_start = i;
                word_width = 0.0f;
            }
        }
    }
    atra_mesh_end(FALSE);

    free(codes);
    free(code_widths);
}

/* Draws a single codepoint character to the screen */
ATRA_API
void
atra_font_drawcodepoint(Font *font, int codepoint, float x, float y,
        float font_size, Color color)
{
    float scale = font_size / font->size;
    float width, height;
    int index = atra_font_getglyphindex(font, codepoint);

    Quad dest_quad = {
        x + font->glyphs[index].offset_x * scale - font->padding * scale,
        y + font->glyphs[index].offset_y * scale - font->padding * scale,
        (font->quads[index].w + 2.0f * font->padding) * scale,
        (font->quads[index].h + 2.0f * font->padding) * scale};
    
    Quad src_quad = {
        font->quads[index].x - font->padding,
        font->quads[index].y - font->padding,
        font->quads[index].w + 2.0f * font->padding,
        font->quads[index].h + 2.0f * font->padding};

    width = font->texture.width;
    height = font->texture.height;

    vec2 top_left = {0};
    vec2 top_right = {0};
    vec2 bot_left = {0};
    vec2 bot_right = {0};

    float dst_x = dest_quad.x;
    float dst_y = dest_quad.y;

    top_left[0] = dst_x; top_left[1] = dst_y;
    top_right[0] = dst_x + dest_quad.w; top_right[1] = dst_y;
    bot_left[0] = dst_x; bot_left[1] = dst_y + dest_quad.h;
    bot_right[0] = dst_x + dest_quad.w; bot_right[1] = dst_y + dest_quad.h;

    atra_gl_batch_checklimit(4);
    atra_gl_batch_settexture(font->texture.id);

    atra_gl_begin(ATRA_QUADS);
    {
        atra_gl_color4ub(color.r, color.g, color.b, color.a);

        atra_gl_vertex2f(top_left[0], top_left[1]);
        atra_gl_texcoord2f(src_quad.x / width, src_quad.y / height);
        
        atra_gl_vertex2f(bot_left[0], bot_left[1]);
        atra_gl_texcoord2f(src_quad.x / width, (src_quad.y + src_quad.h) / height);
        
        atra_gl_vertex2f(bot_right[0], bot_right[1]);
        atra_gl_texcoord2f((src_quad.x + src_quad.w) / width, (src_quad.y + src_quad.h) / height);

        atra_gl_vertex2f(top_right[0], top_right[1]);
        atra_gl_texcoord2f((src_quad.x + src_quad.w) / width, src_quad.y / height);
    }
    atra_gl_end();

    atra_gl_batch_settexture(0);
}

/* Draws a single codepoint character to the given mesh */
ATRA_API
void
atra_font_drawcodepoint_mesh(const Font *font, int codepoint, float x, float y, float depth,
        float font_size, Color color)
{
    float scale = font_size / font->size;
    float width, height;
    int index = atra_font_getglyphindex(font, codepoint);

    Quad dest_quad = {
        x + font->glyphs[index].offset_x * scale - font->padding * scale,
        y + font->glyphs[index].offset_y * scale - font->padding * scale,
        (font->quads[index].w + 2.0f * font->padding) * scale,
        (font->quads[index].h + 2.0f * font->padding) * scale};
    
    Quad src_quad = {
        font->quads[index].x - font->padding,
        font->quads[index].y - font->padding,
        font->quads[index].w + 2.0f * font->padding,
        font->quads[index].h + 2.0f * font->padding};

    width = font->texture.width;
    height = font->texture.height;

    vec2 top_left = {0};
    vec2 top_right = {0};
    vec2 bot_left = {0};
    vec2 bot_right = {0};

    float dst_x = dest_quad.x;
    float dst_y = dest_quad.y;

    top_left[0] = dst_x; top_left[1] = dst_y;
    top_right[0] = dst_x + dest_quad.w; top_right[1] = dst_y;
    bot_left[0] = dst_x; bot_left[1] = dst_y + dest_quad.h;
    bot_right[0] = dst_x + dest_quad.w; bot_right[1] = dst_y + dest_quad.h;

    atra_mesh_vertex3f(top_left[0], top_left[1], depth);
    atra_mesh_vertex3f(bot_left[0], bot_left[1], depth);
    atra_mesh_vertex3f(bot_right[0], bot_right[1], depth);
    atra_mesh_vertex3f(top_right[0], top_right[1], depth);
    
    atra_mesh_texcoord2f(src_quad.x / width, src_quad.y / height);
    atra_mesh_texcoord2f(src_quad.x / width, (src_quad.y + src_quad.h) / height);
    atra_mesh_texcoord2f((src_quad.x + src_quad.w) / width, (src_quad.y + src_quad.h) / height);
    atra_mesh_texcoord2f((src_quad.x + src_quad.w) / width, src_quad.y / height);

    atra_mesh_color4ub(color.r, color.g, color.b, color.a);
    atra_mesh_color4ub(color.r, color.g, color.b, color.a);
    atra_mesh_color4ub(color.r, color.g, color.b, color.a);
    atra_mesh_color4ub(color.r, color.g, color.b, color.a);
}
