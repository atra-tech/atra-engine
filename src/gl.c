/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  gl.c - Provides an interface for OpenGL operations with OpenGL version
 *         checks and extension availability detection.  Also provides a
 *         rendering system which mimics the classic immediate-mode style.
 *
 *  */

#include "atra.h"

const char *gl_type_str[] = {
    "ERROR",
    "DEPRECATED_BEHAVIOR",
    "UNDEFINED_BEHAVIOR",
    "PORTABILITY",
    "PERFORMANCE",
    "OTHER",
    "MARKER"
};

const char *gl_source_str[] = {
    "API",
    "WINDOW_SYSTEM",
    "SHADER_COMPILER",
    "THIRD_PARTY",
    "APPLICATION",
    "OTHER",
};

static RenderBatch *current_batch = {0};
static RenderBatch default_batch = {0};
static GLExtSupport gl_ext_support = {0};
static GLLimits gl_limits = {0};
static GLState gl_state = {0};

/*
 *  Main module functions
 */

/* OpenGL error message callback */
void
GLAPIENTRY
MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar *message, const void *user_param)
{
    type -= GL_DEBUG_TYPE_ERROR;
    source -= GL_DEBUG_SOURCE_API;

    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        atra_log_print(LOG_FATAL, "<%s> (%s) gl: %s",
                gl_type_str[type], gl_source_str[source], message);
    } else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
        atra_log_print(LOG_ERROR, "<%s> (%s) gl: %s",
                gl_type_str[type], gl_source_str[source], message);
    } else if (severity == GL_DEBUG_SEVERITY_LOW) {
        atra_log_print(LOG_WARN, "<%s> (%s) gl: %s",
                gl_type_str[type], gl_source_str[source], message);
    } else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        /*atra_log_print(LOG_INFO, "<%s> (%s) gl: %s",
                gl_type_str[type], gl_source_str[source], message);*/
    }
}

/* Initialises the OpenGL subsystem */
ATRA_API
void
atra_gl_init(int width, int height)
{
    u8 color[4] = {255, 255, 255, 255};
    int i;
    
    atra_gl_detectextensions();
    atra_gl_detectlimits();

    gl_state.def_tex_id = atra_gl_tex_create(color, 1, 1, 1);

    atra_gl_shader_loaddefault();
    gl_state.shader_id = gl_state.def_shader_id;
    gl_state.shader_locs = gl_state.def_shader_locs;

    default_batch = atra_gl_batch_create(ATRA_DEFAULT_BATCH_BUFFERS,
                                         ATRA_DEFAULT_BATCH_BUFFER_ELEMS);
    current_batch = &default_batch;

    for (i = 0; i < ATRA_MAX_MATRIX_STACK_SIZE; i++) {
        glm_mat4_identity(gl_state.stack[i]);
    }
    glm_mat4_identity(gl_state.transform);
    glm_mat4_identity(gl_state.proj);
    glm_mat4_identity(gl_state.modelview);

    gl_state.matrix = &gl_state.modelview;

    /* Initialise default OpenGL states */
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_DEPTH_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    gl_state.fbuffer_width = width;
    gl_state.fbuffer_height = height;

    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallbackARB(MessageCallback, NULL);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/* Stops the OpenGL subsystem and frees all allocated memory */
ATRA_API
void
atra_gl_stop(void)
{
    atra_gl_batch_delete(&default_batch);
    atra_gl_shader_deletedefault();
    glDeleteTextures(1, &gl_state.def_tex_id);
}

/* Returns an enumerated ID of the currently used OpenGL version */
ATRA_API
int
atra_gl_getversion(void)
{
}

/* Runs checks to find which OpenGL features are available */
ATRA_API
void
atra_gl_detectextensions(void)
{
    gl_ext_support.exts[ATRA_GL_EXT_VAO] = 1;
    gl_ext_support.exts[ATRA_GL_EXT_INSTANCING] = 1;
    gl_ext_support.exts[ATRA_GL_EXT_TEX_ARRAY] = 1;
    gl_ext_support.exts[ATRA_GL_EXT_TEX_NPOT] = 1;
    gl_ext_support.exts[ATRA_GL_EXT_TEX_DEPTH] = 1;
    gl_ext_support.exts[ATRA_GL_EXT_TEX_FLOAT32] = 1;
    gl_ext_support.exts[ATRA_GL_EXT_TEX_MIRRORCLAMP] = 1;
    gl_ext_support.exts[ATRA_GL_EXT_TEX_ANISOFILTER] = 1;
    gl_ext_support.exts[ATRA_GL_EXT_TEX_FLOAT32] = 1;
}

/* Checks for limits to some GPU capabilities, such as max texture size */
ATRA_API
void
atra_gl_detectlimits(void)
{
}

/* Returns a pointer to the internal GL state */
ATRA_API
GLState *
atra_gl_getinternalstate(void)
{
    return &gl_state;
}


/*
 *  Render batch functions
 */

/* Creates a new render batch */
ATRA_API
RenderBatch
atra_gl_batch_create(int num_buffers, int buffer_elems)
{
    RenderBatch batch = {0};
    int i, j, k;

    /* Set up vertex buffers for drawing quads */
    batch.vbuffer = malloc(num_buffers * sizeof(VertexBuffer));

    for (i = 0; i < num_buffers; i++) {
        batch.vbuffer[i].elem_count = buffer_elems;

        batch.vbuffer[i].verts   = malloc(buffer_elems * 3 * 4 * sizeof(float));
        batch.vbuffer[i].uvs     = malloc(buffer_elems * 2 * 4 * sizeof(float));
        batch.vbuffer[i].colors  = malloc(buffer_elems * 4 * 4 * sizeof(u8));
        batch.vbuffer[i].indices = malloc(buffer_elems * 6 * sizeof(uint));

        for (j = 0; j < 3 * 4 * buffer_elems; j++) {
            batch.vbuffer[i].verts[j] = 0.0f;
        }
        for (j = 0; j < 2 * 4 * buffer_elems; j++) {
            batch.vbuffer[i].uvs[j] = 0.0f;
        }
        for (j = 0; j < 4 * 4 * buffer_elems; j++) {
            batch.vbuffer[i].colors[j] = 0;
        }
        k = 0;

        /* Initialise indices */
        for (j = 0; j < 6 * buffer_elems; j += 6) {
            batch.vbuffer[i].indices[j + 0] = 4 * k + 0;
            batch.vbuffer[i].indices[j + 1] = 4 * k + 1;
            batch.vbuffer[i].indices[j + 2] = 4 * k + 2;
            batch.vbuffer[i].indices[j + 3] = 4 * k + 0;
            batch.vbuffer[i].indices[j + 4] = 4 * k + 2;
            batch.vbuffer[i].indices[j + 5] = 4 * k + 3;
            k++;
        }

        batch.vbuffer[i].pos_count = 0;
        batch.vbuffer[i].uv_count = 0;
        batch.vbuffer[i].color_count = 0;
    }

    /* Upload to GPU and initialise vertex array and buffers */

    for (i = 0; i < num_buffers; i++) {
        if (gl_ext_support.exts[ATRA_GL_EXT_VAO]) {
            glGenVertexArrays(1, &batch.vbuffer[i].vao_id);
            glBindVertexArray(batch.vbuffer[i].vao_id);
        }

        /* Positions */
        glGenBuffers(1, &batch.vbuffer[i].vbo_ids[0]);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vbuffer[i].vbo_ids[0]);
        glBufferData(
            GL_ARRAY_BUFFER,
            buffer_elems * 3 * 4 * sizeof(float),
            batch.vbuffer[i].verts,
            GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(
            gl_state.shader_locs[ATRA_SHADER_LOC_V_POSITION]);
        glVertexAttribPointer(
            gl_state.shader_locs[ATRA_SHADER_LOC_V_POSITION],
            3, GL_FLOAT, 0, 0, 0);

        /* UV coordinates */
        glGenBuffers(1, &batch.vbuffer[i].vbo_ids[1]);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vbuffer[i].vbo_ids[1]);
        glBufferData(
            GL_ARRAY_BUFFER,
            buffer_elems * 2 * 4 * sizeof(float),
            batch.vbuffer[i].uvs,
            GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(
            gl_state.shader_locs[ATRA_SHADER_LOC_V_TEXCOORD]);
        glVertexAttribPointer(
            gl_state.shader_locs[ATRA_SHADER_LOC_V_TEXCOORD],
            2, GL_FLOAT, 0, 0, 0);
        
        /* Colors */
        glGenBuffers(1, &batch.vbuffer[i].vbo_ids[2]);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vbuffer[i].vbo_ids[2]);
        glBufferData(
            GL_ARRAY_BUFFER,
            buffer_elems * 4 * 4 * sizeof(uchar),
            batch.vbuffer[i].colors,
            GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(
            gl_state.shader_locs[ATRA_SHADER_LOC_V_COLOR]);
        glVertexAttribPointer(
            gl_state.shader_locs[ATRA_SHADER_LOC_V_COLOR],
            4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

        /* Fill index buffer */
        glGenBuffers(1, &batch.vbuffer[i].vbo_ids[3]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.vbuffer[i].vbo_ids[3]);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            buffer_elems * 6 * sizeof(int),
            batch.vbuffer[i].indices,
            GL_STATIC_DRAW);
    }

    /* Unbind the current vertex array */
    if (gl_ext_support.exts[ATRA_GL_EXT_VAO]) {
        glBindVertexArray(0);
    }

    /* Initialise draw calls tracking system */
    batch.draws = malloc(ATRA_DEFAULT_BATCH_DRAWCALLS * sizeof(DrawCall));

    for (i = 0; i < ATRA_DEFAULT_BATCH_DRAWCALLS; i++) {
        batch.draws[i].mode = ATRA_QUADS;
        batch.draws[i].vert_count = 0;
        batch.draws[i].vert_align = 0;
        batch.draws[i].tex_id = gl_state.def_tex_id;
    }
    batch.buff_count = num_buffers;
    batch.draw_count = 1;
    batch.curr_depth = -1.0f;

    return batch;
}

/* Unloads a render batch */
ATRA_API
void
atra_gl_batch_delete(RenderBatch *batch)
{
    int i;

    /* Unbind VAO and VBOs */
    if (gl_ext_support.exts[ATRA_GL_EXT_VAO]) {
        glBindVertexArray(0);
    }
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* Unload all data from the vertex buffers */
    for (i = 0; i < batch->buff_count; i++) {
        glDeleteBuffers(1, &batch->vbuffer[i].vbo_ids[0]);
        glDeleteBuffers(1, &batch->vbuffer[i].vbo_ids[1]);
        glDeleteBuffers(1, &batch->vbuffer[i].vbo_ids[2]);
        glDeleteBuffers(1, &batch->vbuffer[i].vbo_ids[3]);

        if (gl_ext_support.exts[ATRA_GL_EXT_VAO]) {
            glDeleteVertexArrays(1, &batch->vbuffer[i].vao_id);
        }
        free(batch->vbuffer[i].verts);
        free(batch->vbuffer[i].uvs);
        free(batch->vbuffer[i].colors);
        free(batch->vbuffer[i].indices);
    }
    free(batch->vbuffer);
    free(batch->draws);
}

/* Draws render batch data (update -> draw -> reset) */
ATRA_API
void
atra_gl_batch_draw(RenderBatch *batch)
{
    VertexBuffer *vbuffer = batch->vbuffer + batch->curr_buff;
    mat4 mvp, proj, modelview;
    int i, vertex_offset;

    /* 
     * TODO: If no data changed CPU-side, then no update required GPU-side (change flag required)
     */

    if (vbuffer->pos_count > 0) {
        /* Save current matrices to restore later */
        glm_mat4_copy(gl_state.proj, proj);
        glm_mat4_copy(gl_state.modelview, modelview);

        /* Update batch vertex buffers */
        atra_gl_vao_enable(vbuffer->vao_id);
        {
            /* Positions */
            atra_gl_vbo_update(vbuffer->vbo_ids[0], vbuffer->verts,
                               vbuffer->pos_count * 3 * sizeof(float), 0);

            /* UV coordinates */
            atra_gl_vbo_update(vbuffer->vbo_ids[1], vbuffer->uvs,
                               vbuffer->uv_count * 2 * sizeof(float), 0);

            /* Colors */
            atra_gl_vbo_update(vbuffer->vbo_ids[2], vbuffer->colors,
                               vbuffer->color_count * 4 * sizeof(uchar), 0);
        }
        atra_gl_vao_disable();

        /* Draw buffers */
        atra_gl_shader_enable(gl_state.shader_id);
        glm_mat4_mul(gl_state.proj, gl_state.modelview, mvp);
        atra_gl_shader_setuniform(gl_state.shader_locs[ATRA_SHADER_LOC_M_MVP],
            mvp, ATRA_SHADER_UNIFORM_MAT4, 1);
        
        if (gl_ext_support.exts[ATRA_GL_EXT_VAO]) {
            atra_gl_vao_enable(vbuffer->vao_id);
        } else {
            /* Positions */
            atra_gl_vbo_enable(vbuffer->vbo_ids[0]);
            atra_gl_vao_setattrib(
                    gl_state.shader_locs[ATRA_SHADER_LOC_V_POSITION],
                    3, GL_FLOAT, 0, 0, 0);
            atra_gl_vao_enableattrib(
                    gl_state.shader_locs[ATRA_SHADER_LOC_V_POSITION]);

            /* UV coordinates */
            atra_gl_vbo_enable(vbuffer->vbo_ids[1]);
            atra_gl_vao_setattrib(
                    gl_state.shader_locs[ATRA_SHADER_LOC_V_TEXCOORD],
                    2, GL_FLOAT, 0, 0, 0);
            atra_gl_vao_enableattrib(
                    gl_state.shader_locs[ATRA_SHADER_LOC_V_TEXCOORD]);

            /* Colors */
            atra_gl_vbo_enable(vbuffer->vbo_ids[2]);
            atra_gl_vao_setattrib(
                    gl_state.shader_locs[ATRA_SHADER_LOC_V_COLOR],
                    4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
            atra_gl_vao_enableattrib(
                    gl_state.shader_locs[ATRA_SHADER_LOC_V_COLOR]);

            /* Indices */
            atra_gl_ebo_enable(vbuffer->vbo_ids[3]);
        }

        /* Bind additional textures to texture units */
        glUniform4f(gl_state.shader_locs[ATRA_SHADER_LOC_COLOR_DIFFUSE],
                1.0f, 1.0f, 1.0f, 1.0f);
        glUniform1i(gl_state.shader_locs[ATRA_SHADER_LOC_TEX_ALBEDO], 0);

        for (i = 0; i < ATRA_DEFAULT_BATCH_MAX_TEXTURE_UNITS; i++) {
            if (gl_state.active_tex_ids[i] > 0) {
                atra_gl_tex_setactiveunit(1 + i);
                atra_gl_tex_enable(gl_state.active_tex_ids[i]);
            }
        }
        
        /* Activate default texture */
        atra_gl_tex_setactiveunit(0);

        for (i = 0, vertex_offset = 0; i < batch->draw_count; i++) {
            atra_gl_tex_enable(batch->draws[i].tex_id);

            if (batch->draws[i].mode == ATRA_LINES ||
                    batch->draws[i].mode == ATRA_TRIANGLES) {
                glDrawArrays(
                        batch->draws[i].mode,
                        vertex_offset,
                        batch->draws[i].vert_count);
            } else {
                glDrawElements(GL_TRIANGLES,
                        batch->draws[i].vert_count / 4 * 6,
                        GL_UNSIGNED_INT,
                        (void *)(vertex_offset / 4 * 6 * sizeof(uint)));
            }
            vertex_offset += batch->draws[i].vert_count +
                             batch->draws[i].vert_align;
        }

        if (gl_ext_support.exts[ATRA_GL_EXT_VAO]) {
            atra_gl_vao_disable();
        } else {
            atra_gl_vbo_disable();
            atra_gl_ebo_disable();
        }
        atra_gl_tex_disable();
        atra_gl_shader_disable();
    } else {
        return;
    }

    /* Reset batch for next draw */
    vbuffer->pos_count = 0;
    vbuffer->uv_count = 0;
    vbuffer->color_count = 0;

    batch->curr_depth = -1.0f;

    glm_mat4_copy(proj, gl_state.proj);
    glm_mat4_copy(modelview, gl_state.modelview);

    for (i = 0; i < ATRA_DEFAULT_BATCH_DRAWCALLS; i++) {
        batch->draws[i].mode = ATRA_QUADS;
        batch->draws[i].vert_count = 0;
        batch->draws[i].tex_id = gl_state.def_tex_id;
    }

    for (i = 0; i < ATRA_DEFAULT_BATCH_MAX_TEXTURE_UNITS; i++) {
        gl_state.active_tex_ids[i] = 0;
    }
    batch->draw_count = 1;
    batch->curr_buff++;

    if (batch->curr_buff >= batch->buff_count) {
        batch->curr_buff = 0;
    }
}

/* Sets the active internal render batch (NULL for default) */
ATRA_API
void
atra_gl_batch_setactive(RenderBatch *batch)
{
    atra_gl_batch_draw(current_batch);

    if (batch != NULL) {
        current_batch = batch;
    } else {
        current_batch = &default_batch;
    }
}

/* Draws the active internal render batch */
ATRA_API
void
atra_gl_batch_drawactive(void)
{
    atra_gl_batch_draw(current_batch);
}

/* Returns 1 if adding `vcount` vertices will overflow the buffer */
ATRA_API
int
atra_gl_batch_checklimit(int vcount)
{
    int overflow = 0;
    VertexBuffer *vbuffer = &current_batch->vbuffer[current_batch->curr_buff];

    if (vbuffer->pos_count + vcount >= vbuffer->elem_count * 4) {
        overflow = 1;
        atra_gl_batch_draw(current_batch);
    }

    return overflow;
}

/* Set the current texture for the render batch */
ATRA_API
void
atra_gl_batch_settexture(uint tex_id)
{
    VertexBuffer *vbuffer;
    DrawCall *draw_call;

    vbuffer = current_batch->vbuffer + current_batch->curr_buff;
    draw_call = current_batch->draws + current_batch->draw_count - 1;

    if (tex_id == 0) {
        if (vbuffer->pos_count >= vbuffer->elem_count * 4) {
            atra_gl_batch_draw(current_batch);
        }
    } else {
        if (draw_call->tex_id != tex_id) {
            if (draw_call->vert_count > 0) {
                if (draw_call->mode == ATRA_LINES) {
                    draw_call->vert_align = draw_call->vert_count % 4;
                } else if (draw_call->mode == ATRA_TRIANGLES) {
                    draw_call->vert_align = (draw_call->vert_count < 4) ?
                            1 : 4 - (draw_call->vert_count % 4);
                } else {
                    draw_call->vert_align = 0;
                }

                if (atra_gl_batch_checklimit(draw_call->vert_align) == 0) {
                    vbuffer->pos_count += draw_call->vert_align;
                    vbuffer->uv_count += draw_call->vert_align;
                    vbuffer->color_count += draw_call->vert_align;

                    current_batch->draw_count++;
                    draw_call++;
                }
            }

            if (current_batch->draw_count >= ATRA_DEFAULT_BATCH_DRAWCALLS) {
                atra_gl_batch_draw(current_batch);
            }
            draw_call->tex_id = tex_id;
            draw_call->vert_count = 0;
        }
    }
}


/*  
 *  Matrix functions
 */

/* Copies the internal projection matrix into `m` */
ATRA_API
void
atra_gl_matrix_getprojection(vec4 *m)
{
    glm_mat4_copy(gl_state.proj, m);
}

/* Copies the internal model-view matrix into `m` */
ATRA_API
void
atra_gl_matrix_getmodelview(vec4 *m)
{
    glm_mat4_copy(gl_state.modelview, m);
}

/* Copies the internal transformation matrix into `m` */
ATRA_API
void
atra_gl_matrix_gettransform(vec4 *m)
{
    glm_mat4_copy(gl_state.transform, m);
}

/* Replace the internal projection matrix with the contents of `m` */
ATRA_API
void
atra_gl_matrix_setprojection(vec4 *m)
{
    glm_mat4_copy(m, gl_state.proj);
}

/* Replace the internal model-view matrix with the contents of `m` */
ATRA_API
void
atra_gl_matrix_setmodelview(vec4 *m)
{
    glm_mat4_copy(m, gl_state.modelview);
}


/*
 *  Immediate-mode vertex operations
 */

/* Initialise immediate drawing mode */
ATRA_API
void
atra_gl_begin(int mode)
{
    VertexBuffer *vbuffer;
    DrawCall *draw_call;

    vbuffer = current_batch->vbuffer + current_batch->curr_buff;
    draw_call = current_batch->draws + current_batch->draw_count - 1;

    if (draw_call->mode != mode) {
        if (draw_call->vert_count > 0) {
            if (draw_call->mode == ATRA_LINES) {
                if (draw_call->vert_count < 4) {
                    draw_call->vert_align = draw_call->vert_count;
                } else {
                    draw_call->vert_align = draw_call->vert_count % 4;
                }
            } else if (draw_call->mode == ATRA_TRIANGLES) {
                if (draw_call->vert_count < 4) {
                    draw_call->vert_align = 1;
                } else {
                    draw_call->vert_align = 4 - (draw_call->vert_count % 4);
                }
            } else {
                draw_call->vert_align = 0;
            }

            if (atra_gl_batch_checklimit(draw_call->vert_align) == 0) {
                vbuffer->pos_count += draw_call->vert_align;
                vbuffer->uv_count += draw_call->vert_align;
                vbuffer->color_count += draw_call->vert_align;

                current_batch->draw_count++;
                draw_call++;
            }
        }

        if (current_batch->draw_count >= ATRA_DEFAULT_BATCH_DRAWCALLS) {
            atra_gl_batch_draw(current_batch);
        }
        draw_call->mode = mode;
        draw_call->vert_count = 0;
        draw_call->tex_id = gl_state.def_tex_id;
    }
}

/* End immediate drawing mode */
ATRA_API
void
atra_gl_end(void)
{
    VertexBuffer *vbuffer;
    int add_count;
    int i;

    vbuffer = current_batch->vbuffer + current_batch->curr_buff;

    /* Ensure color count matches vertex position count */
    if (vbuffer->pos_count != vbuffer->color_count) {
        add_count = vbuffer->pos_count - vbuffer->color_count;

        for (i = 0; i < add_count; i++) {
            vbuffer->colors[4 * vbuffer->color_count + 0] = vbuffer->colors[4 * vbuffer->color_count - 4];
            vbuffer->colors[4 * vbuffer->color_count + 1] = vbuffer->colors[4 * vbuffer->color_count - 3];
            vbuffer->colors[4 * vbuffer->color_count + 2] = vbuffer->colors[4 * vbuffer->color_count - 2];
            vbuffer->colors[4 * vbuffer->color_count + 3] = vbuffer->colors[4 * vbuffer->color_count - 1];
            vbuffer->color_count++;
        }
    }

    /* Ensure UV coordinate count matches vertex position count */
    if (vbuffer->pos_count != vbuffer->uv_count) {
        add_count = vbuffer->pos_count - vbuffer->uv_count;

        for (i = 0; i < add_count; i++) {
            vbuffer->uvs[2 * vbuffer->uv_count + 0] = 0.0f;
            vbuffer->uvs[2 * vbuffer->uv_count + 1] = 0.0f;
            vbuffer->uv_count++;
        }
    }

    /* TODO:  Add array size correction for normals */

    current_batch->curr_depth += (1.0f / 20000.0f);

    if (vbuffer->pos_count >= vbuffer->elem_count * 4 - 4) {
        for (i = gl_state.stack_size; i >= 0; i--) {
            atra_gl_popmatrix();
        }
        atra_gl_batch_draw(current_batch);
    }
}

/* Initialises internal matrices for 2D rendering using a camera */
ATRA_API
void
atra_gl_begin2D(Camera *cam)
{
    atra_gl_batch_drawactive();

    atra_gl_loadidentity();
    atra_gl_multmatrix(cam->proj);
    atra_gl_multmatrix(*atra_window_getscreenscalemat());
}

/* Cleans up matrices after 2D rendering */
ATRA_API
void
atra_gl_end2D(void)
{
    atra_gl_batch_drawactive();

    atra_gl_loadidentity();
    atra_gl_multmatrix(*atra_window_getscreenscalemat());
}

/* Initialises internal matrices for 3D rendering using a camera */
ATRA_API
void
atra_gl_begin3D(Camera *cam)
{
    atra_gl_batch_drawactive();

    atra_gl_matrixmode(ATRA_PROJECTION);
    atra_gl_pushmatrix();
    atra_gl_loadidentity();

    atra_gl_multmatrix(cam->proj);

    atra_gl_matrixmode(ATRA_MODELVIEW);
    atra_gl_loadidentity();

    atra_gl_multmatrix(cam->view);

    atra_gl_enabledepthtest();
}

/* Cleans up matrices after 3D rendering */
ATRA_API
void
atra_gl_end3D(void)
{
    atra_gl_batch_drawactive();

    atra_gl_matrixmode(ATRA_PROJECTION);
    atra_gl_popmatrix();

    atra_gl_matrixmode(ATRA_MODELVIEW);
    atra_gl_loadidentity();

    //atra_gl_multmatrix(*atra_window_getscreenscalemat());
    atra_gl_disabledepthtest();
}


/* Define a 2D vertex position (int) */
ATRA_API
void
atra_gl_vertex2i(int x, int y)
{
    atra_gl_vertex3f((float)x, (float)y, current_batch->curr_depth);
}

/* Define a 2D vertex position (float) */
ATRA_API
void
atra_gl_vertex2f(float x, float y)
{
    atra_gl_vertex3f(x, y, current_batch->curr_depth);
}

/* Define a 3D vertex position (float) */
ATRA_API
void
atra_gl_vertex3f(float x, float y, float z)
{
    VertexBuffer *vbuffer;
    float tx, ty, tz;
    vec4 *m;

    tx = x;
    ty = y;
    tz = z;
    vbuffer = current_batch->vbuffer + current_batch->curr_buff;

    if (gl_state.transform_req) {
        m = gl_state.transform;
        tx = m[0][0] * x + m[1][0] * y + m[2][0] * z + m[3][0];
        ty = m[0][1] * x + m[1][1] * y + m[2][1] * z + m[3][1];
        tz = m[0][2] * x + m[1][2] * y + m[2][2] * z + m[3][2];
    }

    if (vbuffer->pos_count < vbuffer->elem_count * 4) {
        vbuffer->verts[3 * vbuffer->pos_count + 0] = tx;
        vbuffer->verts[3 * vbuffer->pos_count + 1] = ty;
        vbuffer->verts[3 * vbuffer->pos_count + 2] = tz;
        vbuffer->pos_count++;

        current_batch->draws[current_batch->draw_count - 1].vert_count++;
    } else {
        atra_log_print(LOG_ERROR, "gl: batch elements overflow");
    }
}

/* Define a vertex texture UV coordinate */
ATRA_API
void
atra_gl_texcoord2f(float x, float y)
{
    VertexBuffer *vbuffer;

    vbuffer = current_batch->vbuffer + current_batch->curr_buff;

    vbuffer->uvs[2 * vbuffer->uv_count + 0] = x;
    vbuffer->uvs[2 * vbuffer->uv_count + 1] = y;

    vbuffer->uv_count++;
}

/* Define a vertex normal */
ATRA_API
void
atra_gl_normal3f(float x, float y, float z)
{
    /* TODO: Add normals */
    ATRA_UNUSED(x);
    ATRA_UNUSED(y);
    ATRA_UNUSED(z);
}

/* Define a vertex color (RGBA8) */
ATRA_API
void
atra_gl_color4ub(u8 r, u8 g, u8 b, u8 a)
{
    VertexBuffer *vbuffer;

    vbuffer = current_batch->vbuffer + current_batch->curr_buff;

    vbuffer->colors[4 * vbuffer->color_count + 0] = r;
    vbuffer->colors[4 * vbuffer->color_count + 1] = g;
    vbuffer->colors[4 * vbuffer->color_count + 2] = b;
    vbuffer->colors[4 * vbuffer->color_count + 3] = a;
    
    vbuffer->color_count++;
}

/* Define a vertex color (RGB) */
ATRA_API
void
atra_gl_color3f(float r, float g, float b)
{
    atra_gl_color4ub((u8)(r * 255), (u8)(g * 255), (u8)(b * 255), 255);
}

/* Define a vertex color (RGBA) */
ATRA_API
void
atra_gl_color4f(float r, float g, float b, float a)
{
    atra_gl_color4ub((u8)(r * 255), (u8)(g * 255), (u8)(b * 255), (u8)(a * 255));
}


/* 
 *  Immediate-mode matrix operations
 */

/* Selects the current matrix to manipulate */
ATRA_API
void
atra_gl_matrixmode(int mode)
{
    if (mode == ATRA_PROJECTION) {
        gl_state.matrix = &gl_state.proj;
    } else if (mode == ATRA_MODELVIEW) {
        gl_state.matrix = &gl_state.modelview;
    }
    gl_state.matrix_mode = mode;
}

/* Pushes the current matrix to the matrix stack */
ATRA_API
void
atra_gl_pushmatrix(void)
{
    if (gl_state.stack_size >= ATRA_MAX_MATRIX_STACK_SIZE) {
        atra_log_print(LOG_ERROR, "gl: matrix stack overflow");
    }

    if (gl_state.matrix_mode == ATRA_MODELVIEW) {
        gl_state.transform_req = 1;
        gl_state.matrix = &gl_state.transform;
    }
    glm_mat4_copy(*gl_state.matrix, gl_state.stack[gl_state.stack_size]);
    gl_state.stack_size++;
}

/* Pops the matrix at the top of the matrix stack and makes it current */
ATRA_API
void
atra_gl_popmatrix(void)
{
    if (gl_state.stack_size > 0) {
        glm_mat4_copy(gl_state.stack[gl_state.stack_size - 1], *gl_state.matrix);
        gl_state.stack_size--;
    }

    if (gl_state.stack_size == 0 && gl_state.matrix_mode == ATRA_MODELVIEW) {
        gl_state.matrix = &gl_state.modelview;
        gl_state.transform_req = 0;
    }
}

/* Loads the identity matrix as current */
ATRA_API
void
atra_gl_loadidentity(void)
{
    glm_mat4_identity(*gl_state.matrix);
}

/* Translates the current matrix */
ATRA_API
void
atra_gl_translate(float x, float y, float z)
{
    vec3 trans_vec = {x, y, z};

    glm_translate(*gl_state.matrix, trans_vec);
}

/* Rotates the current matrix */
ATRA_API
void
atra_gl_rotate(float angle, float x, float y, float z)
{
    vec3 rot_vec = {x, y, z};

    glm_rotate(*gl_state.matrix, angle, rot_vec);
}

/* Scales the current matrix */
ATRA_API
void
atra_gl_scale(float x, float y, float z)
{
    vec3 scale_vec = {x, y, z};

    glm_scale(*gl_state.matrix, scale_vec);
}

/* Multiplies a matrix with the current matrix */
ATRA_API
void
atra_gl_multmatrix(vec4 *mat)
{
    glm_mat4_mul(*gl_state.matrix, mat, *gl_state.matrix);
}

/* Multiplies the current matrix with a projection matrix */
ATRA_API
void
atra_gl_frustum(double left, double right, double bottom,
        double top, double znear, double zfar)
{
    mat4 matrix;

    glm_frustum(left, right, bottom, top, znear, zfar, matrix);
    glm_mat4_mul(*gl_state.matrix, matrix, *gl_state.matrix);
}

/* Multiplies the current matrix with an orthographic matrix*/
ATRA_API
void
atra_gl_ortho(double left, double right, double bottom,
        double top, double znear, double zfar)
{
    mat4 matrix;

    glm_ortho(left, right, bottom, top, znear, zfar, matrix);
    glm_mat4_mul(*gl_state.matrix, matrix, *gl_state.matrix);
}

/* Sets the current viewport area */
ATRA_API
void
atra_gl_viewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}


/*
 *  OpenGL wrapper function segment
 *
 *  Most functions here are simply version-aware wrappers around
 *  OpenGL functions.
 *
 *  Many of these functions also have analogs in modules that further abstract
 *  their functionality. It is recommended to use those modules instead where
 *  it is possible to do so.
 */

/*
 *  OpenGL wrapper functions (VAO / VBO)
 */

/* Creates a new vertex array */
ATRA_API
uint
atra_gl_vao_create(void)
{
    uint vao_id = 0;

    if (gl_ext_support.exts[ATRA_GL_EXT_VAO]) {
        glGenVertexArrays(1, &vao_id);
    }

    return vao_id;
}

/* Creates a new vertex buffer */
ATRA_API
uint
atra_gl_vbo_create(void *buffer, int size, int dynamic)
{
    const uint draw_mode = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    uint vbo_id;

    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, size, buffer, draw_mode);

    return vbo_id;
}

/* Creates a new element buffer (usually for indices) */
ATRA_API
uint
atra_gl_ebo_create(void *buffer, int size, int dynamic)
{
    const uint draw_mode = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    uint ebo_id;

    glGenBuffers(1, &ebo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer, draw_mode);

    return ebo_id;
}

/* Deletes and replaces a vertex buffer with new data */
ATRA_API
void
atra_gl_vbo_overwrite(uint vbo_id, void *data, int size, int dynamic)
{
    const uint draw_mode = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, size, data, draw_mode);
}

/* Deletes and replaces a vertex buffer with new data */
ATRA_API
void
atra_gl_ebo_overwrite(uint ebo_id, void *data, int size, int dynamic)
{
    const uint draw_mode = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, draw_mode);
}

/* Updates a vertex buffer with new data */
ATRA_API
void
atra_gl_vbo_update(uint vbo_id, void *data, int size, int offset)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

/* Deletes the given vertex array */
ATRA_API
void
atra_gl_vao_delete(uint vao_id)
{
    if (gl_ext_support.exts[ATRA_GL_EXT_VAO]) {
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vao_id);
    }
}

/* Deletes the given vertex buffer */
ATRA_API
void
atra_gl_vbo_delete(uint vbo_id)
{
    glDeleteBuffers(1, &vbo_id);
}

/* Enables the given vertex array */
ATRA_API
int
atra_gl_vao_enable(uint vao_id)
{
    int success = 0;

    if (gl_ext_support.exts[ATRA_GL_EXT_VAO]) {
        glBindVertexArray(vao_id);
        success = 1;
    }

    return success;
}

/* Disables the current vertex array */
ATRA_API
void
atra_gl_vao_disable(void)
{
    if (gl_ext_support.exts[ATRA_GL_EXT_VAO]) {
        glBindVertexArray(0);
    }
}

/* Enables the given vertex buffer */
ATRA_API
void
atra_gl_vbo_enable(uint vbo_id)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
}

/* Disables the current vertex buffer */
ATRA_API
void
atra_gl_vbo_disable(void)
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/* Enables the given vertex buffer element */
ATRA_API
void
atra_gl_ebo_enable(uint ebo_id)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
}

/* Disables the current vertex buffer element */
ATRA_API
void
atra_gl_ebo_disable(void)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/* Enable the given vertex attribute */
ATRA_API
void
atra_gl_vao_enableattrib(uint index)
{
    glEnableVertexAttribArray(index);
}

/* Disable the given vertex attribute */
ATRA_API
void
atra_gl_vao_disableattrib(uint index)
{
    glDisableVertexAttribArray(index);
}

/* Sets the data of a vertex attribute */
ATRA_API
void
atra_gl_vao_setattrib(uint index, int elem_size, int type, int normalised,
        int stride, void *pointer)
{
    glVertexAttribPointer(index, elem_size, type, normalised, stride, pointer);
}

/* Sets the divisor of a vertex attribute */
ATRA_API
void
atra_gl_vao_setattribdivisor(uint index, int divisor)
{
    glVertexAttribDivisor(index, divisor);
}

/* Sets the vertex attribute default value */
ATRA_API
void
atra_gl_vao_setattribdefault(int loc, const void *data, int type, int count)
{
    switch (type) {
    case ATRA_SHADER_ATTRIB_FLOAT:
        if (count == 1) {
            glVertexAttrib1fv(loc, data);
        }
        break;
    case ATRA_SHADER_ATTRIB_VEC2:
        if (count == 2) {
            glVertexAttrib2fv(loc, data);
        }
        break;
    case ATRA_SHADER_ATTRIB_VEC3:
        if (count == 3) {
            glVertexAttrib3fv(loc, data);
        }
        break;
    case ATRA_SHADER_ATTRIB_VEC4:
        if (count == 4) {
            glVertexAttrib4fv(loc, data);
        }
        break;
    default:
        atra_log_print(LOG_WARN, "gl: vao: Invalid attribute data type");
    }
}

/* Draws the current vertex array */
ATRA_API
void
atra_gl_vao_draw(int offset, int count)
{
    glDrawArrays(GL_TRIANGLES, offset, count);
}

/* Draws the current vertex array with indexed elements */
ATRA_API
void
atra_gl_vao_drawelements(int offset, int count, void *buffer)
{
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT,
            (uint *)buffer + offset);
}

/* Draws the current vertex array (instanced) */
ATRA_API
void
atra_gl_vao_drawinstanced(int offset, int count, int instances)
{
    glDrawArraysInstanced(GL_TRIANGLES, offset, count, instances);
}

/* Draws the current vertex array with indexed elements (instanced) */
ATRA_API
void
atra_gl_vao_drawelementsinstanced(int offset, int count, void *buffer, int instances)
{
    glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT,
            (uint *)buffer + offset, instances);
}


/*
 *  OpenGL wrapper functions (Texture)
 */

/* Creates a new texture on the GPU with the given data */
ATRA_API
uint
atra_gl_tex_create(void *data, int width, int height, int mip_levels)
{
    uint tex_id, mip_size;
    int mip_w, mip_h, mip_offset;
    int i;

    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    mip_w = width;
    mip_h = height;
    mip_offset = 0;
    
    for (i = 0; i < mip_levels; i++) {
        mip_size = atra_gl_tex_calcsize(mip_w, mip_h);

        glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, mip_w, mip_h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, (uchar *)data + mip_offset);

        mip_w /= 2;
        mip_h /= 2;
        mip_offset += mip_size;
        
        if (mip_w < 1) {
            mip_w = 1;
        }
        if (mip_h < 1) {
            mip_h = 1;
        }
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    /* GL_REPEAT texture wrapping and nearest neighbour filtering by default */
    if (mip_levels > 1) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    if (tex_id <= 0) {
        atra_log_print(LOG_ERROR, "gl: texture: failed to load texture");
    }

    return tex_id;
}

/* Creates a new depth buffer texture/renderbuffer for use with a FBO */
ATRA_API
uint
atra_gl_tex_createdepth(int width, int height, int use_rbuffer)
{
    uint tex_id;

    if (gl_ext_support.exts[ATRA_GL_EXT_TEX_DEPTH] == 0) {
        use_rbuffer = true;
    }

    if (use_rbuffer == 0 && gl_ext_support.exts[ATRA_GL_EXT_TEX_DEPTH]) {
        glGenTextures(1, &tex_id);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        glGenRenderbuffers(1, &tex_id);
        glBindRenderbuffer(GL_RENDERBUFFER, tex_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    return tex_id;
}

/* Deletes a texture from the GPU */
ATRA_API
void
atra_gl_tex_delete(uint tex_id)
{
    glDeleteTextures(1, &tex_id);
}

/* Creates a new cubemap texture */
ATRA_API
uint
atra_gl_tex_createcubemap(void *data, int size)
{
    uint tex_id;
    uint data_size;
    uint i;

    data_size = atra_gl_tex_calcsize(size, size);

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);

    for (i = 0; i < 6; i++) {
        if (data == NULL) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
                    size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        } else {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
                    size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                    (uchar *)data + i * data_size);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return tex_id;
}

/* Sets the currently active texture unit */
ATRA_API
void
atra_gl_tex_setactiveunit(int unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
}

/* Binds the given texture to the currently active texture unit */
ATRA_API
void
atra_gl_tex_enable(uint tex_id)
{
    glBindTexture(GL_TEXTURE_2D, tex_id);
}

/* Clears the currently active texture unit */
ATRA_API
void
atra_gl_tex_disable(void)
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

/* Bind the given cubemap texture to the currently active texture unit */
ATRA_API
void
atra_gl_tex_enablecubemap(uint tex_id)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
}

/* Clears the currently active texture unit of a cubemap */
ATRA_API
void
atra_gl_tex_disablecubemap(void)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

/* Set the given texture's parameters */
ATRA_API
void
atra_gl_tex_setparameter(uint tex_id, int param, int value)
{
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, param, value);
}

/* Updates part of a texture with new data, area specified by a rectangle */
ATRA_API
void
atra_gl_tex_update(uint tex_id, IQuad rect, const void *data)
{
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
            rect.x, rect.y, rect.w, rect.h,
            GL_RGBA,GL_UNSIGNED_BYTE, data);
}

/* Generates mipmaps for a texture */
ATRA_API
void
atra_gl_tex_genmipmaps(uint tex_id, int width, int height, int *mipmaps)
{
    int tex_ispow2;

    glBindTexture(GL_TEXTURE_2D, tex_id);

    if ((width > 0 && (width & (width - 1)) == 0) &&
            height > 0 && (height & (height - 1)) == 0) {
        tex_ispow2 = 1;
    } else {
        tex_ispow2 = 0;
    }

    if (tex_ispow2 || gl_ext_support.exts[ATRA_GL_EXT_TEX_NPOT]) {
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

        *mipmaps = 1 + (int)SDL_floor(SDL_log(MAX(width, height)) / SDL_log(2));
    } else {
        atra_log_print(LOG_WARN, "gl: texture: failed to generate mipmaps");
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

/* Reads pixel data from a texture */
ATRA_API
void *
atra_gl_tex_readpixels(uint tex_id, int width, int height)
{
    void *pixels;
    uint size;

    glBindTexture(GL_TEXTURE_2D, tex_id);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    size = atra_gl_tex_calcsize(width, height);
    pixels = malloc(size);
    
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    return pixels;
}

/* Reads pixel data from the screen's color buffer */
ATRA_API
u8 *
atra_gl_tex_readscreenpixels(int width, int height)
{
    u8 *screen_data, *img_data;
    int i, j;
    
    screen_data = calloc(width * height * 4, sizeof(u8));
    img_data = calloc(width * height * 4, sizeof(u8));

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, screen_data);

    /* Flip image vertically */
    for (i = height - 1; i >= 0; i--) {
        for (j = 0; j < width * 4; j++) {
            const int scr_index = j + i * width * 4;
            const int img_index = j + (height - i - 1) * width * 4;

            img_data[img_index] = screen_data[scr_index];

            if ((j + 1) % 4 == 0) {
                img_data[img_index] = 0xFF;
            }
        }
    }
    free(screen_data);

    return img_data;
}

/* Calculates the total size of an image, in bytes */
ATRA_API
int
atra_gl_tex_calcsize(int width, int height)
{
    return width * height * 4;
}

/* Returns the default texture ID */
ATRA_API
uint
atra_gl_tex_getdefaultid(void)
{
    return gl_state.def_tex_id;
}


/*
 *  OpenGL wrapper functions (Framebuffer)
 */

/* Loads a new, empty framebuffer */
ATRA_API
uint
atra_gl_fbo_create(void)
{
    uint fbo_id;

    glGenFramebuffers(1, &fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fbo_id;
}

/* Deletes an existing framebuffer from GPU */
ATRA_API
void
atra_gl_fbo_delete(uint fbo_id)
{
    int depth_type, depth_id;
    uint depth_id_u;

    depth_type = 0;
    depth_id = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &depth_type);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depth_id);

    depth_id_u = depth_id;

    if (depth_type == GL_RENDERBUFFER) {
        glDeleteRenderbuffers(1, &depth_id_u);
    } else if (depth_type == GL_TEXTURE_2D) {
        glDeleteTextures(1, &depth_id_u);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo_id);
}

/* Enable rendering to texture */
ATRA_API
void
atra_gl_fbo_enable(uint fbo_id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
}

/* Disable rendering to texture */
ATRA_API
void
atra_gl_fbo_disable(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/* Attaches a texture/renderbuffer to a framebuffer */
ATRA_API
void
atra_gl_fbo_attach(uint fbo_id, uint tex_id, int attach_type, int tex_type, int mip_level)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    switch (attach_type) {
    case ATRA_ATTACHMENT_COLOR_CHANNEL0:
    case ATRA_ATTACHMENT_COLOR_CHANNEL1:
    case ATRA_ATTACHMENT_COLOR_CHANNEL2:
    case ATRA_ATTACHMENT_COLOR_CHANNEL3:
    case ATRA_ATTACHMENT_COLOR_CHANNEL4:
    case ATRA_ATTACHMENT_COLOR_CHANNEL5:
    case ATRA_ATTACHMENT_COLOR_CHANNEL6:
    case ATRA_ATTACHMENT_COLOR_CHANNEL7:
        if (tex_type == ATRA_ATTACHMENT_TEXTURE_2D) {
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0 + attach_type,
                    GL_TEXTURE_2D,
                    tex_id, mip_level);
        } else if (tex_type == ATRA_ATTACHMENT_RENDERBUFFER) {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0 + attach_type,
                    GL_RENDERBUFFER,
                    tex_id);
        } else if (tex_type == ATRA_ATTACHMENT_CUBEMAP_POSITIVE_X) {
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0 + attach_type,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + tex_type,
                    tex_id, mip_level);
        }
        break;
    case ATRA_ATTACHMENT_DEPTH:
        if (tex_type == ATRA_ATTACHMENT_TEXTURE_2D) {
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    GL_TEXTURE_2D,
                    tex_id, mip_level);
        } else if (tex_type == ATRA_ATTACHMENT_RENDERBUFFER) {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    GL_RENDERBUFFER,
                    tex_id);
        }
        break;
    case ATRA_ATTACHMENT_STENCIL:
        if (tex_type == ATRA_ATTACHMENT_TEXTURE_2D) {
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                    GL_STENCIL_ATTACHMENT,
                    GL_TEXTURE_2D,
                    tex_id, mip_level);
        } else if (tex_type == ATRA_ATTACHMENT_RENDERBUFFER) {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                    GL_STENCIL_ATTACHMENT,
                    GL_RENDERBUFFER,
                    tex_id);
        }
        break;
    default:
        break;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/* Returns true if framebuffer is complete */
ATRA_API
int
atra_gl_fbo_iscomplete(uint fbo_id)
{
    int result = 0;
    uint status;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        switch (status) {
        case GL_FRAMEBUFFER_UNSUPPORTED:
            atra_log_print(LOG_WARN, "gl: fbo: framebuffer unsupported");
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            atra_log_print(LOG_WARN, "gl: fbo: framebuffer has incomplete attachment");
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            atra_log_print(LOG_WARN, "gl: fbo: framebuffer has a missing attachment");
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    result = (status == GL_FRAMEBUFFER_COMPLETE);

    return result;
}

/* Activate multiple draw color buffers */
ATRA_API
void
atra_gl_fbo_activatedrawbuffers(int count)
{
    if (count > 0 && count <= 8) {
        uint buffers[8] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4,
            GL_COLOR_ATTACHMENT5,
            GL_COLOR_ATTACHMENT6,
            GL_COLOR_ATTACHMENT7
        };
        glDrawBuffers(count, buffers);
    } else {
        atra_log_print(LOG_WARN, "gl: fbo: invalid buffer count");
    }
}


/*
 *  OpenGL wrapper functions (Shader)
 */

/* Returns the ID of a new shader program compiled from two code buffers.
 * If shader code string is NULL then the default shader will be used. */
ATRA_API
uint
atra_gl_shader_load(const char *vs_code, const char *fs_code)
{
    uint vshader, fshader, prog_id;

    vshader = atra_gl_shader_compile(vs_code, GL_VERTEX_SHADER);
    fshader = atra_gl_shader_compile(fs_code, GL_FRAGMENT_SHADER);

    prog_id = atra_gl_shader_link(vshader, fshader);

    return prog_id;
}

/* Loads the default shader */
ATRA_API
void
atra_gl_shader_loaddefault(void)
{
    int i;

    gl_state.def_shader_locs = calloc(ATRA_MAX_SHADER_LOCATIONS, sizeof(int));

    for (i = 0; i < ATRA_MAX_SHADER_LOCATIONS; i++) {
        gl_state.def_shader_locs[i] = -1;
    }

    const char *def_vshader_code =
#if defined(ATRA_OPENGL_VERSION_21)
    "#version 120                       \n"
    "attribute vec3 vert_pos;     \n"
    "attribute vec2 vert_uv;     \n"
    "attribute vec4 vert_color;        \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
#elif defined(ATRA_OPENGL_VERSION_33)
    "#version 330                       \n"
    "in vec3 vertexPosition;            \n"
    "in vec2 vertexTexCoord;            \n"
    "in vec4 vertexColor;               \n"
    "out vec2 fragTexCoord;             \n"
    "out vec4 fragColor;                \n"
#endif
    "uniform mat4 viewmat;              \n"
    "uniform mat4 projmat;              \n"
    "uniform mat4 modelmat;             \n"
    "uniform mat4 mvp;                  \n"
    "void main()                        \n"
    "{                                  \n"
    "    fragTexCoord = vert_uv; \n"
    "    fragColor = vert_color;       \n"
    "    gl_Position = mvp * vec4(vert_pos, 1.0); \n"
    "}                                  \n";

    const char *def_fshader_code =
#if defined(ATRA_OPENGL_VERSION_21)
    "#version 120                       \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "uniform sampler2D albedo_map;      \n"
    "uniform vec4 color;                \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture2D(albedo_map, fragTexCoord); \n"
    "    gl_FragColor = texelColor * color * fragColor;   \n"
    "}                                  \n";
#elif defined(ATRA_OPENGL_VERSION_33)
    "#version 330       \n"
    "in vec2 fragTexCoord;              \n"
    "in vec4 fragColor;                 \n"
    "out vec4 finalColor;               \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture(texture0, fragTexCoord);   \n"
    "    finalColor = texelColor*colDiffuse*fragColor;        \n"
    "}                                  \n";
#endif

    gl_state.def_vshader_id = atra_gl_shader_compile(def_vshader_code, GL_VERTEX_SHADER);
    gl_state.def_fshader_id = atra_gl_shader_compile(def_fshader_code, GL_FRAGMENT_SHADER);

    gl_state.def_shader_id = atra_gl_shader_link(gl_state.def_vshader_id, gl_state.def_fshader_id);

    if (gl_state.def_shader_id > 0) {
        gl_state.def_shader_locs[ATRA_SHADER_LOC_V_POSITION] = glGetAttribLocation(gl_state.def_shader_id, "vert_pos");
        gl_state.def_shader_locs[ATRA_SHADER_LOC_V_TEXCOORD] = glGetAttribLocation(gl_state.def_shader_id, "vert_uv");
        gl_state.def_shader_locs[ATRA_SHADER_LOC_V_COLOR]    = glGetAttribLocation(gl_state.def_shader_id, "vert_color");

        gl_state.def_shader_locs[ATRA_SHADER_LOC_M_MVP] = glGetUniformLocation(gl_state.def_shader_id, "mvp");
        gl_state.def_shader_locs[ATRA_SHADER_LOC_M_VIEW] = glGetUniformLocation(gl_state.def_shader_id, "viewmat");
        gl_state.def_shader_locs[ATRA_SHADER_LOC_M_PROJ] = glGetUniformLocation(gl_state.def_shader_id, "projmat");
        gl_state.def_shader_locs[ATRA_SHADER_LOC_M_MODEL] = glGetUniformLocation(gl_state.def_shader_id, "modelmat");
        gl_state.def_shader_locs[ATRA_SHADER_LOC_COLOR_DIFFUSE] = glGetUniformLocation(gl_state.def_shader_id, "color");
        gl_state.def_shader_locs[ATRA_SHADER_LOC_TEX_ALBEDO] = glGetUniformLocation(gl_state.def_shader_id, "albedo_map");
    } else {
        atra_log_print(LOG_WARN, "gl: shader: failed to load default shader");
    }
}

/* Returns the ID of a shader compiled from the given code */
ATRA_API
uint
atra_gl_shader_compile(const char *code, int type)
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
        case GL_FRAGMENT_SHADER:
            atra_log_print(LOG_ERROR, "shader: fragment shader [%d] failed to compile", shader);
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

/* Returns the ID of a shader program, linking two shaders */
ATRA_API
uint
atra_gl_shader_link(uint vs_id, uint fs_id)
{
    uint prog_id;
    int success, max_len, len;

    prog_id = glCreateProgram();

    glAttachShader(prog_id, vs_id);
    glAttachShader(prog_id, fs_id);

    glBindAttribLocation(prog_id, 0, ATRA_ATTRIB_NAME_POSITION);
    glBindAttribLocation(prog_id, 1, ATRA_ATTRIB_NAME_TEXCOORD);
    glBindAttribLocation(prog_id, 2, ATRA_ATTRIB_NAME_NORMAL);
    glBindAttribLocation(prog_id, 3, ATRA_ATTRIB_NAME_COLOR);
    glBindAttribLocation(prog_id, 4, ATRA_ATTRIB_NAME_TANGENT);
    glBindAttribLocation(prog_id, 5, ATRA_ATTRIB_NAME_TEXCOORD2);

    glLinkProgram(prog_id);
    glGetProgramiv(prog_id, GL_LINK_STATUS, &success);

    if (success == GL_FALSE) {
        max_len = 0;
        atra_log_print(LOG_ERROR, "shader: shader [%d] failed to link",
                prog_id);
        glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &max_len);

        if (max_len > 0) {
            len = 0;
            char *log = calloc(max_len, sizeof(char));
            glGetProgramInfoLog(prog_id, max_len, &len, log);
            atra_log_print(LOG_ERROR, "shader: shader [%d] linker error: " \
                    "%s", prog_id, log);
            free(log);
        }
        glDeleteProgram(prog_id);

        prog_id = 0;
    }
    glDetachShader(prog_id, vs_id);
    glDetachShader(prog_id, fs_id);
    glDeleteShader(vs_id);
    glDeleteShader(fs_id);

    return prog_id;
}

/* Deletes a shader program */
ATRA_API
void
atra_gl_shader_delete(uint prog_id)
{
    glDeleteProgram(prog_id);
}

/* Deletes the default shader program */
ATRA_API
void
atra_gl_shader_deletedefault(void)
{
    glUseProgram(0);

    glDeleteProgram(gl_state.def_shader_id);

    free(gl_state.def_shader_locs);
}


/* Enables a shader program */
ATRA_API
void
atra_gl_shader_enable(uint shader_id)
{
    glUseProgram(shader_id);
}

/* Disables the current shader program */
ATRA_API
void
atra_gl_shader_disable(void)
{
    glUseProgram(0);
}

/* Returns the location of a shader uniform */
ATRA_API
int
atra_gl_shader_getuniformloc(uint prog_id, const char *name)
{
    int loc;

    loc = glGetUniformLocation(prog_id, name);

    /* TODO: Add some checks, in case GL returns -1 */

    return loc;
}

/* Returns the location of a shader attribute */
ATRA_API
int
atra_gl_shader_getattribloc(uint prog_id, const char *name)
{
    int loc;

    loc = glGetAttribLocation(prog_id, name);
    
    /* TODO: Add some checks, in case GL returns -1 */

    return loc;
}

/* Sets the value of a shader uniform */
ATRA_API
void
atra_gl_shader_setuniform(int loc, const void *data, int type, int count)
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
    case ATRA_SHADER_UNIFORM_MAT4:    glUniformMatrix4fv(loc, 1, false, data); break;
    default:
        atra_log_print(LOG_WARN, "shader: unable to set uniform data: " \
                "invalid uniform data type");
    }
}

/* Sets the value of a shader attribute */
ATRA_API
void
atra_gl_shader_setattrib(int loc, const void *data, int type, int count)
{
    switch (type) {
    case ATRA_SHADER_ATTRIB_FLOAT:
        if (count == 1) glVertexAttrib1fv(loc, data); break;
    case ATRA_SHADER_ATTRIB_VEC2:
        if (count == 2) glVertexAttrib2fv(loc, data); break;
    case ATRA_SHADER_ATTRIB_VEC3:
        if (count == 3) glVertexAttrib3fv(loc, data); break;
    case ATRA_SHADER_ATTRIB_VEC4:
        if (count == 4) glVertexAttrib4fv(loc, data); break;
    default:
        atra_log_print(LOG_WARN, "gl: shader: invalid attribute data type");
    }
}


/* Set currently active internal shader (id and locations) */
ATRA_API
void
atra_gl_shader_setshader(uint prog_id, int *locs)
{
    if (gl_state.shader_id != prog_id) {
        atra_gl_batch_draw(current_batch);
        gl_state.shader_id = prog_id;
        gl_state.shader_locs = locs;
    }
}

/* Returns the shader ID of the default shader */
ATRA_API
uint
atra_gl_shader_getdefaultid(void)
{
    return gl_state.def_shader_id;
}

/* Returns the shader locations array of the default shader */
ATRA_API
int *
atra_gl_shader_getdefaultlocs(void)
{
    return gl_state.def_shader_locs;
}


/*
 *  OpenGL wrapper functions (General rendering)
 */

/* Enables color blending */
ATRA_API
void
atra_gl_enablecolorblend(void)
{
    glEnable(GL_BLEND);
}

/* Disables color blending */
ATRA_API
void
atra_gl_disablecolorblend(void)
{
    glDisable(GL_BLEND);
}

/* Enables depth testing for occlusion based on draw depth */
ATRA_API
void
atra_gl_enabledepthtest(void)
{
    glEnable(GL_DEPTH_TEST);
}

/* Disables depth testing */
ATRA_API
void
atra_gl_disabledepthtest(void)
{
    glDisable(GL_DEPTH_TEST);
}

/* Enables writing into the depth buffer */
ATRA_API
void
atra_gl_enabledepthmask(void)
{
    glDepthMask(GL_TRUE);
}

/* Disables writing into the depth buffer */
ATRA_API
void
atra_gl_disabledepthmask(void)
{
    glDepthMask(GL_FALSE);
}

/* Enables culling of polygons which face away from the camera */
ATRA_API
void
atra_gl_enablebackfaceculling(void)
{
    glEnable(GL_CULL_FACE);
}

/* Disables culling of polygons which face away from the camera */
ATRA_API
void
atra_gl_disablebackfaceculling(void)
{
    glDisable(GL_CULL_FACE);
}

/* Enables scissor test to prevent drawing outside of a specific screen area */
ATRA_API
void
atra_gl_enablescissortest(void)
{
    glEnable(GL_SCISSOR_TEST);
}

/* Disables scissor test */
ATRA_API
void
atra_gl_disablescissortest(void)
{
    glDisable(GL_SCISSOR_TEST);
}

/* Specifies area of the screen which is rendered to when scissor test is enabled */
ATRA_API
void
atra_gl_scissor(int x, int y, int w, int h)
{
    glScissor(x, y, w, h);
}

/* Enables wire mode */
ATRA_API
void
atra_gl_enablewiremode(void)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

/* Disables wire mode */
ATRA_API
void
atra_gl_disablewiremode(void)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

/* Sets the width that lines drawn using ATRA_LINES should be drawn with */
ATRA_API
void
atra_gl_setlinewidth(float width)
{
    glLineWidth(width);
}

/* Returns the current line width */
ATRA_API
float
atra_gl_getlinewidth(void)
{
    float width;

    glGetFloatv(GL_LINE_WIDTH, &width);

    return width;
}

/* Enables line anti-aliasing */
ATRA_API
void
atra_gl_enablesmoothlines(void)
{
    glEnable(GL_LINE_SMOOTH);
}

/* Disables line anti-aliasing */
ATRA_API
void
atra_gl_disablesmoothlines(void)
{
    glDisable(GL_LINE_SMOOTH);
}

/* Sets the color that the screen buffer should be cleared with */
ATRA_API
void
atra_gl_clearcolor(u8 r, u8 g, u8 b, u8 a)
{
    const float fr = (float)r / 255;
    const float fg = (float)g / 255;
    const float fb = (float)b / 255;
    const float fa = (float)a / 255;

    glClearColor(fr, fg, fb, fa);
}

/* Clears the screen's color and depth buffers */
ATRA_API
void
atra_gl_clearscreenbuffers(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/* Sets the blending mode */
ATRA_API
void
atra_gl_setblendmode(int mode)
{
    if (gl_state.blend_mode != mode) {
        atra_gl_batch_draw(current_batch);

        switch (mode) {
        case ATRA_BLEND_ALPHA:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            break;
        case ATRA_BLEND_ADDITIVE:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
            break;
        case ATRA_BLEND_MULTIPLIED:
            glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            break;
        case ATRA_BLEND_ADD_COLORS:
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
            break;
        case ATRA_BLEND_SUB_COLORS:
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_SUBTRACT);
            break;
        case ATRA_BLEND_CUSTOM:
            glBlendFunc(gl_state.blend_src_factor, gl_state.blend_dst_factor);
            glBlendEquation(gl_state.blend_equation);
            break;
        default:
            break;
        }
        gl_state.blend_mode = mode;
    }
}

/* Sets the blending mode factors and equation */
ATRA_API
void
atra_gl_setblendfactors(int src_factor, int dst_factor, int equation)
{
    gl_state.blend_src_factor = src_factor;
    gl_state.blend_dst_factor = dst_factor;
    gl_state.blend_equation = equation;
}
