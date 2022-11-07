/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  shader.c - Tools for GLSL shader loading, compilation and management.
 *
 *  */

#include "atra.h"

static Shader *active_shader;

/* Compiles a shader program from given code, with specified shader type */
ATRA_API
uint
atra_shader_compile(const char *code, int type)
{
    int success = 0;
    uint shader;

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE) {
        switch (type) {
        case GL_VERTEX_SHADER:
            atra_log_print(LOG_ERROR, "shader: vertex shader [%d] failed to compile", shader);
            break;
        case GL_FRAGMENT_SHADER:
            atra_log_print(LOG_ERROR, "shader: fragment shader [%d] failed to compile", shader);
            break;
        default:
            break;
        }

        int max_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_len);

        if (max_len > 0) {
            int len = 0;
            char *log = calloc(max_len, sizeof(char));
            glGetShaderInfoLog(shader, max_len, &len, log);
            atra_log_print(LOG_ERROR, "shader: shader [%d] compilation error: " \
                    "%s", shader, log);
            free(log);
        }
    }

    return shader;
}

/* Compiles and loads a shader program from file, with specified `type` */
ATRA_API
Shader
atra_shader_loadfile(const char *vs_path, const char *fs_path)
{
    char * const vs_code = (char *) atra_util_loadfile(vs_path, NULL);
    char * const fs_code = (char *) atra_util_loadfile(fs_path, NULL);

    Shader shader = atra_shader_loadbuffer(vs_code, fs_code);
    free(vs_code);
    free(fs_code);

    return shader;
}

/* Compiles and loads a shader program from a text buffer */
ATRA_API
Shader
atra_shader_loadbuffer(const char *vs_code, const char *fs_code)
{
    Shader shader = {0};
    uint v_shader, f_shader;
    int i, success, max_len, len;

    shader.locs = calloc(ATRA_MAX_SHADER_ATTRIBS, sizeof(int));

    for (i = 0; i < ATRA_MAX_SHADER_ATTRIBS; i++) {
        shader.locs[i] = -1;
    }
    v_shader = atra_shader_compile(vs_code, GL_VERTEX_SHADER);
    f_shader = atra_shader_compile(fs_code, GL_FRAGMENT_SHADER);

    shader.prog_id = glCreateProgram();

    glAttachShader(shader.prog_id, v_shader);
    glAttachShader(shader.prog_id, f_shader);

    glBindAttribLocation(shader.prog_id, 0, ATRA_ATTRIB_NAME_POSITION);
    glBindAttribLocation(shader.prog_id, 1, ATRA_ATTRIB_NAME_TEXCOORD);
    glBindAttribLocation(shader.prog_id, 2, ATRA_ATTRIB_NAME_NORMAL);
    glBindAttribLocation(shader.prog_id, 3, ATRA_ATTRIB_NAME_COLOR);
    glBindAttribLocation(shader.prog_id, 4, ATRA_ATTRIB_NAME_TANGENT);
    glBindAttribLocation(shader.prog_id, 5, ATRA_ATTRIB_NAME_TEXCOORD2);
    glBindAttribLocation(shader.prog_id, 6, ATRA_ATTRIB_NAME_BONEIDS);
    glBindAttribLocation(shader.prog_id, 7, ATRA_ATTRIB_NAME_BONEWEIGHTS);

    glBindAttribLocation(shader.prog_id, 11, ATRA_ATTRIB_NAME_MODELS);

    glLinkProgram(shader.prog_id);
    glGetProgramiv(shader.prog_id, GL_LINK_STATUS, &success);

    if (success == GL_FALSE) {
        max_len = 0;
        atra_log_print(LOG_ERROR, "shader: shader [%d] failed to link",
                shader.prog_id);
        glGetProgramiv(shader.prog_id, GL_INFO_LOG_LENGTH, &max_len);

        if (max_len > 0) {
            len = 0;
            char *log = calloc(max_len, sizeof(char));
            glGetProgramInfoLog(shader.prog_id, max_len, &len, log);
            atra_log_print(LOG_ERROR, "shader: shader [%d] linker error: " \
                    "%s", shader.prog_id, log);
            free(log);
        }
        glDeleteProgram(shader.prog_id);

        shader.prog_id = 0;
    }
    glDetachShader(shader.prog_id, v_shader);
    glDetachShader(shader.prog_id, f_shader);
    glDeleteShader(v_shader);
    glDeleteShader(f_shader);

    /* Get attribute and uniform locations */
    if (shader.prog_id > 0) {
        /* Attributes */
        shader.locs[ATRA_SHADER_LOC_V_POSITION]     = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_POSITION);
        shader.locs[ATRA_SHADER_LOC_V_TEXCOORD]     = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_TEXCOORD);
        shader.locs[ATRA_SHADER_LOC_V_NORMAL]       = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_NORMAL);
        shader.locs[ATRA_SHADER_LOC_V_COLOR]        = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_COLOR);
        shader.locs[ATRA_SHADER_LOC_V_TANGENT]      = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_TANGENT);
        shader.locs[ATRA_SHADER_LOC_V_TEXCOORD2]    = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_TEXCOORD2);
        shader.locs[ATRA_SHADER_LOC_V_BONEIDS]      = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_BONEIDS);
        shader.locs[ATRA_SHADER_LOC_V_BONEWEIGHTS]  = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_BONEWEIGHTS);

        /* Vertex shader uniforms */
        shader.locs[ATRA_SHADER_LOC_M_MVP]          = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_MVP);
        shader.locs[ATRA_SHADER_LOC_M_VIEW]         = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_VIEWMAT);
        shader.locs[ATRA_SHADER_LOC_M_PROJ]         = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_PROJMAT);
        shader.locs[ATRA_SHADER_LOC_M_MODEL]        = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_MODELMAT);
        shader.locs[ATRA_SHADER_LOC_M_NORMAL]       = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_NORMALMAT);
        shader.locs[ATRA_SHADER_LOC_M_BONES]        = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_BONEMATS);

        /* Fragment shader uniforms */
        shader.locs[ATRA_SHADER_LOC_COLOR_DIFFUSE]  = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_COLOR);
        shader.locs[ATRA_SHADER_LOC_TEX_ALBEDO]     = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_TEXTURE0);
        shader.locs[ATRA_SHADER_LOC_TEX_SPECULAR]   = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_TEXTURE1);
        shader.locs[ATRA_SHADER_LOC_TEX_NORMAL]     = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_TEXTURE2);
    }

    return shader;
}

/* Compiles and loads a shader program from file, with specified `type` */
ATRA_API
Shader
atra_shader_loadfile_locset(const char *vs_path, const char *fs_path,
        const char **attrib_names, const uint *attrib_ids, uint attrib_count)
{
    char * const vs_code = (char *) atra_util_loadfile(vs_path, NULL);
    char * const fs_code = (char *) atra_util_loadfile(fs_path, NULL);

    Shader shader = atra_shader_loadbuffer_locset(vs_code, fs_code, attrib_names, attrib_ids, attrib_count);
    free(vs_code);
    free(fs_code);

    return shader;
}

/* Compiles and loads a shader program from a text buffer */
ATRA_API
Shader
atra_shader_loadbuffer_locset(const char *vs_code, const char *fs_code,
        const char **attrib_names, const uint *attrib_ids, uint attrib_count)
{
    Shader shader = {0};
    uint v_shader, f_shader;
    int i, success, max_len, len;

    shader.locs = calloc(ATRA_MAX_SHADER_ATTRIBS, sizeof(int));

    for (i = 0; i < ATRA_MAX_SHADER_ATTRIBS; i++) {
        shader.locs[i] = -1;
    }
    v_shader = atra_shader_compile(vs_code, GL_VERTEX_SHADER);
    f_shader = atra_shader_compile(fs_code, GL_FRAGMENT_SHADER);

    shader.prog_id = glCreateProgram();

    glAttachShader(shader.prog_id, v_shader);
    glAttachShader(shader.prog_id, f_shader);

    for (i = 0; (uint) i < attrib_count; i++) {
        glBindAttribLocation(shader.prog_id, attrib_ids[i], attrib_names[i]);
    }
    glLinkProgram(shader.prog_id);
    glGetProgramiv(shader.prog_id, GL_LINK_STATUS, &success);

    if (success == GL_FALSE) {
        max_len = 0;
        atra_log_print(LOG_ERROR, "shader: shader [%d] failed to link",
                shader.prog_id);
        glGetProgramiv(shader.prog_id, GL_INFO_LOG_LENGTH, &max_len);

        if (max_len > 0) {
            len = 0;
            char *log = calloc(max_len, sizeof(char));
            glGetProgramInfoLog(shader.prog_id, max_len, &len, log);
            atra_log_print(LOG_ERROR, "shader: shader [%d] linker error: " \
                    "%s", shader.prog_id, log);
            free(log);
        }
        glDeleteProgram(shader.prog_id);

        shader.prog_id = 0;
    }
    glDetachShader(shader.prog_id, v_shader);
    glDetachShader(shader.prog_id, f_shader);
    glDeleteShader(v_shader);
    glDeleteShader(f_shader);

    /* Get attribute and uniform locations */
    if (shader.prog_id > 0) {
        /* Attributes */
        shader.locs[ATRA_SHADER_LOC_V_POSITION]     = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_POSITION);
        shader.locs[ATRA_SHADER_LOC_V_TEXCOORD]     = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_TEXCOORD);
        shader.locs[ATRA_SHADER_LOC_V_NORMAL]       = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_NORMAL);
        shader.locs[ATRA_SHADER_LOC_V_COLOR]        = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_COLOR);
        shader.locs[ATRA_SHADER_LOC_V_TANGENT]      = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_TANGENT);
        shader.locs[ATRA_SHADER_LOC_V_TEXCOORD2]    = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_TEXCOORD2);

        shader.locs[ATRA_SHADER_LOC_V_BONEIDS]      = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_BONEIDS);
        shader.locs[ATRA_SHADER_LOC_V_BONEWEIGHTS]  = glGetAttribLocation(shader.prog_id, ATRA_ATTRIB_NAME_BONEWEIGHTS);

        /* Vertex shader uniforms */
        shader.locs[ATRA_SHADER_LOC_M_MVP]          = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_MVP);
        shader.locs[ATRA_SHADER_LOC_M_VIEW]         = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_VIEWMAT);
        shader.locs[ATRA_SHADER_LOC_M_PROJ]         = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_PROJMAT);
        shader.locs[ATRA_SHADER_LOC_M_MODEL]        = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_MODELMAT);
        shader.locs[ATRA_SHADER_LOC_M_NORMAL]       = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_NORMALMAT);
        shader.locs[ATRA_SHADER_LOC_M_BONES]        = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_BONEMATS);

        /* Fragment shader uniforms */
        shader.locs[ATRA_SHADER_LOC_COLOR_DIFFUSE]  = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_COLOR);
        shader.locs[ATRA_SHADER_LOC_TEX_ALBEDO]     = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_TEXTURE0);
        shader.locs[ATRA_SHADER_LOC_TEX_SPECULAR]   = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_TEXTURE1);
        shader.locs[ATRA_SHADER_LOC_TEX_NORMAL]     = glGetUniformLocation(shader.prog_id, ATRA_UNIFORM_NAME_TEXTURE2);
    }

    return shader;

}

/* Sets the given shader as the currently active program */
ATRA_API
void
atra_shader_setactive(Shader *shader)
{
    active_shader = shader;
    glUseProgram(shader->prog_id);
}

/* Returns a pointer to the currently active shader */
ATRA_API
Shader *
atra_shader_getactive(void)
{
    return active_shader;
}

/* Returns the location of a shader uniform by name */
ATRA_API
int
atra_shader_getuniformloc(Shader *shader, const char *name)
{
    return glGetUniformLocation(shader->prog_id, name);
}

/* Returns the location of a shader attribute by name */
ATRA_API
int
atra_shader_getattribloc(Shader *shader, const char *name)
{
    return glGetAttribLocation(shader->prog_id, name);
}

/* Sets the value of a shader uniform */
ATRA_API
void
atra_shader_setuniform(int loc, const void *data, int type, int count)
{
    switch (type) {    
    case ATRA_SHADER_UNIFORM_FLOAT:   glUniform1fv(loc, count, data); break;
    case ATRA_SHADER_UNIFORM_VEC2:    glUniform2fv(loc, count, data); break;
    case ATRA_SHADER_UNIFORM_VEC3:    glUniform3fv(loc, count, data); break;
    case ATRA_SHADER_UNIFORM_VEC4:    glUniform4fv(loc, count, data); break;
    case ATRA_SHADER_UNIFORM_INT:     glUniform1iv(loc, count, data); break;
    case ATRA_SHADER_UNIFORM_IVEC2:   glUniform2iv(loc, count, data); break;
    case ATRA_SHADER_UNIFORM_IVEC3:   glUniform3iv(loc, count, data); break;
    case ATRA_SHADER_UNIFORM_IVEC4:   glUniform4iv(loc, count, data); break;
    case ATRA_SHADER_UNIFORM_TEXTURE: glUniform1iv(loc, count, data); break;
    case ATRA_SHADER_UNIFORM_MAT4:    glUniformMatrix4fv(loc, count, false, data); break;
    default:
        atra_log_print(LOG_WARN, "shader: unable to set uniform data: " \
                "invalid uniform data type");
    }
}

/* Unloads the shader from the GPU */
ATRA_API
void
atra_shader_destroy(Shader *shader)
{
    glDeleteProgram(shader->prog_id);
    free(shader->locs);
}
