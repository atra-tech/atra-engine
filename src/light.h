/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  light.h - Simple module for creating and using lights of various types
 *
 *  */

#ifndef ATRA_LIGHT_H
#define ATRA_LIGHT_H

#include "common.h"
#include "framebuffer.h"

#define ATRA_LIGHT_FAR_PLANE    1000.0f
#define ATRA_LIGHT_NEAR_PLANE   1.0f

#ifdef __cplusplus
extern "C" {
#endif

enum light_frustum_type {
    ATRA_LIGHT_PROJ,
    ATRA_LIGHT_ORTHO
};

enum light_rotation_type {
    ATRA_LIGHT_ROT_RADIAN,      /* Rotation reads as a basic rotation vector in radians */
    ATRA_LIGHT_ROT_DIRECTION,   /* Rotation data is a directional normal */
    ATRA_LIGHT_ROT_TARGET       /* Rotation data is a target position to rotate towards */
};

enum light_shadow_type {
    ATRA_LIGHT_SHADOW_SINGLE,   /* Single depth-buffer shadow mapping */
    ATRA_LIGHT_SHADOW_CASCADING /* TODO: Multi depth-buffer cascading shadow mapping */
};

typedef struct DirLight {
    int width;             /* Width of the light projection frustum */
    int height;            /* Height of the light projection frustum */
    uint enabled;           /* Set when light should be calculated */
    uint frustum_type;      /* Light frustum type, projection or orthographic */
    uint has_shadows;       /* Set when light is meant to cast shadows */
    uint has_falloff;       /* Set when light intensity should decrease over distance */
    uint rotation_type;     /* Method by which `rot` field should be processed */

    float fov;              /* Horizontal field of view in radians */

    Color color;            /* Light color */
    float linear;           /* Linear falloff component */
    float quadratic;        /* Quadratic falloff component */

    vec3 pos;               /* Light position in world-space */
    vec3 rot;               /* Light rotation (behavior dependent on flag) */
    vec3 dir;               /* Light dir vector */
    vec3 up;                /* Light up vector */
    vec3 right;             /* Light right vector */
    mat4 view;              /* Light view matrix */
    mat4 proj;              /* Light projection matrix */

    FrameBuffer fbo;        /* Light framebuffer for recording depth */
} DirLight;

typedef struct PointLight {
    uint enabled;

    Color color;
    float linear;
    float quadratic;

    vec3 pos;
    
} PointLight;

/* Returns a new directional light with some basic configurations.
 * Light rotation/direction must be set separately. */
ATRA_API DirLight atra_dirlight_create(uint frustum_type,
        uint width, uint height, uint fbo_width, uint fbo_height,
        uint has_shadows, uint has_falloff, Color color, vec3 pos);

/* Sets the position of the directional light */
ATRA_API void atra_dirlight_setpos(DirLight *light, vec3 pos);

/* Sets the rotation of the light (with specific behavior) and updates relevant vectors */
ATRA_API void atra_dirlight_setrot(DirLight *light, uint type, vec3 rot);

/* Modifies the near and far planes of the light's frustum */
ATRA_API void atra_dirlight_setplanes(DirLight *light, float near, float far);

/* Update the light's view orientation vectors */
ATRA_API void atra_dirlight_updatevectors(DirLight *light);

/* Update the view matrix of the light (view vectors must already be updated) */
ATRA_API void atra_dirlight_updateview(DirLight *light);

/* Update the projection matrix of the light */
ATRA_API void atra_dirlight_updateproj(DirLight *light);

/* Binds the light's depth framebuffer for a depth pass from its perspective */
ATRA_API void atra_dirlight_bindfbo(DirLight *light);

/* Unbinds the currently bound depth framebuffer */
ATRA_API void atra_dirlight_unbindfbo();

/* Binds the depth texture to the given texture unit for use in shadow mapping */
ATRA_API void atra_dirlight_binddepthtex(DirLight *light, uint tex_unit);

#ifdef __cplusplus
}
#endif

#endif /* ATRA_LIGHT_H */
