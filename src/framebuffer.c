/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  framebuffer.c - Framebuffer object stuctures and functions
 *
 *  */

#include "atra.h"
#include "framebuffer.h"


/* Create a new, empty FBO */
ATRA_API
FrameBuffer
atra_framebuffer_create(uint width, uint height)
{
    FrameBuffer fb = {0};

    fb.width = width;
    fb.height = height;
    glGenFramebuffers(1, &fb.id);

    return fb;
}

/* Unbind, clear and delete an existing FBO */
ATRA_API
void
atra_framebuffer_delete(FrameBuffer *fb)
{
    atra_framebuffer_clear(fb);
    glDeleteFramebuffers(1, &fb->id);
}

/* Add a new texture and attach it to the given attachment slot of the FBO */
ATRA_API
void
atra_framebuffer_add(FrameBuffer *fb, uint tex_type, uint attach_slot,
        uint internal_format, uint format, uint data_type, uint clamp_border)
{
    float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    uint * const tex = &fb->textures[attach_slot];
    
    if (tex_type == ATRA_FBO_TEXTYPE_TEXTURE) {
        glGenTextures(1, tex);
        glBindTexture(GL_TEXTURE_2D, *tex);
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, fb->width, fb->height, 0, format, data_type, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (clamp_border) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
        }
    } else if (tex_type == ATRA_FBO_TEXTYPE_RENDERTEX) {
        glGenRenderbuffers(1, tex);
        glBindRenderbuffer(GL_RENDERBUFFER, *tex);
        glRenderbufferStorage(GL_RENDERBUFFER, internal_format, fb->width, fb->height);
    }
    fb->types[attach_slot] = tex_type;
    fb->format[attach_slot] = format;
    fb->internal_format[attach_slot] = internal_format;
    fb->data_type[attach_slot] = data_type;

    glBindFramebuffer(GL_FRAMEBUFFER, fb->id);

    switch (attach_slot) {
    case ATRA_FBO_ATTACH_COLOR0:
    case ATRA_FBO_ATTACH_COLOR1:
    case ATRA_FBO_ATTACH_COLOR2:
    case ATRA_FBO_ATTACH_COLOR3:
    case ATRA_FBO_ATTACH_COLOR4:
    case ATRA_FBO_ATTACH_COLOR5:
    case ATRA_FBO_ATTACH_COLOR6:
    case ATRA_FBO_ATTACH_COLOR7:
        if (tex_type == ATRA_FBO_TEXTYPE_TEXTURE) {
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0 + attach_slot,
                    GL_TEXTURE_2D,
                    *tex, 0);
        } else if (tex_type == ATRA_FBO_TEXTYPE_RENDERTEX) {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0 + attach_slot,
                    GL_RENDERBUFFER,
                    *tex);
        }
        break;
    case ATRA_FBO_ATTACH_DEPTH:
        if (tex_type == ATRA_FBO_TEXTYPE_TEXTURE) {
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    GL_TEXTURE_2D,
                    *tex, 0);
        } else if (tex_type == ATRA_FBO_TEXTYPE_RENDERTEX) {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    GL_RENDERBUFFER,
                    *tex);
        }
        break;
    case ATRA_FBO_ATTACH_STENCIL:
        if (tex_type == ATRA_FBO_TEXTYPE_TEXTURE) {
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                    GL_STENCIL_ATTACHMENT,
                    GL_TEXTURE_2D,
                    *tex, 0);
        } else if (tex_type == ATRA_FBO_TEXTYPE_RENDERTEX) {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                    GL_STENCIL_ATTACHMENT,
                    GL_RENDERBUFFER,
                    *tex);
        }
        break;
    default:
        atra_log_print(LOG_WARN, "framebuffer: Invalid attachment type given");
        break;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/* Detach and delete the texture at the given attachment slot of the FBO */
ATRA_API
void
atra_framebuffer_remove(FrameBuffer *fb, uint attach_slot)
{
    const uint tex_type = fb->types[attach_slot];
    uint tex = fb->textures[attach_slot];

    glBindFramebuffer(GL_FRAMEBUFFER, fb->id);

    if (tex_type == ATRA_FBO_TEXTYPE_TEXTURE) {
        glDeleteTextures(1, &tex);
    } else if (tex_type == ATRA_FBO_TEXTYPE_RENDERTEX) {
        glDeleteRenderbuffers(1, &tex);
    }
    fb->types[attach_slot] = ATRA_FBO_TEXTYPE_NONE;
    fb->textures[attach_slot] = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/* Detach and delete all textures in the FBO */
ATRA_API
void
atra_framebuffer_clear(FrameBuffer *fb)
{
    uint i;

    glBindFramebuffer(GL_FRAMEBUFFER, fb->id);

    for (i = 0; i < ATRA_FBO_ATTACH_MAX; i++) {
        if (fb->types[i] == ATRA_FBO_TEXTYPE_TEXTURE) {
            glDeleteTextures(1, &fb->textures[i]);
        } else if (fb->types[i] == ATRA_FBO_TEXTYPE_RENDERTEX) {
            glDeleteRenderbuffers(1, &fb->textures[i]);
        }
        fb->types[i] = ATRA_FBO_TEXTYPE_NONE;
        fb->textures[i] = 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/* Resizes an FBO by binding new textures (will erase existing FBO data) */
ATRA_API
void
atra_framebuffer_resize(FrameBuffer *fb, uint width, uint height)
{
    FrameBuffer fb_new;
    uint i;

    fb_new = atra_framebuffer_create(width, height);
    
    for (i = 0; i < ATRA_FBO_ATTACH_MAX; i++) {
        if (fb->types[i] != ATRA_FBO_TEXTYPE_NONE) {
            atra_framebuffer_add(&fb_new, fb->types[i], i,
                    fb->internal_format[i], fb->format[i], fb->data_type[i], fb->has_border[i]);
        }
    }
    atra_framebuffer_delete(fb);
    *fb = fb_new;
}

/* Bind the FBO for drawing to contained textures/RBOs */
ATRA_API
void
atra_framebuffer_bind(FrameBuffer *fb)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fb->id);
}

/* Unbind the currently bound FBO */
ATRA_API
void
atra_framebuffer_unbind(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/* Bind a FBO texture at the given slot to the given texture unit */
ATRA_API
void
atra_framebuffer_bind_texunit(FrameBuffer *fb, uint attach_slot, uint tex_unit)
{
    atra_gl_tex_setactiveunit(tex_unit);
    atra_gl_tex_enable(fb->textures[attach_slot]);
}

/* Check and validate the given FBO, enabling it's use */
ATRA_API
int
atra_framebuffer_finalize(FrameBuffer *fb)
{
    uint count, status, i;
    uint attachments[ATRA_FBO_ATTACH_COLORMAX];

    count = 0;
    fb->valid = FALSE;

    glBindFramebuffer(GL_FRAMEBUFFER, fb->id);

    for (i = 0; i < ATRA_FBO_ATTACH_COLORMAX; i++) {
        if (fb->types[i] != ATRA_FBO_TEXTYPE_NONE) {
            attachments[i] = GL_COLOR_ATTACHMENT0 + i;
            count++;
        }
    }
    glDrawBuffers(count, attachments);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        switch (status) {
        case GL_FRAMEBUFFER_UNSUPPORTED:
            atra_log_print(LOG_WARN, "gl: fbo: framebuffer unsupported");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            atra_log_print(LOG_WARN, "gl: fbo: framebuffer has incomplete attachment");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            atra_log_print(LOG_WARN, "gl: fbo: framebuffer has a missing attachment");
            break;
        default:
            break;
        }
    } else {
        fb->valid = TRUE;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return (status == GL_FRAMEBUFFER_COMPLETE);
}
