/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  framebuffer.h - Framebuffer object stuctures and functions
 *
 *  */

#ifndef ATRA_FRAMEBUFFER_H
#define ATRA_FRAMEBUFFER_H

#include "common.h"
#include "texture.h"

#define ATRA_FBO_ATTACH_COLORMAX    8

#ifdef __cplusplus
extern "C" {
#endif

enum framebuffer_texture_type {
    ATRA_FBO_TEXTYPE_NONE,
    ATRA_FBO_TEXTYPE_TEXTURE,
    ATRA_FBO_TEXTYPE_RENDERTEX
};

enum framebuffer_texture_format {
    ATRA_RGBA               = GL_RGBA,
    ATRA_RGBA_16F           = GL_RGBA16F,
    ATRA_DEPTH_COMPONENT    = GL_DEPTH_COMPONENT,
    ATRA_STENCIL_INDEX      = GL_STENCIL_INDEX
};

enum framebuffer_attach_slot {
    ATRA_FBO_ATTACH_COLOR0,
    ATRA_FBO_ATTACH_COLOR1,
    ATRA_FBO_ATTACH_COLOR2,
    ATRA_FBO_ATTACH_COLOR3,
    ATRA_FBO_ATTACH_COLOR4,
    ATRA_FBO_ATTACH_COLOR5,
    ATRA_FBO_ATTACH_COLOR6,
    ATRA_FBO_ATTACH_COLOR7,
    ATRA_FBO_ATTACH_DEPTH,
    ATRA_FBO_ATTACH_STENCIL,
    ATRA_FBO_ATTACH_MAX
};


/* Framebuffer object abstraction structure */
typedef struct FrameBuffer {
    uint id;                                    /* Framebuffer object ID */
    uint valid;                                 /* FBO validation flag */
    uint width;                                 /* Width of FBO attachments */
    uint height;                                /* Height of FBO attachments */
    uint types[ATRA_FBO_ATTACH_MAX];            /* Type of each attached texture */
    uint textures[ATRA_FBO_ATTACH_MAX];         /* Textures attached to FBO */
    uint format[ATRA_FBO_ATTACH_MAX];           /* External texture format */
    uint has_border[ATRA_FBO_ATTACH_MAX];       /* White border clamping flag */
    uint internal_format[ATRA_FBO_ATTACH_MAX];  /* Internal texture format */
    uint data_type[ATRA_FBO_ATTACH_MAX];        /* Data type (e.g. FLOAT, UNSIGNED BYTE, etc.) */
} FrameBuffer;


/* Create a new, empty FBO */
ATRA_API FrameBuffer atra_framebuffer_create(uint width, uint height);

/* Unbind, clear and delete an existing FBO */
ATRA_API void atra_framebuffer_delete(FrameBuffer *fb);

/* Add a new texture and attach it to the given attachment slot of the FBO */
ATRA_API void atra_framebuffer_add(FrameBuffer *fb, uint type, uint attach_slot,
        uint internal_format, uint format, uint data_type, uint clamp_border);

/* Detach and delete the texture at the given attachment slot of the FBO */
ATRA_API void atra_framebuffer_remove(FrameBuffer *fb, uint attach_slot);

/* Detach and delete all textures in the FBO */
ATRA_API void atra_framebuffer_clear(FrameBuffer *fb);

/* Resizes an FBO by binding new textures (will erase existing FBO data) */
ATRA_API void atra_framebuffer_resize(FrameBuffer *fb, uint width, uint height);

/* Bind the FBO for drawing to contained textures/RBOs */
ATRA_API void atra_framebuffer_bind(FrameBuffer *fb);

/* Unbind the currently bound FBO */
ATRA_API void atra_framebuffer_unbind(void);

/* Bind a FBO texture at the given slot to the given texture unit */
ATRA_API void atra_framebuffer_bind_texunit(FrameBuffer *fb, uint attach_slot, uint tex_unit);

/* Check and validate the given FBO, enabling it's use.
 * Returns true on success, otherwise false. */
ATRA_API int atra_framebuffer_finalize(FrameBuffer *fb);


#ifdef __cplusplus
extern "C" {
#endif

#endif /* ATRA_FRAMEBUFFER_H */
