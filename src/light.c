/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  light.c - Simple module for creating and using lights of various types
 *
 *  */

#include "atra.h"
#include "light.h"
#include "framebuffer.h"

/* Returns a new directional light with some basic configurations.
 * Light rotation/direction must be set separately. */
ATRA_API
DirLight
atra_dirlight_create(uint frustum_type,
        uint width, uint height, uint fbo_width, uint fbo_height,
        uint has_shadows, uint has_falloff, Color color, vec3 pos)
{
    DirLight light = {0};

    light.width = width;
    light.height = height;
    light.enabled = 1;
    light.rotation_type = ATRA_LIGHT_ROT_RADIAN;
    light.fov = DEG_TO_RAD(90);
    light.frustum_type = frustum_type;
    light.has_shadows = has_shadows;
    light.has_falloff = has_falloff;
    light.color = color;

    glm_vec3_copy(pos, light.pos);

    if (light.has_shadows) {
        light.fbo = atra_framebuffer_create(fbo_width, fbo_height);
        atra_framebuffer_add(&light.fbo,
                ATRA_FBO_TEXTYPE_TEXTURE, ATRA_FBO_ATTACH_DEPTH,
                ATRA_DEPTH_COMPONENT, ATRA_DEPTH_COMPONENT,
                ATRA_FLOAT, TRUE);
        atra_framebuffer_finalize(&light.fbo);
    }
    atra_dirlight_updateview(&light);
    atra_dirlight_updateproj(&light);

    return light;
}

/* Sets the position of the directional light */
ATRA_API
void
atra_dirlight_setpos(DirLight *light, vec3 pos)
{
    glm_vec3_copy(pos, light->pos);
}

/* Sets the rotation of the light (with specific behavior) and updates relevant vectors */
ATRA_API
void
atra_dirlight_setrot(DirLight *light, uint type, vec3 rot)
{
    light->rotation_type = type;
    glm_vec3_copy(rot, light->rot);

    atra_dirlight_updateview(light);
    atra_dirlight_updateproj(light);
}

/* Modifies the near and far planes of the light's frustum */
ATRA_API
void
atra_dirlight_setplanes(DirLight *light, float near, float far)
{
    if (near < 0.0 || far < 0.0 || near >= far) {
        atra_log_print(LOG_WARN, "light: Invalid near/far planes: Reverting to defaults");
        near = ATRA_LIGHT_NEAR_PLANE;
        far = ATRA_LIGHT_FAR_PLANE;
    }

    if (light->frustum_type == ATRA_LIGHT_PROJ) {
        glm_perspective(light->fov,
                (float)light->width / (float)light->height,
                near, far,
                light->proj);
    } else if (light->frustum_type == ATRA_LIGHT_ORTHO) {
        glm_ortho(-light->width / 2.0f,
                light->width / 2.0f,
                -light->height / 2.0f,
                light->height / 2.0f,
                near, far,
                light->proj);
    } else {
        atra_log_print(LOG_WARN, "light: Invalid frustum projection mode");
    }

}

/* Update the light's view orientation vectors */
ATRA_API
void
atra_dirlight_updatevectors(DirLight *light)
{
    vec3 rot;

    if (light->rotation_type == ATRA_LIGHT_ROT_RADIAN) {
        glm_vec3_copy(light->rot, rot);
    } else if (light->rotation_type == ATRA_LIGHT_ROT_DIRECTION) {
        vec2 dir_xz, dir_yz;

        dir_xz[0] = light->rot[0];
        dir_xz[1] = light->rot[2];

        dir_yz[0] = light->rot[1];
        dir_yz[1] = sqrtf(1 - light->rot[1] * light->rot[1]);

        glm_vec2_normalize(dir_xz);
        glm_vec2_normalize(dir_yz);

        rot[0] = atan2f(dir_yz[0], dir_yz[1]);
        rot[1] = atan2f(dir_xz[0], dir_xz[1]);
        rot[2] = 0.0f;
    } else if (light->rotation_type == ATRA_LIGHT_ROT_TARGET) {
        vec3 norm;
        
        glm_vec3_sub(light->rot, light->pos, norm);
        glm_vec3_normalize(norm);

        vec2 dir_xz = {norm[0], norm[2]};
        vec2 dir_yz = {norm[1], norm[2]};

        dir_xz[0] = norm[0];
        dir_xz[1] = norm[2];

        dir_yz[0] = norm[1];
        dir_yz[1] = sqrtf(1 - norm[1] * norm[1]);

        glm_vec2_normalize(dir_xz);
        glm_vec2_normalize(dir_yz);

        rot[0] = atan2f(dir_yz[0], dir_yz[1]);
        rot[1] = atan2f(dir_xz[0], dir_xz[1]);
        rot[2] = 0.0f;
    }
    light->dir[0] = cosf(rot[0]) * sinf(rot[1]);
    light->dir[1] = sinf(rot[0]);
    light->dir[2] = cosf(rot[0]) * cosf(rot[1]);

    light->right[0] = sinf(rot[1] - (PI / 2.0f));
    light->right[1] = 0.0f;
    light->right[2] = cosf(rot[1] - (PI / 2.0f));

    glm_vec3_cross(light->right, light->dir, light->up);
}

/* Update the view matrix of the light */
ATRA_API
void
atra_dirlight_updateview(DirLight *light)
{
    vec3 target;

    glm_vec3_add(light->pos, light->dir, target);
    glm_lookat(light->pos, target, light->up, light->view);
}

/* Update the projection matrix of the light */
ATRA_API
void
atra_dirlight_updateproj(DirLight *light)
{
    if (light->frustum_type == ATRA_LIGHT_PROJ) {
        glm_perspective(light->fov,
                (float)light->width / (float)light->height,
                ATRA_LIGHT_NEAR_PLANE,
                ATRA_LIGHT_FAR_PLANE,
                light->proj);
    } else if (light->frustum_type == ATRA_LIGHT_ORTHO) {
        glm_ortho(-light->width / 2.0f,
                light->width / 2.0f,
                -light->height / 2.0f,
                light->height / 2.0f,
                ATRA_LIGHT_NEAR_PLANE,
                ATRA_LIGHT_FAR_PLANE,
                light->proj);
    } else {
        atra_log_print(LOG_WARN, "light: Invalid frustum projection mode");
    }
}

/* Binds the light's depth framebuffer for a depth pass from its perspective */
ATRA_API
void
atra_dirlight_bindfbo(DirLight *light)
{
    atra_framebuffer_bind(&light->fbo);
}

/* Unbinds the light's depth framebuffer */
ATRA_API
void
atra_dirlight_unbindfbo()
{
    atra_framebuffer_unbind();
}

/* Binds the depth texture to the given texture unit for use in shadow mapping */
ATRA_API
void
atra_dirlight_binddepthtex(DirLight *light, uint tex_unit)
{
    atra_gl_tex_setactiveunit(tex_unit);
    atra_gl_tex_enable(light->fbo.textures[ATRA_FBO_ATTACH_DEPTH]);
}

