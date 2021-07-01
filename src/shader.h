/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  shader.h - Tools for GLSL shader loading, compilation and management.
 *
 *  */

#ifndef ATRA_SHADER_H
#define ATRA_SHADER_H

#include "common.h"

typedef struct Shader {
    uint prog_id;
    int *locs;
} Shader;

/* Compiles a shader program from file, with specified `type` */
ATRA_API GLuint atra_shader_loadfile(const char *path, GLenum type);

/* Compiles vertex and fragment shader from file, returns program ID */
ATRA_API uint atra_shader_loadfilevf(const char *vs_path, const char *fs_path);

#endif /* ATRA_SHADER_H */
