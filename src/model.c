/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  model.h - Subsystem which provides a set of tools to easily manage models,
 *            meshes, materials and animations.  Abstracting and simplifying
 *            much of OpenGL's vertex data and rendering.
 *
 *  */

#include "atra.h"
#include "stb/cgltf.h"

ATRA_INLINE Model atra_gltf_load(const char *path);
ATRA_INLINE void atra_gltf_initbones(Model *model, const cgltf_data *data);
ATRA_INLINE void atra_gltf_loadmaterial(Model *model, const char *path, const cgltf_data *data);
ATRA_INLINE void atra_gltf_bindprimstobones(Model *model, const cgltf_data *data, int prim_index);
ATRA_INLINE ModelAnim * atra_gltf_loadmodelanims(const char *path, int *anim_count);
ATRA_INLINE void atra_gltf_loadmesh(cgltf_data *data, cgltf_mesh *mesh, Model *out_model,
        mat4 curr_transform, int *prim_index, const char *path);
ATRA_INLINE void atra_gltf_loadnode(cgltf_data *data, cgltf_node *node, Model *out_model,
        mat4 curr_transform, int *prim_index, const char *path);
ATRA_INLINE void atra_gltf_getprimcount(cgltf_node *node, int *out_count);


/*
 *  Model functions
 */

/* Returns a new model from model data from a file (GLTF) */
ATRA_API
Model
atra_model_loadfile(const char *path)
{

}

/* Returns a new model constructed from a mesh */
ATRA_API Model atra_model_loadmesh(Mesh *mesh);

/* Deletes a model and all contained bones and meshes */
ATRA_API void atra_model_delete(Model *model);

/* Deletes a model but leaves the meshes allocated */
ATRA_API void atra_model_meshlessdelete(Model *model);

/* Sets the material for a model's mesh */
ATRA_API void atra_model_setmeshmaterial(Model *model, int mesh_id, int mat_id);

/* Renders a model to the screen */
ATRA_API void atra_model_draw(Model *model, vec3 pos, float scale, Color tint);

/* Renders a model to the screen (extended parameters) */
ATRA_API void atra_model_drawext(Model *model, vec3 pos, vec3 rot_axis,
        vec3 rot_angle, float scale, Color tint);

/* Renders a model wireframe to the screen */
ATRA_API void atra_model_wiredraw(Model *model, vec3 pos, float scale, Color tint);

/* Renders a model wireframe to the screen (extended parameters) */
ATRA_API void atra_model_wiredrawext(Model *model, vec3 pos, vec3 rot_axis,
        vec3 rot_angle, float scale, Color tint);


/*
 *  Mesh functions
 */

static Mesh *curr_mesh;

/* Sets up a new mesh to be manually drawn to in an immediate-mode style.
 * Sets the new mesh as the currently active mesh
 * NOTE: Vertex position and texture UVs are enabled by default.
 *       The mesh expects all enabled buffers to have the same number of entries. 
 *       Failing to meet this requirement may cause unexpected behaviour */
ATRA_API
Mesh
atra_mesh_begin(int mode, int buffer_flags)
{
    Mesh mesh = {0};
    int prim_size;

    if (mode == ATRA_TRIANGLES) {
        prim_size = 3;
    } else if (mode == ATRA_QUADS) {
        prim_size = 4;
    } else {
        prim_size = 2;
    }
    const int size = ATRA_DEFAULT_MESH_SIZE * prim_size;

    mesh.vert_count = 0;
    mesh.tri_count = 0;
    
    mesh.im = calloc(1, sizeof(IMMeshInfo));
    mesh.im->limit = ATRA_DEFAULT_MESH_SIZE;
    mesh.im->mode = mode;
    mesh.im->prim_size = prim_size;

    /* Mesh and vertex buffers are enabled by default */
    mesh.verts = malloc(size * 3 * sizeof(float));
    mesh.uvs = malloc(size * 2 * sizeof(float));

    if (BIT_CHK(buffer_flags, ATRA_MESH_BUFFER_NORMALS)) {
        mesh.norms = malloc(size * 3 * sizeof(float));
    }

    if (BIT_CHK(buffer_flags, ATRA_MESH_BUFFER_COLORS)) {
        mesh.colors = malloc(size * 4 * sizeof(u8));
    }

    if (BIT_CHK(buffer_flags, ATRA_MESH_BUFFER_TANGENTS)) {
        mesh.tangents = malloc(size * 4 * sizeof(float));
    }

    if (BIT_CHK(buffer_flags, ATRA_MESH_BUFFER_TEXCOORD2)) {
        mesh.uvs2 = malloc(size * 2 * sizeof(float));
    }

    return mesh;
}

/* Finalises the currently active mesh and uploads its data to the GPU */
ATRA_API
void
atra_mesh_end(int dynamic)
{
    int i, j;

    curr_mesh->vert_count = curr_mesh->im->vert_count;
    curr_mesh->tri_count = curr_mesh->im->vert_count / 2;

    if (curr_mesh->im->mode == ATRA_QUADS) {
        /* Initialise indices */
        j = 0;

        curr_mesh->indices = malloc(curr_mesh->tri_count * 3 * sizeof(float));

        for (i = 0; i < curr_mesh->tri_count * 3; i += 6) {
            curr_mesh->indices[i + 0] = 4 * j + 0;
            curr_mesh->indices[i + 1] = 4 * j + 1;
            curr_mesh->indices[i + 2] = 4 * j + 2;
            curr_mesh->indices[i + 3] = 4 * j + 0;
            curr_mesh->indices[i + 4] = 4 * j + 2;
            curr_mesh->indices[i + 5] = 4 * j + 3;
            j++;
        }
    }
    atra_mesh_upload(curr_mesh, dynamic);
}

/* Doubles the size of the vertex buffers of the currently active mesh */
ATRA_INLINE
void
atra_mesh_growbuffers()
{
    int new_limit, new_size;
    
    new_limit = curr_mesh->im->limit * 2;
    new_size = new_limit * curr_mesh->im->prim_size;

    curr_mesh->verts = realloc(curr_mesh->verts, new_size * 3 * sizeof(float));
    curr_mesh->uvs = realloc(curr_mesh->uvs, new_size * 2 * sizeof(float));

    if (curr_mesh->norms) {
        curr_mesh->norms = realloc(curr_mesh->norms, new_size * 3 * sizeof(float));
    }

    if (curr_mesh->colors) {
        curr_mesh->colors = realloc(curr_mesh->colors, new_size * 4 * sizeof(u8));
    }

    if (curr_mesh->tangents) {
        curr_mesh->tangents = realloc(curr_mesh->tangents, new_size * 4 * sizeof(float));
    }

    if (curr_mesh->uvs2) {
        curr_mesh->uvs2 = realloc(curr_mesh->uvs2, new_size * 3 * sizeof(float));
    }
    curr_mesh->im->limit = new_limit;

}

/* Adds a vertex to the current mesh */
ATRA_API
void
atra_mesh_vertex3f(float x, float y, float z)
{
    const vec3 data = {x, y, z};

    atra_mesh_vertexv(data);
}

/* Adds a vertex to the current mesh (vec3 argument) */
ATRA_API
void
atra_mesh_vertexv(const vec3 pos)
{
    void *const dest = curr_mesh->verts + curr_mesh->im->vert_count * 3;
 
    if (curr_mesh->vert_count + 1 >=
            curr_mesh->im->limit * curr_mesh->im->prim_size) {
        atra_mesh_growbuffers();
    }
    curr_mesh->im->vert_count++;
    memcpy(dest, pos, sizeof(vec3));
}

/* Adds a texture UV coordinate to the current mesh */
ATRA_API
void
atra_mesh_texcoord2f(float x, float y)
{
    const vec2 data = {x, y};

    atra_mesh_texcoordv(data);
}

/* Adds a texture UV coordinate to the current mesh (vec2 argument) */
ATRA_API
void
atra_mesh_texcoordv(const vec2 uv)
{
    void *const dest = curr_mesh->uvs + curr_mesh->im->uv_count * 2;

    curr_mesh->im->uv_count++;
    memcpy(dest, uv, sizeof(vec2));
}

/* Adds a vertex normal to the current mesh */
ATRA_API
void
atra_mesh_normal3f(float x, float y, float z)
{
    const vec3 data = {x, y, z};

    atra_mesh_normalv(data);
}

/* Adds a vertex normal to the current mesh (vec3 argument) */
ATRA_API
void
atra_mesh_normalv(const vec3 normal)
{
    void *const dest = curr_mesh->norms + curr_mesh->im->norm_count * 3;

    curr_mesh->im->norm_count++;
    memcpy(dest, normal, sizeof(vec3));
}

/* Adds a vertex color to the current mesh */
ATRA_API
void
atra_mesh_color4ub(u8 r, u8 g, u8 b, u8 a)
{
    const u8 data[4] = {r, g, b, a};

    atra_mesh_colorv(data);
}

/* Adds a vertex color to the current mesh (4-array argument) */
ATRA_API
void
atra_mesh_colorv(const u8 *color)
{
    void *const dest = curr_mesh->colors + curr_mesh->im->color_count * 4;

    curr_mesh->im->color_count++;
    memcpy(dest, color, 4 * sizeof(u8));
}

/* Adds a vertex tangent to the current mesh */
ATRA_API
void
atra_mesh_tangent4f(float x, float y, float z, float w)
{
    const vec4 data = {x, y, z, w};

    atra_mesh_tangentv(data);
}

/* Adds a vertex tangent to the current mesh (vec4 argument) */
ATRA_API
void
atra_mesh_tangentv(const vec4 tangent)
{
    void *const dest = curr_mesh->tangents + curr_mesh->im->tangent_count * 4;

    curr_mesh->im->tangent_count++;
    memcpy(dest, tangent, sizeof(vec4));
}

/* Adds a secondary texture UV to the current mesh */
ATRA_API
void
atra_mesh_texcoord22f(float x, float y)
{
    const vec2 data = {x, y};

    atra_mesh_texcoord2v(data);
}

/* Adds a secondary texture UV to the current mesh (vec2 argument) */
ATRA_API
void
atra_mesh_texcoord2v(const vec2 uv2)
{
    void *const dest = curr_mesh->uvs2 + curr_mesh->im->uv2_count * 2;

    curr_mesh->im->uv2_count++;
    memcpy(dest, uv2, sizeof(vec2));
}

/* Deletes mesh data from both the CPU and GPU */
ATRA_API
void
atra_mesh_delete(Mesh *mesh)
{
    int i;

    atra_gl_vao_delete(mesh->vao_id);

    for (i = 0; i < ATRA_MAX_MESH_VERTEX_BUFFERS; i++) {
        atra_gl_vbo_delete(mesh->vbo_ids[i]);
    }
    free(mesh->vbo_ids);

    free(mesh->verts);
    free(mesh->uvs);
    free(mesh->norms);
    free(mesh->colors);
    free(mesh->tangents);
    free(mesh->uvs2);
    free(mesh->indices);

    free(mesh->anim_verts);
    free(mesh->anim_norms);
    free(mesh->bone_weights);
    free(mesh->bone_ids);
}

/* Sets the currently active mesh */
ATRA_API
void
atra_mesh_setactive(Mesh *mesh)
{
    curr_mesh = mesh;
}

/* Returns a pointer to the currently active mesh */
ATRA_API
Mesh *
atra_mesh_getactive(void)
{
    return curr_mesh;
}

/* Uploads mesh vertex data to the GPU */
ATRA_API
void
atra_mesh_upload(Mesh *mesh, int dynamic)
{
    void *verts, *norms;

    if (mesh->vao_id > 0) {
        atra_log_print(LOG_WARN, "model: Attempted to upload an already loaded mesh");
        return;
    }
    mesh->vbo_ids = calloc(ATRA_MAX_MESH_VERTEX_BUFFERS, sizeof(uint));
    mesh->vao_id = atra_gl_vao_create();
    atra_gl_vao_enable(mesh->vao_id);

    /* Enable vertex positions attribute */
    verts = (mesh->anim_verts != NULL) ? mesh->anim_verts : mesh->verts;
    mesh->vbo_ids[0] = atra_gl_vbo_create(verts,
            mesh->vert_count * 3 * sizeof(float), dynamic);
    atra_gl_vao_setattrib(0, 3, ATRA_FLOAT, 0, 0, NULL);
    atra_gl_vao_enableattrib(0);

    /* Enable texture UV coordinate attribute */
    mesh->vbo_ids[1] = atra_gl_vbo_create(mesh->uvs,
            mesh->vert_count * 2 * sizeof(float), dynamic);
    atra_gl_vao_setattrib(1, 2, ATRA_FLOAT, 0, 0, NULL);
    atra_gl_vao_enableattrib(1);

    if (mesh->norms != NULL) {
        /* Enable vertex normals attribute */
        norms = mesh->anim_norms != NULL ? mesh->anim_norms : mesh->anim_norms;
        mesh->vbo_ids[2] = atra_gl_vbo_create(norms,
                mesh->vert_count * 3 * sizeof(float), dynamic);
        atra_gl_vao_setattrib(2, 3, ATRA_FLOAT, 0, 0, NULL);
        atra_gl_vao_enableattrib(2);
    } else {
        /* Set normal attribute to default value */
        const vec3 value = {1.0f, 1.0f, 1.0f};
        atra_gl_vao_setattribdefault(2, value, ATRA_SHADER_ATTRIB_VEC3, 3);
        atra_gl_vao_disableattrib(2);
    }

    if (mesh->colors != NULL) {
        /* Enable vertex color attribute */
        mesh->vbo_ids[3] = atra_gl_vbo_create(mesh->colors,
                mesh->vert_count * 4 * sizeof(uchar), dynamic);
        atra_gl_vao_setattrib(3, 4, ATRA_UNSIGNED_BYTE, 1, 0, NULL);
        atra_gl_vao_enableattrib(3);
    } else {
        /* Set color attribute to default value */
        const vec4 value = {1.0f, 1.0f, 1.0f, 1.0f};
        atra_gl_vao_setattribdefault(3, value, ATRA_SHADER_ATTRIB_VEC4, 4);
        atra_gl_vao_disableattrib(3);
    }

    if (mesh->tangents != NULL) {
        /* Enable vertex tangents attribute */
        mesh->vbo_ids[4] = atra_gl_vbo_create(mesh->tangents,
                mesh->vert_count * 4 * sizeof(float), dynamic);
        atra_gl_vao_setattrib(4, 4, ATRA_FLOAT, 0, 0, NULL);
        atra_gl_vao_enableattrib(4);
    } else {
        /* Set tangents attribute to default value */
        const vec4 value = {0.0f, 0.0f, 0.0f, 0.0f};
        atra_gl_vao_setattribdefault(4, value, ATRA_SHADER_ATTRIB_VEC4, 4);
        atra_gl_vao_disableattrib(4);
    }

    if (mesh->uvs2 != NULL) {
        /* Enable vertex secondary texture UV coordinate attribute */
        mesh->vbo_ids[5] = atra_gl_vbo_create(mesh->uvs2,
                mesh->vert_count * 2 * sizeof(float), dynamic);
        atra_gl_vao_setattrib(5, 2, ATRA_FLOAT, 0, 0, NULL);
        atra_gl_vao_enableattrib(5);
    } else {
        /* Set secondary texture UV coordinate attribute to default value */
        const vec2 value = {0.0f, 0.0f};
        atra_gl_vao_setattribdefault(5, value, ATRA_SHADER_ATTRIB_VEC2, 2);
        atra_gl_vao_disableattrib(5);
    }

    if (mesh->indices != NULL) {
        mesh->vbo_ids[6] = atra_gl_ebo_create(mesh->indices,
                mesh->tri_count * 3 * sizeof(uint), dynamic);
    }

    if (mesh->vao_id > 0) {
        atra_log_print(LOG_INFO, "mesh: Mesh [%d] successfully uploaded to GPU", mesh->vao_id);
    } else {
        atra_log_print(LOG_ERROR, "mesh: Mesh [%d] failed during GPU upload", mesh->vao_id);
    }

    atra_gl_vao_disable();
}

/* Updates mesh vertex data in the GPU for a specific buffer index */
ATRA_API
void
atra_mesh_update(Mesh *mesh, int index, void *data, int size, int offset)
{
    atra_gl_vbo_update(mesh->vbo_ids[index], data, size, offset);
}

/* Renders a mesh with the specified material and transform */
ATRA_API
void
atra_mesh_draw(Mesh *mesh, Material *mat, mat4 transform)
{
    mat4 model, view, proj, projview, normal, mvp, gl_transform;
    int i;

    atra_shader_setactive(&mat->shader);

    if (mat->shader.locs[ATRA_SHADER_LOC_COLOR_DIFFUSE] != -1) {
        vec4 color = {
            (float)mat->maps[ATRA_TEXTURE_ALBEDO].color.r / 255.0f,
            (float)mat->maps[ATRA_TEXTURE_ALBEDO].color.g / 255.0f,
            (float)mat->maps[ATRA_TEXTURE_ALBEDO].color.b / 255.0f,
            (float)mat->maps[ATRA_TEXTURE_ALBEDO].color.a / 255.0f
        };
        atra_shader_setuniform(
                mat->shader.locs[ATRA_SHADER_LOC_COLOR_DIFFUSE],
                color,
                ATRA_SHADER_UNIFORM_VEC4, 1);
    }

    if (mat->shader.locs[ATRA_SHADER_LOC_COLOR_SPECULAR] != -1) {
        vec4 color = {
            (float)mat->maps[ATRA_TEXTURE_SPECULAR].color.r / 255.0f,
            (float)mat->maps[ATRA_TEXTURE_SPECULAR].color.g / 255.0f,
            (float)mat->maps[ATRA_TEXTURE_SPECULAR].color.b / 255.0f,
            (float)mat->maps[ATRA_TEXTURE_SPECULAR].color.a / 255.0f,
        };
        atra_shader_setuniform(
                mat->shader.locs[ATRA_SHADER_LOC_COLOR_SPECULAR],
                color,
                ATRA_SHADER_UNIFORM_VEC4, 1);
    }

    /* Set up camera matrix uniforms */
    atra_gl_matrix_getmodelview(view);
    atra_gl_matrix_getprojection(proj);

    if (mat->shader.locs[ATRA_SHADER_LOC_M_VIEW] != -1) {
        atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_VIEW],
                view, ATRA_SHADER_UNIFORM_MAT4, 1);
    }

    if (mat->shader.locs[ATRA_SHADER_LOC_M_PROJ] != -1) {
        atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_PROJ],
                proj, ATRA_SHADER_UNIFORM_MAT4, 1);
    }

    if (mat->shader.locs[ATRA_SHADER_LOC_M_MODEL] != -1) {
        atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_MODEL],
                transform, ATRA_SHADER_UNIFORM_MAT4, 1);
    }
    atra_gl_matrix_gettransform(gl_transform);
    glm_mat4_mul(transform, gl_transform, model);
    glm_mat4_mul(proj, view, projview);

    if (mat->shader.locs[ATRA_SHADER_LOC_M_NORMAL] != -1) {
        glm_mat4_inv(transform, normal);
        glm_mat4_transpose(normal);
        atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_NORMAL],
                normal, ATRA_SHADER_UNIFORM_MAT4, 1);
    }

    /* Bind all available texture maps */
    for (i = 0; i < ATRA_TEXTURE_TOTAL; i++) {
        if (mat->maps[i].tex.id > 0) {
            atra_gl_tex_setactiveunit(i);

            if (i == ATRA_TEXTURE_CUBEMAP ||
                    i == ATRA_TEXTURE_IRRADIANCE ||
                    i == ATRA_TEXTURE_PREFILTER) {
                atra_gl_tex_enablecubemap(mat->maps[i].tex.id);
            } else {
                atra_gl_tex_enable(mat->maps[i].tex.id);
            }

            atra_shader_setuniform(
                    mat->shader.locs[ATRA_SHADER_LOC_TEX_ALBEDO + i],
                    &i, ATRA_SHADER_UNIFORM_INT, 1);
        }

    }

    /* Bind VAO/VBOs */
    if (atra_gl_vao_enable(mesh->vao_id)) {
        /* Bind vertex position */
        atra_gl_vbo_enable(mesh->vbo_ids[0]);
        atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_POSITION],
                3, ATRA_FLOAT, 0, 0, NULL);
        atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_POSITION]);

        /* Bind vertex texture UV coordinates */
        atra_gl_vbo_enable(mesh->vbo_ids[1]);
        atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_TEXCOORD],
                2, ATRA_FLOAT, 0, 0, NULL);
        atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_TEXCOORD]);

        /* Bind vertex normals */
        if (mat->shader.locs[ATRA_SHADER_LOC_V_NORMAL] != -1) {
            atra_gl_vbo_enable(mesh->vbo_ids[2]);
            atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_NORMAL],
                    3, ATRA_FLOAT, 0, 0, NULL);
            atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_NORMAL]);
        }

        /* Bind vertex colors */
        if (mat->shader.locs[ATRA_SHADER_LOC_V_COLOR] != -1) {
            if (mesh->vbo_ids[3] != 0) {
                atra_gl_vbo_enable(mesh->vbo_ids[3]);
                atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_COLOR],
                        4, ATRA_UNSIGNED_BYTE, 1, 0, NULL);
                atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_COLOR]);
            } else {
                const vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
                atra_gl_vao_setattribdefault(mat->shader.locs[ATRA_SHADER_LOC_V_COLOR],
                        color, ATRA_SHADER_ATTRIB_VEC2, 4);
                atra_gl_vao_disableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_COLOR]);
            }
        }

        /* Bind vertex tangents */
        if (mat->shader.locs[ATRA_SHADER_LOC_V_TANGENT] != -1) {
            atra_gl_vbo_enable(mesh->vbo_ids[4]);
            atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_TANGENT],
                    4, ATRA_FLOAT, 0, 0, NULL);
            atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_TANGENT]);
        }

        /* Bind secondary vertex texture UV coordinates */
        if (mat->shader.locs[ATRA_SHADER_LOC_V_TEXCOORD2] != -1) {
            atra_gl_vbo_enable(mesh->vbo_ids[5]);
            atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_TEXCOORD2],
                    2, ATRA_FLOAT, 0, 0, NULL);
            atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_TEXCOORD2]);
        }

        if (mesh->indices != NULL) {
            atra_gl_ebo_enable(mesh->vbo_ids[6]);
        }
    }
    glm_mat4_mul(projview, model, mvp);
    atra_gl_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_MVP],
            mvp, ATRA_SHADER_UNIFORM_MAT4, 1);
    
    /* Draw the mesh */
    if (mesh->indices == NULL) {
        if (mesh->im) {
            if (mesh->im->mode == ATRA_LINES) {
                /* Draw lines (only possible in immediate-style rendering) */
                glDrawArrays(ATRA_LINES, 0, mesh->vert_count);
            }
        } else {
            /* Default rendering mode */
            atra_gl_vao_draw(0, mesh->vert_count);
        }
    } else {
        atra_gl_vao_drawelements(0, mesh->tri_count * 3, NULL);
    }

    /* Post-draw cleanup */
    for (i = 0; i < ATRA_TEXTURE_TOTAL; i++) {
        atra_gl_tex_setactiveunit(i);

        if (i == ATRA_TEXTURE_IRRADIANCE ||
                i == ATRA_TEXTURE_PREFILTER ||
                i == ATRA_TEXTURE_CUBEMAP) {
            atra_gl_tex_disablecubemap();
        } else {
            atra_gl_tex_disable();
        }
    }
    atra_gl_vao_disable();
    atra_gl_vbo_disable();
    atra_gl_ebo_disable();

    atra_gl_shader_disable();

    /* Restore internal matrices */
    atra_gl_matrix_setmodelview(view);
    atra_gl_matrix_setprojection(proj);
}

/* Instance renders a mesh with a single material and different transforms */
ATRA_API
void
atra_mesh_drawinstanced(Mesh *mesh, Material *mat, mat4 *transforms, int count)
{
    mat4 model, view, proj, modelview, normal, mvp, gl_transform;
    uint instance_vboid = 0;
    int i;

    atra_shader_setactive(&mat->shader);

    if (mat->shader.locs[ATRA_SHADER_LOC_COLOR_DIFFUSE] != -1) {
        vec4 color = {
            (float)mat->maps[ATRA_TEXTURE_ALBEDO].color.r / 255.0f,
            (float)mat->maps[ATRA_TEXTURE_ALBEDO].color.g / 255.0f,
            (float)mat->maps[ATRA_TEXTURE_ALBEDO].color.b / 255.0f,
            (float)mat->maps[ATRA_TEXTURE_ALBEDO].color.a / 255.0f
        };
        atra_shader_setuniform(
                mat->shader.locs[ATRA_SHADER_LOC_COLOR_DIFFUSE],
                color,
                ATRA_SHADER_UNIFORM_VEC4, 1);
    }

    if (mat->shader.locs[ATRA_SHADER_LOC_COLOR_SPECULAR] != -1) {
        vec4 color = {
            (float)mat->maps[ATRA_TEXTURE_SPECULAR].color.r / 255.0f,
            (float)mat->maps[ATRA_TEXTURE_SPECULAR].color.g / 255.0f,
            (float)mat->maps[ATRA_TEXTURE_SPECULAR].color.b / 255.0f,
            (float)mat->maps[ATRA_TEXTURE_SPECULAR].color.a / 255.0f,
        };
        atra_shader_setuniform(
                mat->shader.locs[ATRA_SHADER_LOC_COLOR_SPECULAR],
                color,
                ATRA_SHADER_UNIFORM_VEC4, 1);
    }

    /* Set up camera matrix uniforms */
    atra_gl_matrix_getmodelview(view);
    atra_gl_matrix_getprojection(proj);

    if (mat->shader.locs[ATRA_SHADER_LOC_M_VIEW] != -1) {
        atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_VIEW],
                view, ATRA_SHADER_UNIFORM_MAT4, 1);
    }

    if (mat->shader.locs[ATRA_SHADER_LOC_M_PROJ] != -1) {
        atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_PROJ],
                proj, ATRA_SHADER_UNIFORM_MAT4, 1);
    }

    atra_gl_vao_enable(mesh->vao_id);
    instance_vboid = atra_gl_vbo_create(transforms, count * sizeof(float), 0);

    for (i = 0; i < 4; i++) {
        atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_M_MODEL] + i);
        atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_M_MODEL] + i,
                4, ATRA_FLOAT, 0, sizeof(mat4), (void *)(i * sizeof(vec4)));
        atra_gl_vao_setattribdivisor(mat->shader.locs[ATRA_SHADER_LOC_M_MODEL] + i, 1);
    }
    atra_gl_vbo_disable();
    atra_gl_vao_disable();

    atra_gl_matrix_gettransform(gl_transform);
    glm_mat4_mul(gl_transform, view, modelview);

    if (mat->shader.locs[ATRA_SHADER_LOC_M_NORMAL] != -1) {
        glm_mat4_inv(model, normal);
        glm_mat4_transpose(normal);
        atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_NORMAL],
                normal, ATRA_SHADER_UNIFORM_MAT4, 1);
    }

    /* Bind all available texture maps */
    for (i = 0; i < ATRA_TEXTURE_TOTAL; i++) {
        if (mat->maps[i].tex.id > 0) {
            atra_gl_tex_setactiveunit(i);

            if (i == ATRA_TEXTURE_CUBEMAP ||
                    i == ATRA_TEXTURE_IRRADIANCE ||
                    i == ATRA_TEXTURE_PREFILTER) {
                atra_gl_tex_enablecubemap(mat->maps[i].tex.id);
            } else {
                atra_gl_tex_enable(mat->maps[i].tex.id);
            }

            atra_shader_setuniform(
                    mat->shader.locs[ATRA_SHADER_LOC_TEX_ALBEDO + i],
                    &i, ATRA_SHADER_UNIFORM_INT, 1);
        }

    }

    /* Bind VAO/VBOs */
    if (atra_gl_vao_enable(mesh->vao_id)) {
        /* Bind vertex position */
        atra_gl_vbo_enable(mesh->vbo_ids[0]);
        atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_POSITION],
                3, ATRA_FLOAT, 0, 0, NULL);
        atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_POSITION]);

        /* Bind vertex texture UV coordinates */
        atra_gl_vbo_enable(mesh->vbo_ids[1]);
        atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_TEXCOORD],
                2, ATRA_FLOAT, 0, 0, NULL);
        atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_TEXCOORD]);

        /* Bind vertex normals */
        if (mat->shader.locs[ATRA_SHADER_LOC_V_NORMAL] != -1) {
            atra_gl_vbo_enable(mesh->vbo_ids[2]);
            atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_NORMAL],
                    3, ATRA_FLOAT, 0, 0, NULL);
            atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_NORMAL]);
        }

        /* Bind vertex colors */
        if (mat->shader.locs[ATRA_SHADER_LOC_V_COLOR] != -1) {
            if (mesh->vbo_ids[3] != 0) {
                atra_gl_vbo_enable(mesh->vbo_ids[3]);
                atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_COLOR],
                        4, ATRA_UNSIGNED_BYTE, 1, 0, NULL);
                atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_COLOR]);
            } else {
                const vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
                atra_gl_vao_setattribdefault(mat->shader.locs[ATRA_SHADER_LOC_V_COLOR],
                        color, ATRA_SHADER_ATTRIB_VEC2, 4);
                atra_gl_vao_disableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_COLOR]);
            }
        }

        /* Bind vertex tangents */
        if (mat->shader.locs[ATRA_SHADER_LOC_V_TANGENT] != -1) {
            atra_gl_vbo_enable(mesh->vbo_ids[4]);
            atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_TANGENT],
                    4, ATRA_FLOAT, 0, 0, NULL);
            atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_TANGENT]);
        }

        /* Bind secondary vertex texture UV coordinates */
        if (mat->shader.locs[ATRA_SHADER_LOC_V_TEXCOORD2] != -1) {
            atra_gl_vbo_enable(mesh->vbo_ids[5]);
            atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_TEXCOORD2],
                    2, ATRA_FLOAT, 0, 0, NULL);
            atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_TEXCOORD2]);
        }

        if (mesh->indices != NULL) {
            atra_gl_ebo_enable(mesh->vbo_ids[6]);
        }
    }
    glm_mat4_mul(modelview, proj, mvp);
    atra_gl_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_MVP],
            mvp, ATRA_SHADER_UNIFORM_MAT4, 1);
    
    /* Draw the mesh */
    if (mesh->indices == NULL) {
        atra_gl_vao_drawinstanced(0, mesh->vert_count, count);
    } else {
        atra_gl_vao_drawelementsinstanced(0, mesh->tri_count * 3, NULL, count);
    }

    /* Post-draw cleanup */
    for (i = 0; i < ATRA_TEXTURE_TOTAL; i++) {
        atra_gl_tex_setactiveunit(i);

        if (i == ATRA_TEXTURE_IRRADIANCE ||
                i == ATRA_TEXTURE_PREFILTER ||
                i == ATRA_TEXTURE_CUBEMAP) {
            atra_gl_tex_disablecubemap();
        } else {
            atra_gl_tex_disable();
        }
    }
    atra_gl_vao_disable();
    atra_gl_vbo_disable();
    atra_gl_ebo_disable();

    atra_gl_shader_disable();

    /* Restore internal matrices */
    atra_gl_matrix_setmodelview(view);
    atra_gl_matrix_setprojection(proj);

}

/* Exports mesh data to a file.  Returns 1 on success, otherwise 0 */
ATRA_API void atra_mesh_export(Mesh *mesh, const char *path);

/* Returns a new minimum bounding box for a mesh */
ATRA_API AABB atra_mesh_createaabb(Mesh *mesh);

/* Calculates mesh tangents */
ATRA_API void atra_mesh_calctangents(Mesh *mesh);

/* Calculates mesh bitangents */
ATRA_API void atra_mesh_calcbitangents(Mesh *mesh);

/* Computes a mesh from a heightmap image */
ATRA_API Mesh atra_mesh_loadheightmap(Image *heightmap, vec3 size);

/*
 *  Material functions
 */

/* Loads materials from a model file (GLTF).
 * Returns an array of materials and an amount of materials */
ATRA_API Material * atra_material_loadfile(const char *path, int *mat_count);

/* Loads the default material (supports diffuse, specular and normal maps) */
ATRA_API Material atra_material_loaddefault(void);

/* Deletes material data from GPU */
ATRA_API void atra_material_delete(Material *mat);

/* Sets the texture for a material type */
ATRA_API void atra_material_settexture(Material *mat, int tex_type, Texture *tex);


/*
 *  Model animation functions
 */

/* Loads an array of model animations from a file (GLTF) */
ATRA_API ModelAnim * atra_anim_loadfile(const char *file, int *anim_count);

/* Updates the pose of a model to a specified frame of an animation */
ATRA_API void atra_anim_updatemodel(ModelAnim *anim, Model *model, int frame);

/* Deletes model animation data */
ATRA_API void atra_anim_delete(ModelAnim *anim);

/* Deletes model animation array data */
ATRA_API void atra_anim_deletearray(ModelAnim **anims, int count);

/* Returns 1 if the model animation skeleton is a match */
ATRA_API void atra_anim_modelskeletonmatch(Model *model, ModelAnim *anim);


/*
 *  Immediate-style drawing functions
 */

/* Renders a bounding box to the screen as a wireframe */
ATRA_API void atra_draw_aabb(AABB box, Color color);

/* Draws a texture as a billboard */
ATRA_API void atra_draw_billboardtex(Camera *cam, Texture *tex, vec3 centre,
        float size, Color tint);

/* Draws a subtexture as a billboard */
ATRA_API void atra_draw_billboardsubtex(Camera *cam, Texture *tex, IQuad rect,
        vec3 centre, float size, Color tint);


/*
 *  Static function definitions
 */

ATRA_INLINE
Model
atra_load_gltf(const char *path)
{
    Model model = {0};
    size_t data_size = 0;
    mat4 static_transform;
    i8 *file_data;
    int prim_count, prim_index;
    uint i;

    file_data = atra_util_loadfile(path, &data_size);

    if (file_data == NULL) {
        return model;
    }
    cgltf_options options = {0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse(&options, file_data, data_size, &data);

    if (result == cgltf_result_success) {
        result = cgltf_load_buffers(&options, data, path);

        if (result != cgltf_result_success) {
            atra_log_print(LOG_WARN, "model: failed to load glTF mesh/material buffer");
        }

        if (data->scenes_count > 1) {
            atra_log_print(LOG_WARN, "model: multiple scenes in glTF file, " \
                    "only the first scene will be loaded");
        }

        for (i = 0; i < data->scene->nodes_count; i++) {
            atra_gltf_getprimcount(data->scene->nodes[i], &prim_count);
        }
        model.mesh_count = prim_count;
        model.meshes = calloc(model.mesh_count, sizeof(Mesh));
        model.material_count = (int)data->materials_count + 1;
        model.materials = malloc(model.material_count * sizeof(Material));
        model.mesh_materials = malloc(model.mesh_count * sizeof(int));
        model.bone_count = (int)data->nodes_count;
        model.bones = calloc(model.bone_count, sizeof(BoneInfo));
        model.bind_pose = calloc(model.bone_count, sizeof(Transform));

        atra_gltf_initbones(&model, data);
        atra_gltf_loadmaterial(&model, path, data);

        prim_index = 0;

        for (i = 0; i < data->scene->nodes_count; i++) {
            glm_mat4_identity(static_transform);
            atra_gltf_loadnode(data, data->scene->nodes[i], &model,
                    static_transform, &prim_index, path);
        }
        cgltf_free(data);
    } else {
        atra_log_print(LOG_ERROR, "model: failed to load glTF file: %s", path);
    }
    free(file_data);

    return model;
}

ATRA_INLINE
void
atra_gltf_initbones(Model *restrict model, const cgltf_data *restrict data)
{
    Transform *curr_tf, *parent_tf;
    BoneInfo *curr_bone;
    int *completed_bones, completed_count, root;
    uint i;

    for (i = 0; i < data->nodes_count; i++) {
        strcpy(model->bones[i].name, data->nodes[i].name == 0 ? "ANIMJOINT" : data->nodes[i].name);
        model->bones[i].parent = (data->nodes[i].parent != NULL) ? (int)(data->nodes[i].parent - data->nodes) : -1;
    }

    for (i = 0; i < data->nodes_count; i++) {
        if (data->nodes[i].has_translation) {
            memcpy(model->bind_pose[i].translation, data->nodes[i].translation, 3 * sizeof(float));
        } else {
            glm_vec3_zero(model->bind_pose[i].translation);
        }

        if (data->nodes[i].has_rotation) {
            memcpy(model->bind_pose[i].rotation, data->nodes[i].rotation, 4 * sizeof(float));
        } else {
            glm_quat_identity(model->bind_pose[i].rotation);
        }

        glm_quat_normalize(model->bind_pose[i].rotation);

        if (data->nodes[i].has_scale) {
            memcpy(model->bind_pose[i].scale, data->nodes[i].scale, 3 * sizeof(float));
        } else {
            glm_vec3_one(model->bind_pose[i].scale);
        }
    }
    completed_bones = calloc(model->bone_count, sizeof(int));
    completed_count = 0;

    while (completed_count < model->bone_count) {
        for (i = 0; i < (uint)model->bone_count; i++) {
            if (completed_bones[i]) {
                continue;
            }

            if (model->bones[i].parent < 0) {
                completed_bones[i] = 1;
                completed_count++;
                continue;
            }

            if (!completed_bones[model->bones[i].parent]) {
                continue;
            }
            curr_tf = &model->bind_pose[i];
            curr_bone = &model->bones[i];
            root = curr_bone->parent;
            root = (root < model->bone_count) ? root : 0;
            parent_tf = &model->bind_pose[root];

            glm_quat_mul(parent_tf->rotation, curr_tf->rotation, curr_tf->rotation);
            glm_quat_rotatev(parent_tf->rotation, curr_tf->translation, curr_tf->translation);
            glm_vec3_add(parent_tf->translation, curr_tf->translation, curr_tf->translation);
            glm_vec3_mul(parent_tf->scale, curr_tf->scale, curr_tf->scale);
            completed_bones[i] = 1;
            completed_count++;
        }
    }
    free(completed_bones);
}

ATRA_INLINE
void
atra_gltf_loadmaterial(Model *model, const char *path, const cgltf_data *data)
{

}

ATRA_INLINE
void
atra_gltf_bindprimstobones(Model *model, const cgltf_data *data, int prim_index)
{

}

ATRA_INLINE
ModelAnim *
atra_gltf_loadmodelanims(const char *path, int *anim_count)
{

}

ATRA_INLINE
void
atra_gltf_loadmesh(cgltf_data *data, cgltf_mesh *mesh, Model *out_model,
        mat4 curr_transform, int *prim_index, const char *path)
{

}

ATRA_INLINE
void
atra_gltf_loadnode(cgltf_data *data, cgltf_node *node, Model *out_model,
        mat4 curr_transform, int *prim_index, const char *path)
{

}

ATRA_INLINE
void
atra_gltf_getprimcount(cgltf_node *node, int *out_count)
{

}
