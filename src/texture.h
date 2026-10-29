/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  texture.h - Image and pixel array-like data processing and texture data
 *              transmission from CPU to GPU.
 *
 *  */

#ifndef ATRA_TEXTURE_H
#define ATRA_TEXTURE_H

#include "common.h"
#include "shader.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

enum texture_sampling_style {
    ATRA_SAMPLE_NEAREST,
    ATRA_SAMPLE_LINEAR
};

enum texture_type {
    ATRA_TEXTURE_ALBEDO,
    ATRA_TEXTURE_SPECULAR,
    ATRA_TEXTURE_NORMAL,
    ATRA_TEXTURE_ROUGHNESS,
    ATRA_TEXTURE_OCCLUSION,
    ATRA_TEXTURE_EMISSION,
    ATRA_TEXTURE_HEIGHT,
    ATRA_TEXTURE_CUBEMAP,
    ATRA_TEXTURE_IRRADIANCE,
    ATRA_TEXTURE_PREFILTER,
    ATRA_TEXTURE_BRDF,
    ATRA_TEXTURE_TOTAL
};

enum gbuffer_type {
    ATRA_GBUFFER_POSITION,
    ATRA_GBUFFER_NORMAL,
    ATRA_GBUFFER_ALBEDOSPEC,
    ATRA_GBUFFER_DATA,
    ATRA_GBUFFER_TOTAL
};

enum cubemap_face {
    ATRA_CUBEMAP_POS_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    ATRA_CUBEMAP_NEG_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    ATRA_CUBEMAP_POS_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    ATRA_CUBEMAP_NEG_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    ATRA_CUBEMAP_POS_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    ATRA_CUBEMAP_NEG_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

/* A CPU/RAM bound image */
typedef struct Image {
    void *data;         /* Raw image data */
    int width, height;  /* Image dimensions */
    int mipmaps;        /* Mipmap levels for texture conversion */
} Image;

/* A GPU/VRAM bound image */
typedef struct Texture {
    uint id;            /* OpenGL texture ID */
    int width, height;  /* Texture dimensions */
    int mipmaps;        /* Mipmap levels */
    int base_sample;    /* Base texture sample mode (default: nearest) */
    int mip_sample;     /* Mipmap texture sample mode (default: nearest) */
} Texture;

/* Deferred rendering G-buffer abstraction */
typedef struct GBuffer {
    uint gbuffer;
    uint buffers[ATRA_GBUFFER_TOTAL];
    uint depth;
} GBuffer;

/* Render texture (RBO) abstraction */
typedef struct RenderTexture {
    uint id;            /* FBO ID */
    Texture color;      /* Color attachment texture */
    Texture depth;      /* Depth attachment texture */
} RenderTexture;

/* An abstraction of OpenGL's texture arrays.  Can replace texture atlases */
typedef struct TextureArray {
    uint id;            /* OpenGL texture ID */
    int width, height;  /* Texture dimensions */
    int depth;          /* Number of textures in array */
    int mipmaps;        /* Mipmap levels */
    int base_sample;    /* Base texture sample mode (default: nearest) */
    int mip_sample;     /* Mipmap texture sample mode (default: nearest) */
} TextureArray;

ATRA_OPTION_DECLARE(Image)
ATRA_OPTION_DECLARE(Texture)

/*
 *  Image Management Functions
 */

/* Creates a new image from data in a file */
ATRA_API Option_Image atra_image_loadfile(const char *path);

/* Frees memory occupied by image data */
ATRA_API void atra_image_destroy(Image *image);

/* Returns a copy of the entire image */
ATRA_API Image atra_image_copy(Image *image);

/* Returns a copy of part of an image defined by a rectangle */
ATRA_API Image atra_image_subcopy(Image *image, const IQuad rect);

/* Applies a given tint color to an image */
ATRA_API void atra_image_applytint(Image *image, Color tint);

/* Resizes an image using bilinear scaling */
ATRA_API void atra_image_scalebilinear(Image *image, int w_new, int h_new);

/* Resizes an image using nearest-neighbour scaling */
ATRA_API void atra_image_scalenearest(Image *image, int w_new, int h_new);

/* Creates an image with the specified color */
ATRA_API Image atra_image_gencolor(Color color, int w, int h);

/* Creates an image with a vertical gradient of two colors */
ATRA_API Image atra_image_gengradient_v(Color top, Color bot, int w, int h);

/* Creates an image with a horizontal gradient of two colors */
ATRA_API Image atra_image_gengradient_h(Color left, Color right, int w, int h);

/* Creates an image with a radial gradient of two colors */
ATRA_API Image atra_image_gengradient_radial(Color inner, Color outer, int w, int h);

/* Generates a white-noise image */
ATRA_API Image atra_image_genwhitenoise(int w, int h, float factor);

/* Generates a perlin noise image */
ATRA_API Image atra_image_genperlinnoise(int w, int h, int x_offset, int y_offset, float scale);

/* Returns a new image with the contents of the screen buffer */
ATRA_API Image atra_image_getscreen(void);

/* Converts a quad to a set of UV coordinates and stores them in `uvs` */
ATRA_API void atra_image_quadtouvs(const Image *image, IQuad quad, float *uvs);

/*
 *  GPU/VRAM texture functions
 */

/* Creates a new OpenGL texture reserving space one the GPU with no data */

/* Creates a new OpenGL texture from a file */
ATRA_API Option_Texture atra_texture_loadfile(const char *path);

/* Creates a new OpenGL texture from an existing image */
ATRA_API Texture atra_texture_loadimage(Image *image);

/* Binds the given texture to the chosen OpenGL texture unit */
ATRA_API void atra_texture_bind(Texture *tex, int tex_unit);

/* Binds the given cubemap texture to the chosen OpenGL texture unit */
ATRA_API void atra_texture_bindcubemap(Texture *tex, int tex_unit);

/* Removes a texture of the GPU and deletes it */
ATRA_API void atra_texture_destroy(Texture *tex);

/* Loads the given texture for rendering (framebuffer) */
ATRA_API RenderTexture atra_texture_loadrendertex(int width, int height);

/* Removes the render texture from the GPU and deletes it */
ATRA_API void atra_texture_destroyrendertex(RenderTexture *tex);

/* Sets the sampling style for the base texture level */
ATRA_API void atra_texture_samplebase(Texture *tex, int mode);

/* Sets the sampling style for texture mipmaps */
ATRA_API void atra_texture_samplemip(Texture *tex, int mode);

/* Generates mipmaps using the default OpenGL mipmapping function */
ATRA_API void atra_texture_genmipmaps(Texture *tex);

/* Sets the maximum mipmap level of the given texture. Base texture is level 0. */
ATRA_API void atra_texture_setmaxmip(Texture *tex, int level);

/* Sets the maximum anisotropy level of the given texture. */
ATRA_API void atra_texture_setanisotropy(Texture *tex, int max_aniso);

/* Write an entire texture with provided pixel array.
 * The pixel array is expected to contain as many pixels as the texture */
ATRA_API void atra_texture_write(Texture *tex, const void *pixels);

/* Similar to atra_texture_update() but only updates a specified rectangle */
ATRA_API void atra_texture_writesub(Texture *tex, int x, int y, int w, int h,
        const void *pixels);

/* Returns a new image with texture data from the GPU */
ATRA_API Image atra_texture_getdata(Texture *tex);

/* Converts a quad to a set of UV coordinates and stores them in `uvs` */
ATRA_API void atra_texture_quadtouvs(const Texture *image, IQuad quad, float *uvs);

/*
 *  Texture Array Functions 
 */


/*
 *  G-Buffer Functions
 */

/* Create a new g-buffer */
ATRA_API GBuffer atra_gbuffer_create(void);

/* Deletes a g-buffer */
ATRA_API void atra_gbuffer_delete(GBuffer *gbuffer);

/* Binds the g-buffer */
ATRA_API void atra_gbuffer_enable(GBuffer *gbuffer);

/* Unbinds the g-buffer */
ATRA_API void atra_gbuffer_disable(void);

/* Blit depth buffer to default framebuffer's depth buffer */
ATRA_API void atra_gbuffer_blitdepth(GBuffer *gbuffer);

#ifdef __cplusplus
}
#endif

#endif /* ATRA_TEXTURE_H */
