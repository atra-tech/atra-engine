/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  texture.c - Image and pixel array-like data processing and texture data
 *              transmission from CPU to GPU.
 *
 *  */

#include "atra.h"
#include "stb/stb_perlin.h"

#define mix(a, b, t) ((a) + ((b) - (a)) * (t))
#define clamp(x, min, max) (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))
#define fract(x) ((x) - SDL_floorf(x));

/* Converts an RGB image to RGBA */
ATRA_INLINE
void
atra_image_addalpha(void *src, void *dst, int w, int h)
{
    int i, j;
    i8 *a, *b;
    i8 data[4];

    a = src;
    b = dst;
    data[3] = 0xFF;

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            b[0] = a[0];
            b[1] = a[1];
            b[2] = a[2];
            b[3] = 0xFF;
            a += 3;
            b += 4;
        }
    }
}

/*
 *  Image Management Functions
 */

/* Creates a new image from data in a file */
ATRA_API
Image
atra_image_loadfile(const char *path)
{
    Image image = {0};
    SDL_Surface *surface = IMG_Load(path);

    if (!surface) {
        atra_log_print(LOG_ERROR, "image: %s", IMG_GetError());
        return image;
    }
    image.width = surface->w;
    image.height = surface->h;
    image.mipmaps = 1;
    image.data = malloc(image.width * image.height * 4);
    
    if (surface->format->BytesPerPixel == 4) {
        memcpy(image.data, surface->pixels, image.width * image.height * 4);
    } else {
        atra_image_addalpha(surface->pixels, image.data, image.width, image.height);
    }
    SDL_FreeSurface(surface);

    return image;
}

/* Frees memory occupied by image data */
ATRA_API
void
atra_image_destroy(Image *image)
{
    free(image->data);
}

/* Returns a copy of the entire image */
ATRA_API
Image
atra_image_copy(Image *image)
{
    Image new_img;

    new_img = *image;
    new_img.data = malloc(image->width * image->height * 4);
    memcpy(new_img.data, image->data, image->width * image->height * 4);

    return new_img;
}

/* Returns a copy of part of an image defined by a rectangle */
ATRA_API
Image
atra_image_subcopy(Image *image, const IQuad rect)
{
    Image new_img;
    i8 *src, *dst;
    int i;

    new_img.data = malloc(rect.w * rect.h * 4);
    new_img.width = rect.w;
    new_img.height = rect.h;
    new_img.mipmaps = 1;

    src = image->data;
    dst = new_img.data;

    for (i = rect.y; i < rect.y + rect.h; i++) {
        memcpy(dst, src, rect.w * 4);
        src += rect.w * 4;
        dst += rect.w * 4;
    }

    return new_img;
}

/* Applies a given tint color to an image */
ATRA_API
void
atra_image_applytint(Image *image, Color tint)
{
    i8 *pixels;
    int i, j;
    float cr, cg, cb, ca;

    /* Full-white tint has no effect, return early. */
    if (tint.r == 0xFF && tint.g == 0xFF && tint.b == 0xFF && tint.a == 0xFF) {
        return;
    }

    /* Check that image is valid */
    if (image->data == NULL || image->width == 0 || image->height == 0) {
        return;
    }
    pixels = image->data;

    cr = tint.r / 255.0f;
    cg = tint.g / 255.0f;
    cb = tint.b / 255.0f;
    ca = tint.a / 255.0f;

    for (i = 0; i < image->height; i++) {
        for (j = 0; j < image->width; j++) {
            const int index = (j + i * image->width) * 4;

            pixels[index + 0] = (pixels[index + 0] / 255.0f * cr) * 255.0f;
            pixels[index + 1] = (pixels[index + 1] / 255.0f * cg) * 255.0f;
            pixels[index + 2] = (pixels[index + 2] / 255.0f * cb) * 255.0f;
            pixels[index + 3] = (pixels[index + 3] / 255.0f * ca) * 255.0f;
        }
    }
}

/* Resizes an image using bilinear scaling */
ATRA_API
void
atra_image_scalebilinear(Image *restrict image, int w_new, int h_new)
{
    const float new_pix_w = 1.0f / w_new;
    const float new_pix_h = 1.0f / h_new;

    int i, j;
    int x0, x1, y0, y1;
    float a, cx, cy, norm_x, norm_y;
    i8 *new_data;
    i8 *x0y0, *x1y0, *x0y1, *x1y1;

    new_data = malloc(w_new * h_new * 4);

    for (i = 0; i < h_new; i++) {
        norm_y = i * new_pix_h;

        for (j = 0; j < w_new; j++) {
            norm_x = j * new_pix_w;

            cx = norm_x * image->width;
            cy = norm_y * image->height;
            x1 = clamp(cx + 0.5, 0, image->width - 1);
            y1 = clamp(cy + 0.5, 0, image->width - 1);
            x0 = clamp(cx - 0.5, 0, image->width - 1);
            y0 = clamp(cy - 0.5, 0, image->width - 1);

            x0y0 = (i8 *)image->data + (x0 + y0 * image->width) * 4;
            x1y0 = (i8 *)image->data + (x1 + y0 * image->width) * 4;
            x0y1 = (i8 *)image->data + (x0 + y1 * image->width) * 4;
            x1y1 = (i8 *)image->data + (x1 + y1 * image->width) * 4;
            
            a = fract(cx - 0.5);

            float lerp_y0[4] = {
                mix(x0y0[0], x1y0[0], a),
                mix(x0y0[1], x1y0[1], a),
                mix(x0y0[2], x1y0[2], a),
                mix(x0y0[3], x1y0[3], a)
            };

            float lerp_y1[4] = {
                mix(x0y1[0], x1y1[0], a),
                mix(x0y1[1], x1y1[1], a),
                mix(x0y1[2], x1y1[2], a),
                mix(x0y1[3], x1y1[3], a)
            };

            a = fract(cy - 0.5);

            float lerp_x[4] = {
                mix(lerp_y0[0], lerp_y1[0], a),
                mix(lerp_y0[1], lerp_y1[1], a),
                mix(lerp_y0[2], lerp_y1[2], a),
                mix(lerp_y0[3], lerp_y1[3], a),
            };

            memcpy(new_data + (j + i * w_new) * 4, lerp_x, 4);
        }
    }
    free(image->data);
    image->data = new_data;
    image->width = w_new;
    image->height = h_new;
}

/* Resizes an image using nearest-neighbour scaling */
ATRA_API
void
atra_image_scalenearest(Image *image, int w_new, int h_new)
{
    const float new_pix_w = 1.0f / w_new;
    const float new_pix_h = 1.0f / h_new;

    int i, j;
    int cx, cy;
    float norm_x, norm_y;
    i8 *new_data;

    new_data = malloc(w_new * h_new * 4);

    for (i = 0; i < h_new; i++) {
        norm_y = i * new_pix_h;

        for (j = 0; j < w_new; j++) {
            norm_x = j * new_pix_w;

            cx = norm_x * image->width;
            cy = norm_y * image->height;

            memcpy(new_data + (j + i * w_new) * 4,
                    (i8 *)image->data + (cx + cy * image->width) * 4, 4);
        }
    }
    free(image->data);
    image->data = new_data;
    image->width = w_new;
    image->height = h_new;

}

/* Creates an image with the specified color */
ATRA_API
Image
atra_image_gencolor(Color color, int w, int h)
{
    Image image;
    i8 *data;
    int i;

    image.width = w;
    image.height = h;
    image.data = malloc(w * h * 4);
    image.mipmaps = 1;

    data = image.data;

    for (i = 0; i < w * h; i++) {
        data[0] = color.r;
        data[1] = color.g;
        data[2] = color.b;
        data[3] = color.a;
        data += 4;
    }

    return image;
}

/* Creates an image with a vertical gradient of two colors */
ATRA_API
Image
atra_image_gengradient_v(Color top, Color bot, int w, int h)
{
    const float h_inv = 1.0 / h;

    Image image;
    i8 *data;
    int i, j;
    i8 lerp_y[4];
    float degree;

    image.width = w;
    image.height = h;
    image.data = malloc(w * h * 4);
    image.mipmaps = 1;

    data = image.data;

    for (i = 0; i < h; i++) {
        degree = i * h_inv;
        lerp_y[0] = mix(top.r, bot.r, degree);
        lerp_y[1] = mix(top.g, bot.g, degree);
        lerp_y[2] = mix(top.b, bot.b, degree);
        lerp_y[3] = mix(top.a, bot.a, degree);

        for (j = 0; j < w; j++) {
            data[0] = lerp_y[0];
            data[1] = lerp_y[1];
            data[2] = lerp_y[2];
            data[3] = lerp_y[3];
            data += 4;
        }
    }

    return image;
}

/* Creates an image with a horizontal gradient of two colors */
ATRA_API
Image
atra_image_gengradient_h(Color left, Color right, int w, int h)
{
    const float w_inv = 1.0 / w;

    Image image;
    i8 *row_data, *rd, *data;
    float degree;
    int i;

    image.width = w;
    image.height = h;
    image.data = malloc(w * h * 4);
    image.mipmaps = 1;

    row_data = malloc(w * 4);
    rd = row_data;

    for (i = 0; i < w ; i++) {
        degree = i * w_inv;
        rd[0] = mix(left.r, right.r, degree);
        rd[1] = mix(left.g, right.g, degree);
        rd[2] = mix(left.b, right.b, degree);
        rd[3] = mix(left.a, right.a, degree);
        rd += 4;
    }
    data = image.data;

    for (i = 0; i < h; i++) {
        memcpy(data, row_data, w * 4);
        data += w * 4;
    }
    free(row_data);

    return image;
}

/* Creates an image with a radial gradient of two colors */
ATRA_API
Image
atra_image_gengradient_radial(Color inner, Color outer, int w, int h)
{

}

/* Generates a white noise image */
ATRA_API
Image
atra_image_genwhitenoise(int w, int h, float factor)
{
    Image image;
    i32 *data;
    int i;

    image.width = w;
    image.height = h;
    image.mipmaps = 1;
    image.data = malloc(w * h * 4);

    data = image.data;

    for (i = 0; i < w * h; i++, data++) {
        if (atra_util_randomint(0, 99) < (int)(factor * 100.0f)) {
            *data = 0xFFFFFFFF;     /* White color */
        } else {
            *data = 0xFF000000;     /* Black color */
        }
    }

    return image;
}

/* Generates a perlin noise image */
ATRA_API
Image
atra_image_genperlinnoise(int w, int h, int x_offset, int y_offset, float scale)
{
    Image image;
    u8 *data;
    float nx, ny, p;
    int i, j, intensity;

    image.width = w;
    image.height = h;
    image.mipmaps = 1;
    image.data = malloc(w * h * 4);

    data = image.data;

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            nx = (float)(j + x_offset) * scale / (float)w;
            ny = (float)(i + y_offset) * scale / (float)h;

            p = (stb_perlin_fbm_noise3(nx, ny, 1.0f, 2.0f, 0.5f, 6) + 1.0f) * 0.5f;
            intensity = p * 255.0f;

            data[0] = intensity;
            data[1] = intensity;
            data[2] = intensity;
            data[3] = 0xFF;

            data += 4;
        }
    }

    return image;
}

/* Returns a new image with the contents of the screen buffer */
ATRA_API
Image
atra_image_getscreen(void)
{
    Image image;
    i8 *scr_data, *img_data;
    int i, j;

    image.width = atra_window_getwidth();
    image.height = atra_window_getheight();
    image.mipmaps = 1;
    image.data = malloc(image.width * image.height * 4);
    scr_data = malloc(image.width * image.height * 4);
    img_data = image.data;
    
    glReadPixels(0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, scr_data);

    for (i = image.height - 1; i >= 0; i--) {
        for (j = 0; j < image.width; j++) {
            const int scr_index = j + i * image.width * 4;
            const int img_index = j + (image.height - i - 1) * image.width * 4;

            img_data[img_index] = scr_data[scr_index];

            if ((j + 1) % 4 == 0) {
                img_data[img_index] = 0xFF;
            }
        }
    }
    free(scr_data);

    return image;
}

/* Converts a quad to a set of UV coordinates and stores them in `uvs` */
ATRA_API
void
atra_image_quadtouvs(const Image *image, IQuad quad, float *uvs)
{
    const float pix_w = 1.0f / image->width;
    const float pix_h = 1.0f / image->height;
    const float offset_x = pix_w * 0.001f;
    const float offset_y = pix_h * 0.001f;

    const float x0 = quad.x * pix_w + offset_x;
    const float y0 = quad.y * pix_h + offset_y;
    const float x1 = (quad.x + quad.w) * pix_w - offset_x;
    const float y1 = (quad.y + quad.y) * pix_h - offset_y;

    uvs[0] = x0; uvs[1] = y1;
    uvs[2] = x0; uvs[3] = y0;
    uvs[4] = x1; uvs[5] = y0;
    uvs[5] = x1; uvs[7] = y1;
}

/*
 *  GPU/VRAM texture functions
 */

/* Creates a new OpenGL texture from a file */
ATRA_API
Texture
atra_texture_loadfile(const char *path)
{
    Texture tex = {0};
    Image image;

    image = atra_image_loadfile(path);

    tex.width = image.width;
    tex.height = image.height;
    tex.mipmaps = 1;
    tex.base_sample = ATRA_SAMPLE_NEAREST;
    tex.mip_sample = ATRA_SAMPLE_NEAREST;

    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glTexImage2D(GL_TEXTURE_2D,
            0, GL_RGBA, tex.width, tex.height,
            0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);

    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

/* Creates a new OpenGL texture from an existing image */
ATRA_API
Texture
atra_texture_loadimage(Image *image)
{
    Texture tex = {0};
    
    tex.width = image->width;
    tex.height = image->height;
    tex.mipmaps = image->mipmaps;
    tex.base_sample = ATRA_SAMPLE_NEAREST;
    tex.mip_sample = ATRA_SAMPLE_NEAREST;

    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glTexImage2D(GL_TEXTURE_2D,
            0, GL_RGBA, tex.width, tex.height,
            0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, tex.mipmaps);
    
    if (tex.mipmaps > 1) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

/* Binds the given texture to the chosen OpenGL texture unit */
ATRA_API
void
atra_texture_bind(Texture *tex, int tex_unit)
{
    glActiveTexture(tex_unit);
    glBindTexture(GL_TEXTURE_2D, tex->id);
}

/* Binds the given cubemap texture to the chosen OpenGL texture unit */
ATRA_API
void
atra_texture_bindcubemap(Texture *tex, int tex_unit)
{
    glEnable(GL_TEXTURE_CUBE_MAP);
    glActiveTexture(tex_unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex->id);
}

/* Loads the given texture for rendering (framebuffer) */
ATRA_API
RenderTexture
atra_texture_loadrendertex(int width, int height)
{
    RenderTexture tex = {0};

    tex.id = atra_gl_fbo_create();

    if (tex.id > 0) {
        atra_gl_fbo_enable(tex.id);

        tex.color.id = atra_gl_tex_create(NULL, width, height, 1);
        tex.color.width = width;
        tex.color.height = height;
        tex.color.mipmaps = 1;

        tex.depth.id = atra_gl_tex_createdepth(width, height, 1);
        tex.depth.width = width;
        tex.depth.height = height;
        tex.depth.mipmaps = 1;

        atra_gl_fbo_attach(tex.id, tex.color.id,
                ATRA_ATTACHMENT_COLOR_CHANNEL0, ATRA_ATTACHMENT_TEXTURE_2D, 0);
        atra_gl_fbo_attach(tex.id, tex.depth.id,
                ATRA_ATTACHMENT_DEPTH, ATRA_ATTACHMENT_RENDERBUFFER, 0);
        
        if (atra_gl_fbo_iscomplete(tex.id)) {
            /* TODO: Put some message here */
        }
        atra_gl_fbo_disable();
    }

    return tex;
}

/* Removes a texture of the GPU and deletes it */
ATRA_API
void
atra_texture_destroy(Texture *tex)
{
    glDeleteTextures(1, &tex->id);
}

/* Removes the render texture from the GPU and deletes it */
ATRA_API
void
atra_texture_destroyrendertex(RenderTexture *tex)
{
    int i;

    if (tex->id > 0) {
        atra_gl_tex_delete(tex->color.id);
        atra_gl_fbo_delete(tex->id);
    }
}

/* Sets the sampling style for the base texture level */
ATRA_API
void
atra_texture_samplebase(Texture *tex, int mode)
{
    int mipmode = tex->mip_sample;
    int m;

    tex->base_sample = mode;

    if (tex->mipmaps > 1) {
        m = (mode == ATRA_SAMPLE_NEAREST) ?
            ((mipmode == ATRA_SAMPLE_NEAREST) ?
                GL_NEAREST_MIPMAP_NEAREST :
                GL_NEAREST_MIPMAP_LINEAR) :
            ((mipmode == ATRA_SAMPLE_NEAREST) ?
                GL_LINEAR_MIPMAP_NEAREST :
                GL_LINEAR_MIPMAP_LINEAR);
    } else {
        m = (mode == ATRA_SAMPLE_NEAREST) ? GL_NEAREST : GL_LINEAR;
    }

    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m);
}

/* Sets the sampling style for texture mipmaps */
ATRA_API
void
atra_texture_samplemip(Texture *tex, int mode)
{
    int basemode = tex->base_sample;
    int m;

    tex->mip_sample = mode;

    if (tex->mipmaps > 1) {
        m = (mode == ATRA_SAMPLE_NEAREST) ?
            ((basemode == ATRA_SAMPLE_NEAREST) ?
                GL_NEAREST_MIPMAP_NEAREST :
                GL_LINEAR_MIPMAP_NEAREST) :
            ((basemode == ATRA_SAMPLE_NEAREST) ?
                GL_NEAREST_MIPMAP_LINEAR :
                GL_LINEAR_MIPMAP_LINEAR);

        glBindTexture(GL_TEXTURE_2D, tex->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m);
    }
}

/* Generates mipmaps using the default OpenGL mipmapping function */
ATRA_API
void
atra_texture_genmipmaps(Texture *tex)
{
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glGenerateMipmap(GL_TEXTURE_2D);
}

/* Sets the maximum mipmap level of the given texture. Base texture is level 1. */
ATRA_API
void
atra_texture_setmaxmip(Texture *tex, int level)
{
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level);
}

/* Sets the maximum anisotropy level of the given texture. */
ATRA_API
void
atra_texture_setanisotropy(Texture *tex, int max_aniso)
{
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)max_aniso);
}

/* Write an entire texture with provided pixel array.
 * The pixel array is expected to contain as many pixels as the texture */
ATRA_API
void
atra_texture_write(Texture *tex, const void *pixels)
{
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
            tex->width, tex->height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

/* Similar to atra_texture_update() but only updates a specified rectangle */
ATRA_API
void
atra_texture_writesub(Texture *tex, int x, int y, int w, int h, const void *pixels)
{
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

/* Returns a new image with texture data from the GPU */
ATRA_API
Image
atra_texture_getdata(Texture *tex)
{
    Image image;

    image.data = malloc(tex->width * tex->height * 4);

    glBindTexture(GL_TEXTURE_2D, tex->id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &image.width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &image.height);
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, &image.mipmaps);

    return image;
}

/* Converts a quad to a set of UV coordinates and stores them in `uvs` */
ATRA_API
void
atra_texture_quadtouvs(const Texture *tex, IQuad quad, float *uvs)
{
    const float pix_w = 1.0f / tex->width;
    const float pix_h = 1.0f / tex->height;
    const float offset_x = pix_w * 0.001f;
    const float offset_y = pix_h * 0.001f;

    const float x0 = quad.x * pix_w + offset_x;
    const float y0 = quad.y * pix_h + offset_y;
    const float x1 = (quad.x + quad.w) * pix_w - offset_x;
    const float y1 = (quad.y + quad.y) * pix_h - offset_y;

    uvs[0] = x0; uvs[1] = y1;
    uvs[2] = x0; uvs[3] = y0;
    uvs[4] = x1; uvs[5] = y0;
    uvs[5] = x1; uvs[7] = y1;
}
