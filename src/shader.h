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

#define ATRA_MAX_SHADER_ATTRIBS 32

#define ATRA_ATTRIB_NAME_POSITION   "vert_pos"
#define ATRA_ATTRIB_NAME_TEXCOORD   "vert_uv"
#define ATRA_ATTRIB_NAME_NORMAL     "vert_norm"
#define ATRA_ATTRIB_NAME_COLOR      "vert_color"
#define ATRA_ATTRIB_NAME_TANGENT    "vert_tangent"
#define ATRA_ATTRIB_NAME_TEXCOORD2  "vert_uv2"

#define ATRA_UNIFORM_NAME_MVP       "mvp"
#define ATRA_UNIFORM_NAME_VIEWMAT   "viewmat"
#define ATRA_UNIFORM_NAME_PROJMAT   "projmat"
#define ATRA_UNIFORM_NAME_MODELMAT  "modelmat"
#define ATRA_UNIFORM_NAME_NORMALMAT "normalmat"
#define ATRA_UNIFORM_NAME_COLOR     "color"
#define ATRA_UNIFORM_NAME_TEXTURE0  "albedo_map"
#define ATRA_UNIFORM_NAME_TEXTURE1  "specular_map"
#define ATRA_UNIFORM_NAME_TEXTURE2  "normal_map"

enum shader_locs {
    ATRA_SHADER_LOC_V_POSITION,
    ATRA_SHADER_LOC_V_TEXCOORD,
    ATRA_SHADER_LOC_V_NORMAL,
    ATRA_SHADER_LOC_V_COLOR,
    ATRA_SHADER_LOC_V_TANGENT,
    ATRA_SHADER_LOC_V_TEXCOORD2,
    ATRA_SHADER_LOC_M_MVP,
    ATRA_SHADER_LOC_M_VIEW,
    ATRA_SHADER_LOC_M_PROJ,
    ATRA_SHADER_LOC_M_MODEL,
    ATRA_SHADER_LOC_M_NORMAL,
    ATRA_SHADER_LOC_V_VIEW,
    ATRA_SHADER_LOC_COLOR_DIFFUSE,
    ATRA_SHADER_LOC_COLOR_SPECULAR,
    ATRA_SHADER_LOC_COLOR_AMBIENT,
    ATRA_SHADER_LOC_VEC_NORMAL,
    ATRA_SHADER_LOC_TEX_ALBEDO,
    ATRA_SHADER_LOC_TEX_SPECULAR,
    ATRA_SHADER_LOC_TEX_NORMAL,
    ATRA_SHADER_LOC_TEX_ROUGHNESS,
    ATRA_SHADER_LOC_TEX_OCCLUSION,
    ATRA_SHADER_LOC_TEX_EMISSION,
    ATRA_SHADER_LOC_TEX_HEIGHT,
    ATRA_SHADER_LOC_TEX_CUBEMAP,
    ATRA_SHADER_LOC_TEX_IRRADIANCE,
    ATRA_SHADER_LOC_TEX_PREFILTER,
    ATRA_SHADER_LOC_TEX_BRDF,
};

enum uniform_type {
    ATRA_SHADER_UNIFORM_FLOAT,
    ATRA_SHADER_UNIFORM_VEC2,
    ATRA_SHADER_UNIFORM_VEC3,
    ATRA_SHADER_UNIFORM_VEC4,
    ATRA_SHADER_UNIFORM_INT,
    ATRA_SHADER_UNIFORM_IVEC2,
    ATRA_SHADER_UNIFORM_IVEC3,
    ATRA_SHADER_UNIFORM_IVEC4,
    ATRA_SHADER_UNIFORM_TEXTURE,
    ATRA_SHADER_UNIFORM_MAT4
};

enum attribute_type {
    ATRA_SHADER_ATTRIB_FLOAT,
    ATRA_SHADER_ATTRIB_VEC2,
    ATRA_SHADER_ATTRIB_VEC3,
    ATRA_SHADER_ATTRIB_VEC4
};

typedef struct Color {
    u8 r, g, b, a;
} Color;

typedef struct Shader {
    uint prog_id;
    int *locs;
} Shader;

#ifdef __cplusplus
extern "C" {
#endif

/* Compiles a shader program from given code, with specified shader type */
ATRA_API uint atra_shader_compile(const char *code, int type);

/* Compiles and loads a shader program from file, with specified `type` */
ATRA_API Shader atra_shader_loadfile(const char *vs_path, const char *fs_path);

/* Compiles and loads a shader program from a text buffer */
ATRA_API Shader atra_shader_loadbuffer(const char *vs_code, const char *fs_code);

/* Sets the given shader as the currently active program */
ATRA_API void atra_shader_setactive(Shader *shader);

/* Returns a pointer to the currently active shader */
ATRA_API Shader * atra_shader_getactive(void);

/* Returns the location of a shader uniform by name */
ATRA_API int atra_shader_getuniformloc(Shader *shader, const char *name);

/* Returns the location of a shader attribute by name */
ATRA_API int atra_shader_getattribloc(Shader *shader, const char *name);

/* Sets the value of a shader uniform */
ATRA_API void atra_shader_setuniform(int loc, const void *data, int type, int count);

/* Unloads the shader from the GPU, freeing all memory */
ATRA_API void atra_shader_destroy(Shader *shader);

#ifdef __cplusplus
}
#endif

#endif /* ATRA_SHADER_H */
