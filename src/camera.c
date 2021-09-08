/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  camera.c - Structure and functions for creating and manipulating key
 *             matrices used to calculate the view of the world for drawing to
 *             the screen.
 *  */

#include "atra.h"
#include <math.h>

static Camera *main_cam;

/* Returns a new camera of the chosen type */
ATRA_API
Camera
atra_camera_create(int type, int width, int height)
{
    Camera cam;

    cam.type = type;
    cam.width = width;
    cam.height = height;

    cam.fov = ATRA_CAMERA_DEFAULT_FOV;
    cam.zoom = ATRA_CAMERA_DEFAULT_ZOOM;

    cam.pos[0] = 0.0f;
    cam.pos[1] = 0.0f;
    cam.pos[2] = 0.0f;

    cam.rot[0] = 0.0f;
    cam.rot[1] = 0.0f;
    cam.rot[2] = 0.0f;

    atra_camera_updateviewmat(&cam);

    return cam;
}

/* Sets a camera's type and computes it's projection matrix */
ATRA_API
void
atra_camera_settype(Camera *cam, int type)
{
    cam->type = type;
    atra_camera_updateprojmat(cam);
}

/* Updates a camera's FOV to a new value */
ATRA_API
void
atra_camera_setfov(Camera *cam, float fov)
{
    cam->fov = fov;
    atra_camera_updateprojmat(cam);
}

/* Updates the camera's zoom to a new value */
ATRA_API
void
atra_camera_setzoom(Camera *cam, float zoom)
{
    cam->zoom = zoom;
    atra_camera_updateprojmat(cam);
}

/* Updates the camera's view width and height to new values */
ATRA_API
void
atra_camera_setsize(Camera *cam, int width, int height)
{
    cam->width = width;
    cam->height = height;
    atra_camera_updateprojmat(cam);
}

/* Updates a camera's view matrix and vectors */
ATRA_API
void
atra_camera_update(Camera *cam)
{
    atra_camera_updatevectors(cam);
    atra_camera_updateviewmat(cam);
}

/* Updates a camera's vectors */
ATRA_API
void
atra_camera_updatevectors(Camera *cam)
{
    cam->dir[0] = SDL_cosf(cam->rot[0]) * SDL_sinf(cam->rot[1]);
    cam->dir[1] = SDL_sinf(cam->rot[0]);
    cam->dir[2] = SDL_cosf(cam->rot[0]) * SDL_cosf(cam->rot[1]);

    cam->right[0] = SDL_sinf(cam->rot[1] - (PI / 2.0f));
    cam->right[1] = 0.0f;
    cam->right[2] = SDL_cosf(cam->rot[1] - (PI / 2.0f));

    glm_vec3_cross(cam->right, cam->dir, cam->up);
}

/* Updates a camera's view matrix */
ATRA_API
void
atra_camera_updateviewmat(Camera *cam)
{
    vec3 centre;

    glm_vec3_add(cam->pos, cam->dir, centre);
    glm_lookat(cam->pos, centre, cam->up, cam->view);
}

/* Updates a camera's projection matrix */
ATRA_API
void
atra_camera_updateprojmat(Camera *cam)
{
    if (cam->type == ATRA_CAMERA_PROJECTION) {
        glm_perspective(cam->fov,
                (float)cam->width / (float)cam->height,
                ATRA_CAMERA_NEAR_PLANE,
                ATRA_CAMERA_FAR_PLANE,
                cam->proj);
    } else if (cam->type == ATRA_CAMERA_ORTHOGRAPHIC) {
        glm_ortho(-cam->width / cam->zoom,
                cam->width / cam->zoom,
                -cam->height / cam->zoom,
                cam->height / cam->zoom,
                ATRA_CAMERA_NEAR_PLANE,
                ATRA_CAMERA_FAR_PLANE,
                cam->proj);
    } else {
        atra_log_print(LOG_WARN, "camera: Invalid camera view type");
    }
}

/* Sets the given camera as the 'main' camera */
ATRA_API
void
atra_camera_setmain(Camera *cam)
{
    main_cam = cam;
}

/* Returns a pointer to the 'main' camera */
ATRA_API
Camera *
atra_camera_getmain(void)
{
    return main_cam;
}

/* Copies the main camera's view matrix into the given matrix */
ATRA_API
void
atra_camera_getmainview(vec4 *mat)
{
    memcpy(mat, main_cam->view, sizeof(mat4));
}

/* Copies the main camera's view matrix into the given matrix */
ATRA_API
void
atra_camera_getmainproj(vec4 *mat)
{
    memcpy(mat, main_cam->proj, sizeof(mat4));
}
