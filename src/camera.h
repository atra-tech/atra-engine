/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  camera.h - Structure and functions for creating and manipulating key
 *             matrices used to calculate the view of the world for drawing to
 *             the screen.
 *
 *  */

#ifndef ATRA_CAMERA_H
#define ATRA_CAMERA_H

#define ATRA_CAMERA_DEFAULT_FOV     PI / 2.0f;
#define ATRA_CAMERA_DEFAULT_ZOOM    2.0f
#define ATRA_CAMERA_NEAR_PLANE      0.1f
#define ATRA_CAMERA_FAR_PLANE       10000.0f

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

enum camera_type {
    ATRA_CAMERA_PROJECTION,
    ATRA_CAMERA_ORTHOGRAPHIC,
};

typedef struct Camera {
    int type;       /* Projection or orthographic view */
    int width;      /* Width of camera view */
    int height;     /* Height of camera view */
    
    float fov;      /* Y-axis field of view angle (radians) */
    float zoom;     /* Camera zoom modifier (useful for orthographic view) */
    
    vec3 pos;       /* Camera world-space position */
    vec3 rot;       /* Camera rotation (non-quaternion) */
    vec3 target;    /* Position the camera looks at */
    vec3 dir;       /* Directional vector, points towards the target position */
    vec3 up;        /* Up vector (perpendicular to directional vector) */
    vec3 right;     /* Right vector (perpendicular to up and dir vectors) */

    mat4 view;      /* View matrix */
    mat4 proj;      /* Projection matrix */
} Camera;

/* Returns a new camera of the chosen type
 * (will set this camera as the 'main' camera if no other is set) */
ATRA_API Camera atra_camera_create(int type, int width, int height);

/* Sets a camera's type and computes it's projection matrix */
ATRA_API void atra_camera_settype(Camera *cam, int type);

/* Updates a camera's FOV to a new value */
ATRA_API void atra_camera_setfov(Camera *cam, float fov);

/* Updates the camera's zoom to a new value */
ATRA_API void atra_camera_setzoom(Camera *cam, float zoom);

/* Updates the camera's view width and height to new values */
ATRA_API void atra_camera_setsize(Camera *cam, int width, int height);

/* Updates a camera's view matrix and vectors */
ATRA_API void atra_camera_update(Camera *cam);

/* Updates a camera's vectors */
ATRA_API void atra_camera_updatevectors(Camera *cam);

/* Updates a camera's view matrix */
ATRA_API void atra_camera_updateviewmat(Camera *cam);

/* Updates a camera's projection matrix */
ATRA_API void atra_camera_updateprojmat(Camera *cam);

/* Sets the given camera as the 'main' camera */
ATRA_API void atra_camera_setmain(Camera *cam);

/* Returns a pointer to the 'main' camera */
ATRA_API Camera * atra_camera_getmain(void);

/* Copies the main camera's view matrix into the given matrix */
ATRA_API void atra_camera_getmainview(vec4 *mat);

/* Copies the main camera's view matrix into the given matrix */
ATRA_API void atra_camera_getmainproj(vec4 *mat);

#ifdef __cplusplus
}
#endif

#endif /* ATRA_CAMERA_H */
