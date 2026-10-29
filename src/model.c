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
#include "stb/stb_image.h"
#include "stb/tinyobj_loader_c.h"

#include <limits.h>
#include <math.h>

static const uchar base64_table[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 62, 0, 0, 0, 63, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 0, 0, 0, 0, 0, 0, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51
};

ATRA_INLINE Model atra_gltf_load(const char *path);
ATRA_INLINE void atra_gltf_initbones(Model *model, const cgltf_data *data);
ATRA_INLINE Image atra_gltf_loadimage(cgltf_image *image, const char *tex_path, Color tint);
ATRA_INLINE void atra_gltf_loadmaterial(Model *model, const char *path, const cgltf_data *data);
ATRA_INLINE void atra_gltf_bindprimstobones(Model *model, const cgltf_data *data, int prim_index);
ATRA_INLINE ModelAnim * atra_gltf_loadmodelanims(const char *path, int *anim_count);
ATRA_INLINE ModelAnimRaw * atra_gltf_loadmodelanimsraw(const char *path, int *anim_count);
ATRA_INLINE void atra_gltf_loadmesh(cgltf_data *data, cgltf_mesh *mesh, Model *out_model,
        mat4 curr_transform, int *prim_index, const char *path);
static void atra_gltf_loadnode(cgltf_data *data, cgltf_node *node, Model *out_model,
        mat4 curr_transform, int *prim_index, const char *path);
static void atra_gltf_getprimcount(cgltf_node *node, int *out_count);
ATRA_INLINE int atra_gltf_readvalue(cgltf_accessor *acc, uint index, void *variable);
ATRA_INLINE void * atra_gltf_readvalueas(cgltf_accessor *acc, cgltf_component_type type, int adjust_on_down_casting);
ATRA_INLINE void atra_gltf_getnodetransform(cgltf_node *node, mat4 current, vec4 *dest);

ATRA_INLINE void atra_mat4_make_transrot(vec3 trans, versor rot, mat4 dest);
ATRA_INLINE void atra_log_printmatrix(mat4 m);

ATRA_INLINE int atra_base64_getsize(char *in);
ATRA_INLINE uchar * atra_base64_decode(char *in, int *size);

ATRA_INLINE void atra_mesh_growbuffers(void);

/*
 *  Model functions
 */

/* Returns a new model from model data from a file (GLTF) */
ATRA_API
Model
atra_model_loadfile(const char *path, int auto_upload)
{
    Model model = {0};
    int i;

    if (atra_util_filehasext(path, ".gltf;.glb")) {
        model = atra_gltf_load(path);
    } else {
        atra_log_print(LOG_WARN, "model: Unable to load model file %s: " \
                "Unrecognised model file format.", path);
    }

    glm_mat4_identity(model.transform);

    if (model.mesh_count == 0) {
        model.mesh_count = 1;
        model.meshes = calloc(model.mesh_count, sizeof(Mesh));

        /* TODO: Add default, hard-coded mesh for erroneous loads */

        atra_log_print(LOG_WARN, "model: Failed to load mesh data from model file: %s", path);
    } else {
        if (auto_upload) {
            for (i = 0; i < model.mesh_count; i++) {
                atra_mesh_upload(model.meshes, 0);
            }
        }
    }

    if (model.material_count == 0) {
        atra_log_print(LOG_WARN, "model: Failed to load material data from model file: %s", path);

        /* TODO: Load default material */
        model.material_count = 1;
        model.materials = calloc(model.material_count, sizeof(Material));
        model.materials[0] = atra_material_loaddefault();

        if (model.mesh_materials == NULL) {
            model.mesh_materials = calloc(model.mesh_count, sizeof(int));
        }
    }

    return model;
}

/* Returns a new model constructed from a mesh */
ATRA_API
Model
atra_model_loadmesh(Mesh *mesh)
{
    Model model = {0};

    glm_mat4_identity(model.transform);

    model.mesh_count = 1;
    model.meshes = calloc(model.mesh_count, sizeof(Mesh));
    model.meshes[0] = *mesh;

    model.material_count = 1;
    model.materials = calloc(model.material_count, sizeof(Material));
    model.materials[0] = atra_material_loaddefault();

    model.mesh_materials = calloc(model.mesh_count, sizeof(int));
    model.mesh_materials[0] = 0;

    return model;
}

/* Deletes a model and all contained bones and meshes */
ATRA_API
void
atra_model_delete(Model *model)
{
    int i;

    for (i = 0; i < model->mesh_count; i++) {
        atra_mesh_delete(model->meshes + i);
    }
    free(model->meshes);

    for (i = 0; i < model->material_count; i++) {
        atra_material_delete(model->materials + i);
    }
    free(model->materials);
    free(model->mesh_materials);

    free(model->bones);
    free(model->bind_pose);
    free(model->invbind_mats);
    free(model->world_mats);
    free(model->bone_mats);
}

/* Deletes a model but leaves the meshes allocated */
ATRA_API
void
atra_model_meshlessdelete(Model *model)
{
    free(model->meshes);
    free(model->materials);
    free(model->mesh_materials);

    free(model->bones);
    free(model->bind_pose);
    free(model->invbind_mats);
    free(model->world_mats);
    free(model->bone_mats);
}

/* Sets the material for a model's mesh */
ATRA_API
void
atra_model_setmeshmaterial(Model *model, int mesh_id, int mat_id)
{
    if (mesh_id >= model->mesh_count) {

    } else if (mat_id >= model->material_count) {

    } else {
        model->mesh_materials[mesh_id] = mat_id;
    }
}

/* Renders a model to the screen */
ATRA_API
void
atra_model_draw(Model *model, vec3 pos, float scale, Color tint)
{
    vec3 scale_vec = {scale, scale, scale};
    vec3 rot_axis = {0.0f, 1.0f, 0.0f};

    atra_model_drawext(model, pos, rot_axis, 0.0f, scale_vec, tint);
}

/* Renders a model to the screen (extended parameters) */
ATRA_API
void
atra_model_drawext(Model *model, vec3 pos, vec3 rot_axis,
        float rot_angle, vec3 scale, Color tint)
{
    mat4 trans_mat, rot_mat, scale_mat;
    mat4 transform;
    int i;

    glm_translate_make(trans_mat, pos);
    glm_rotate_make(rot_mat, rot_angle, rot_axis);
    glm_scale_make(scale_mat, scale);

    glm_mat4_mul(trans_mat, rot_mat, transform);
    glm_mat4_mul(transform, scale_mat, transform);

    glm_mat4_mul(transform, model->transform, model->transform);

    for (i = 0; i < model->mesh_count; i++) {
        Material *mat = &model->materials[model->mesh_materials[i]];
        Color color = mat->maps[ATRA_TEXTURE_ALBEDO].color;
        Color color_tint;

        color_tint.r = (((color.r / 255.0f) * (tint.r / 255.0f)) * 255.0f);
        color_tint.g = (((color.g / 255.0f) * (tint.g / 255.0f)) * 255.0f);
        color_tint.b = (((color.b / 255.0f) * (tint.b / 255.0f)) * 255.0f);
        color_tint.a = (((color.a / 255.0f) * (tint.a / 255.0f)) * 255.0f);
    
        atra_shader_setactive(&mat->shader);

        /* TODO: Add animation bone matrix uniform push here */
        if (mat->shader.locs[ATRA_SHADER_LOC_M_BONES] != -1) {
            atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_BONES],
                    model->bone_mats, ATRA_SHADER_UNIFORM_MAT4, model->bone_count);
        }

        mat->maps[ATRA_TEXTURE_ALBEDO].color = color_tint;
        atra_mesh_draw(model->meshes + i, model->materials + model->mesh_materials[i], model->transform);
        mat->maps[ATRA_TEXTURE_ALBEDO].color = color;
    }
}

/* Instance renders a model to the screen */
ATRA_API
void
atra_model_draw_instanced(Model *model, uint inst_count, vec3 pos, float scale,
        Color tint, mat4 *transforms)
{
    vec3 scale_vec = {scale, scale, scale};
    vec3 rot_axis = {0.0f, 1.0f, 0.0f};

    atra_model_drawext_instanced(model, inst_count, pos, rot_axis, 0.0f,
            scale_vec, tint, transforms);
}

/* Instance renders a model to the screen (extended parameters) */
ATRA_API
void
atra_model_drawext_instanced(Model *model, uint inst_count, vec3 pos,
        vec3 rot_axis, float rot_angle, vec3 scale, Color tint, mat4 *transforms)
{
    mat4 trans_mat, rot_mat, scale_mat;
    mat4 transform;
    int i;

    glm_translate_make(trans_mat, pos);
    glm_rotate_make(rot_mat, rot_angle, rot_axis);
    glm_scale_make(scale_mat, scale);

    glm_mat4_mul(trans_mat, rot_mat, transform);
    glm_mat4_mul(transform, scale_mat, transform);

    glm_mat4_mul(transform, model->transform, model->transform);

    /* TODO: Either integrate or remove `transform` into instance renders */

    for (i = 0; i < model->mesh_count; i++) {
        Color color = model->materials[model->mesh_materials[i]].maps[ATRA_TEXTURE_ALBEDO].color;
        Color color_tint;

        color_tint.r = (((color.r / 255.0f) * (tint.r / 255.0f)) * 255.0f);
        color_tint.g = (((color.g / 255.0f) * (tint.g / 255.0f)) * 255.0f);
        color_tint.b = (((color.b / 255.0f) * (tint.b / 255.0f)) * 255.0f);
        color_tint.a = (((color.a / 255.0f) * (tint.a / 255.0f)) * 255.0f);

        model->materials[model->mesh_materials[i]].maps[ATRA_TEXTURE_ALBEDO].color = color_tint;
        atra_mesh_drawinstanced(model->meshes + i, model->materials + model->mesh_materials[i], transforms,
                inst_count, model->bone_mats, model->bone_count);
        model->materials[model->mesh_materials[i]].maps[ATRA_TEXTURE_ALBEDO].color = color;
    }
}


/* Renders a model wireframe to the screen */
ATRA_API
void
atra_model_wiredraw(Model *model, vec3 pos, float scale, Color tint)
{
    atra_gl_enablewiremode();

    atra_model_draw(model, pos, scale, tint);

    atra_gl_disablewiremode();
}

/* Renders a model wireframe to the screen (extended parameters) */
ATRA_API
void
atra_model_wiredrawext(Model *model, vec3 pos, vec3 rot_axis,
        float rot_angle, vec3 scale, Color tint)
{
    atra_gl_enablewiremode();

    atra_model_drawext(model, pos, rot_axis, rot_angle, scale, tint);

    atra_gl_disablewiremode();
}


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

    /* Initialise indices */
    if (curr_mesh->im->mode == ATRA_QUADS) {
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
atra_mesh_growbuffers(void)
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
    if (curr_mesh->im->vert_count + 1 >=
            curr_mesh->im->limit * curr_mesh->im->prim_size) {
        atra_mesh_growbuffers();
    }
    void *const dest = curr_mesh->verts + curr_mesh->im->vert_count * 3;
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
    free(mesh->im);
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
        norms = mesh->anim_norms != NULL ? mesh->anim_norms : mesh->norms;
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
                mesh->vert_count * 3 * sizeof(float), dynamic);
        atra_gl_vao_setattrib(4, 3, ATRA_FLOAT, 0, 0, NULL);
        atra_gl_vao_enableattrib(4);
    } else {
        /* Set tangents attribute to default value */
        const vec4 value = {0.0f, 0.0f, 0.0f};
        atra_gl_vao_setattribdefault(4, value, ATRA_SHADER_ATTRIB_VEC3, 4);
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

    if (mesh->bone_ids != NULL && mesh->bone_weights != NULL) {
        /* Enable bone VBOs */
        mesh->vbo_ids[6] = atra_gl_vbo_create(
                mesh->bone_ids, mesh->vert_count * 4 * sizeof(float), 1);
        atra_gl_vao_setattrib(6, 4, ATRA_FLOAT, 0, 0, NULL);
        atra_gl_vao_enableattrib(6);
        mesh->vbo_ids[7] = atra_gl_vbo_create(
                mesh->bone_weights, mesh->vert_count * 4 * sizeof(float), 1);
        atra_gl_vao_setattrib(7, 4, ATRA_FLOAT, 0, 0, NULL);
        atra_gl_vao_enableattrib(7);
    }

    if (mesh->indices != NULL) {
        mesh->vbo_ids[8] = atra_gl_ebo_create(mesh->indices,
                mesh->tri_count * 3 * sizeof(uint), dynamic);
    }

    if (mesh->vao_id > 0) {
        //atra_log_print(LOG_INFO, "mesh: Mesh [%d] successfully uploaded to GPU", mesh->vao_id);
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
    mat4 model, view, proj, modelview, normal, mvp, gl_transform;
    int i;

    if (mesh->vert_count == 0) {
        return;
    }

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
            (float)mat->maps[ATRA_TEXTURE_SPECULAR].color.a / 255.0f
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
    glm_mat4_mul(view, model, modelview);

    if (mat->shader.locs[ATRA_SHADER_LOC_M_NORMAL] != -1) {
        glm_mat4_inv(transform, normal);
        glm_mat4_transpose(normal);
        atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_NORMAL],
                normal, ATRA_SHADER_UNIFORM_MAT4, 1);
    }

	if (mat->shader.locs[ATRA_SHADER_LOC_V_SCREENRES] != -1) {
		ivec2 screen_res = {
			atra_window_getwidth(),
			atra_window_getheight()
		};
		atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_V_SCREENRES],
				screen_res, ATRA_SHADER_UNIFORM_IVEC2, 1);
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
    if (atra_gl_vao_enable(mesh->vao_id) != 0) {
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
                    3, ATRA_FLOAT, 0, 0, NULL);
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
            atra_gl_ebo_enable(mesh->vbo_ids[8]);
        }
    }
    glm_mat4_mul(proj, modelview, mvp);
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
atra_mesh_drawinstanced(Mesh *mesh, Material *mat, mat4 *transforms, int count,
        float *bone_transforms, uint bone_count)
{
    ATRA_UNUSED(transforms);

    mat4 view, proj, modelview, normal, mvp, gl_transform;
    //uint instance_vboid = 0;
    uint i;

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
    //instance_vboid = atra_gl_vbo_create(transforms, count * sizeof(float), 0);
    
    if (bone_transforms != NULL) {
        atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_BONES],
                bone_transforms, ATRA_SHADER_UNIFORM_MAT4, bone_count);
    }

    if (mesh->vbo_ids[ATRA_SHADER_LOC_M_MODEL] == 0) {
        mesh->vbo_ids[ATRA_SHADER_LOC_M_MODEL] = atra_gl_vbo_create(transforms, count * 16 * sizeof(float), 1);
    } else {
        atra_gl_vbo_overwrite(mesh->vbo_ids[ATRA_SHADER_LOC_M_MODEL], transforms, count * 16 * sizeof(float), 1);
    }
    for (i = 0; i < 4; i++) {
        atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_M_MODEL] + i);
        atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_M_MODEL] + i,
                4, ATRA_FLOAT, 0, sizeof(mat4), (void *)(i * sizeof(vec4)));
        atra_gl_vao_setattribdivisor(mat->shader.locs[ATRA_SHADER_LOC_M_MODEL] + i, 1);
    }
    //atra_gl_vbo_disable();
    //atra_gl_vao_disable();

    atra_gl_matrix_gettransform(gl_transform);
    glm_mat4_mul(view, gl_transform, modelview);

    if (mat->shader.locs[ATRA_SHADER_LOC_M_NORMAL] != -1) {
        glm_mat4_inv(gl_transform, normal);
        glm_mat4_transpose(normal);
        atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_NORMAL],
                normal, ATRA_SHADER_UNIFORM_MAT4, 1);
    }

	if (mat->shader.locs[ATRA_SHADER_LOC_V_SCREENRES] != -1) {
		ivec2 screen_res = {
			atra_window_getwidth(),
			atra_window_getheight()
		};
		atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_V_SCREENRES],
				screen_res, ATRA_SHADER_UNIFORM_IVEC2, 1);
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
    if (atra_gl_vao_enable(mesh->vao_id) != 0) {
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

        /* Bind vertex bone IDs */
        if (mat->shader.locs[ATRA_SHADER_LOC_V_BONEIDS] != -1) {
            atra_gl_vbo_enable(mesh->vbo_ids[6]);
            atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_BONEIDS],
                    4, ATRA_INT, 0, 0, NULL);
            atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_BONEIDS]);
        }

        /* Bind vertex bone weights */
        if (mat->shader.locs[ATRA_SHADER_LOC_V_BONEWEIGHTS] != -1) {
            atra_gl_vbo_enable(mesh->vbo_ids[7]);
            atra_gl_vao_setattrib(mat->shader.locs[ATRA_SHADER_LOC_V_BONEWEIGHTS],
                    4, ATRA_FLOAT, 0, 0, NULL);
            atra_gl_vao_enableattrib(mat->shader.locs[ATRA_SHADER_LOC_V_BONEWEIGHTS]);
        }

        if (mesh->indices != NULL) {
            atra_gl_ebo_enable(mesh->vbo_ids[8]);
        }

    }
    glm_mat4_mul(proj, modelview, mvp);
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
ATRA_API
void
atra_mesh_export(Mesh *mesh, const char *path)
{
    ATRA_UNUSED(mesh);
    ATRA_UNUSED(path);
}

/* Returns a new minimum bounding box for a mesh */
ATRA_API
AABB
atra_mesh_createaabb(Mesh *mesh)
{
    AABB box = {0};

    ATRA_UNUSED(mesh);

    return box;
}

/* Calculates mesh tangents */
ATRA_API
void
atra_mesh_calctangents(Mesh *mesh)
{
    const uint index_count = mesh->tri_count * 3;
    const uint *indices = mesh->indices;

    float *verts = mesh->verts;
    float *uvs = mesh->uvs;
    float *tans = mesh->tangents;

    uint i;

    for (i = 0; i < index_count; i += 3) {
        const uint idx1 = indices[i + 0];
        const uint idx2 = indices[i + 1];
        const uint idx3 = indices[i + 2];
                        
        float *pos1 = verts + (idx1 * 3);
        float *pos2 = verts + (idx2 * 3);
        float *pos3 = verts + (idx3 * 3);
                        
        float *uv1 = uvs + (idx1 * 2);
        float *uv2 = uvs + (idx2 * 2);
        float *uv3 = uvs + (idx3 * 2);
                        
        vec3 e1, e2;
        vec2 d_uv1, d_uv2;
                        
        glm_vec3_sub(pos2, pos1, e1);
        glm_vec3_sub(pos3, pos1, e2);
        glm_vec2_sub(uv2, uv1, d_uv1);
        glm_vec2_sub(uv3, uv1, d_uv2);
                        
        float f = 1.0f / (d_uv1[0] * d_uv2[1] - d_uv2[0] * d_uv1[1]);
        vec3 tan;
                        
        tan[0] = f * (d_uv2[1] * e1[0] - d_uv1[1] * e2[0]);
        tan[1] = f * (d_uv2[1] * e1[1] - d_uv1[1] * e2[1]);
        tan[2] = f * (d_uv2[1] * e1[2] - d_uv1[1] * e2[2]);
                        
        glm_vec3_normalize(tan);
                        
        memcpy(tans + (idx1 * 3), tan, 3 * sizeof(float));
        memcpy(tans + (idx2 * 3), tan, 3 * sizeof(float));
        memcpy(tans + (idx3 * 3), tan, 3 * sizeof(float));
    }
}

/* Computes a mesh from a heightmap image */
ATRA_API
Mesh
atra_mesh_loadheightmap(Image *heightmap, vec3 size)
{
    Mesh mesh = {0};

    ATRA_UNUSED(heightmap);
    ATRA_UNUSED(size);

    return mesh;
}


/*
 *  Material functions
 */

/* Loads materials from a model file (GLTF).
 * Returns an array of materials and an amount of materials */
ATRA_API
Material *
atra_material_loadfile(const char *path, int *mat_count)
{
    
    Material *materials = NULL;
    uint count = 0;
    uint i;

    if (atra_util_filehasext(path, ".mtl")) {
        tinyobj_material_t *mats = NULL;
        int result = tinyobj_parse_mtl_file(&mats, &count, path);

        if (result != TINYOBJ_SUCCESS) {
            atra_log_print(LOG_WARN, "model: Failed to parse materials file: %s", path);
        }

        /* TODO: Process materials to return */

        tinyobj_materials_free(mats, count);
    }

    if (materials == NULL) {
        for (i = 0; i < count; i++) {
            /* TODO: Get default info */
            materials[i].shader.prog_id = atra_gl_shader_getdefaultid();
            materials[i].shader.locs = atra_gl_shader_getdefaultlocs();
        }
    }
    *mat_count = count;

    return materials;
}


/* Loads the default material (supports diffuse, specular and normal maps) */
ATRA_API
Material
atra_material_loaddefault(void)
{
    Material mat = {0};
    
    //mat.maps = calloc(ATRA_TEXTURE_TOTAL, sizeof(MaterialMap));
    mat.shader.prog_id = atra_gl_shader_getdefaultid();
    mat.shader.locs = atra_gl_shader_getdefaultlocs();

    mat.maps[ATRA_TEXTURE_ALBEDO].tex = (Texture){atra_gl_tex_getdefaultid(),
            1, 1, 1, ATRA_SAMPLE_NEAREST, ATRA_SAMPLE_NEAREST};
    
    
    mat.maps[ATRA_TEXTURE_ALBEDO].color = (Color){0xFF, 0xFF, 0xFF, 0xFF};
    mat.maps[ATRA_TEXTURE_SPECULAR].color = (Color){0xFF, 0xFF, 0xFF, 0xFF};

    return mat;
}

/* Deletes material data from GPU */
ATRA_API
void
atra_material_delete(Material *mat)
{
    int i;

    if (mat->shader.prog_id != atra_gl_shader_getdefaultid()) {
        atra_gl_shader_delete(mat->shader.prog_id);
        free(mat->shader.locs);
    }

    for (i = 0; i < ATRA_TEXTURE_TOTAL; i++) {
        if (mat->maps[i].tex.id != atra_gl_tex_getdefaultid()) {
            atra_gl_tex_delete(mat->maps[i].tex.id);
        }
    }
}

/* Sets the texture for a material type */
ATRA_API
void
atra_material_settexture(Material *mat, int tex_type, Texture *tex)
{
    mat->maps[tex_type].tex = *tex;
}


/*
 *  Model animation functions
 */

/* Loads an array of model animations from a file (GLTF) */
ATRA_API
ModelAnim *
atra_anim_loadfile(const char *file, int *anim_count)
{
    ModelAnim *anim = NULL;

    /*if (atra_util_filehasext(file, ".gltf;.glb")) {*/
        anim = atra_gltf_loadmodelanims(file, anim_count);
    /*} else {
        atra_log_print(LOG_WARN, "model: Unable to load model animations from model file %s: " \
                "Unrecognised model file format", file);
    }*/

    return anim;
}

/* Updates the pose of a model to a specified frame of an animation */
ATRA_API
void
atra_anim_updatemodel(ModelAnim *anim, Model *model, int frame)
{
    int m, i, j;

    if (anim->frame_count > 0 &&
            anim->bones != NULL &&
            anim->frame_poses != NULL) {

        frame = frame % anim->frame_count;

        for (m = 0; m < model->mesh_count; m++) {
            vec3 anim_vert = {0};
            vec3 anim_norm = {0};
            
            vec3 in_trans = {0};
            versor in_rot = {0};

            vec3 out_trans = {0};
            versor out_rot = {0};
            vec3 out_scale = {0};

            int v_counter = 0;
            int bone_counter = 0;
            int bone_id = 0;
            float bone_weight = 0.0f;

            for (i = 0; i < model->meshes[m].vert_count; i++) {
                model->meshes[m].anim_verts[v_counter + 0] = 0;
                model->meshes[m].anim_verts[v_counter + 1] = 0;
                model->meshes[m].anim_verts[v_counter + 2] = 0;

                model->meshes[m].anim_norms[v_counter + 0] = 0;
                model->meshes[m].anim_norms[v_counter + 1] = 0;
                model->meshes[m].anim_norms[v_counter + 2] = 0;

                for (j = 0; j < 4; j++) {
                    bone_id = model->meshes[m].bone_ids[bone_counter];
                    bone_weight = model->meshes[m].bone_weights[bone_counter];

                    glm_vec3_copy(model->bind_pose[bone_id].translation, in_trans);
                    glm_quat_copy(model->bind_pose[bone_id].rotation, in_rot);

                    glm_vec3_copy(anim->frame_poses[frame][bone_id].translation, out_trans);
                    glm_quat_copy(anim->frame_poses[frame][bone_id].rotation, out_rot);
                    glm_vec3_copy(anim->frame_poses[frame][bone_id].scale, out_scale);

                    /* Process vertices */
                    versor tmp_rot = {0};

                    glm_vec3_copy(model->meshes[m].verts + v_counter, anim_vert);
                    glm_vec3_sub(anim_vert, in_trans, anim_vert);
                    glm_vec3_mul(anim_vert, out_scale, anim_vert);

                    glm_quat_inv(in_rot, tmp_rot);
                    glm_quat_mul(out_rot, tmp_rot, tmp_rot);
                    glm_quat_rotatev(tmp_rot, anim_vert, anim_vert);
                    glm_vec3_add(anim_vert, out_trans, anim_vert);

                    model->meshes[m].anim_verts[v_counter + 0] += anim_vert[0] * bone_weight;
                    model->meshes[m].anim_verts[v_counter + 1] += anim_vert[1] * bone_weight;
                    model->meshes[m].anim_verts[v_counter + 2] += anim_vert[2] * bone_weight;

                    if (model->meshes[m].norms != NULL) {
                        /* Process normals */
                        glm_vec3_copy(model->meshes[m].norms + v_counter, anim_norm);
                        glm_quat_rotatev(tmp_rot, anim_norm, anim_norm);

                        model->meshes[m].anim_norms[v_counter + 0] += anim_norm[0] * bone_weight;
                        model->meshes[m].anim_norms[v_counter + 1] += anim_norm[1] * bone_weight;
                        model->meshes[m].anim_norms[v_counter + 2] += anim_norm[2] * bone_weight;
                    }
                    bone_counter += 1;
                }
                v_counter += 3;
            }

            /* Upload new vertex information to GPU */
            atra_gl_vbo_update(model->meshes[m].vbo_ids[ATRA_SHADER_LOC_V_POSITION],
                    model->meshes->anim_verts, model->meshes[m].vert_count * 3 * sizeof(float), 0);
            atra_gl_vbo_update(model->meshes[m].vbo_ids[ATRA_SHADER_LOC_V_NORMAL],
                    model->meshes->anim_norms, model->meshes[m].vert_count * 3 * sizeof(float), 0);
        }
    }
}

/* Deletes model animation data */
ATRA_API
void
atra_anim_delete(ModelAnim *anim)
{
    int i;

    for (i = 0; i < anim->frame_count; i++) {
        free(anim->frame_poses[i]);
    }
    free(anim->frame_poses);
    free(anim->bones);
}

/* Deletes model animation array data */
ATRA_API
void
atra_anim_deletearray(ModelAnim *anims, int count)
{
    int i;

    for (i = 0; i < count; i++) {
        atra_anim_delete(anims + i);
    }
    free(anims);
}

/* Returns 1 if the model animation skeleton is a match */
ATRA_API
int
atra_anim_modelskeletonmatch(Model *model, ModelAnim *anim)
{
    int result = 1;
    int i;

    if (model->bone_count != anim->bone_count) {
        result = 0;
    } else {
        for (i = 0; i < model->bone_count; i++) {
            if (model->bones[i].parent != anim->bones[i].parent) {
                result = 0;
                break;
            }
        }
    }

    return result;
}

/* Loads an array of model animations from a file (GLTF) */
ATRA_API
ModelAnimRaw *
atra_rawanim_loadfile(const char *file, int *anim_count)
{
    ModelAnimRaw *anim = NULL;

    /*if (atra_util_filehasext(file, ".gltf;.glb")) {*/
        anim = atra_gltf_loadmodelanimsraw(file, anim_count);
    /*} else {
        atra_log_print(LOG_WARN, "model: Unable to load model animations from model file %s: " \
                "Unrecognised model file format", file);
    }*/

    return anim;
}

/* Updates the pose of a model, interpolating the resulting pose from the given time.
 * Allows for offsetting to bone arrays of other instances */
/*ATRA_API
void
atra_rawanim_updatemodel(ModelAnimRaw *anim, Model *model, float time, int offset)
{
    float *trans = model->curr_trans + offset * model->bone_count * 3;
    float *rot = model->curr_rot + offset * model->bone_count * 4;
    float *scale = model->curr_scale + offset * model->bone_count * 3;

    atra_rawanim_updatemodelext(anim, model, time, trans, rot, scale);
}*/

/* Updates the pose of a model to a specified frame of an animation
 * Modifies given transform pointers directly, rather than model internal transforms */
ATRA_API
void
atra_rawanim_updatemodelext(ModelAnimRaw *anim, Model *model, float time)
{
    int i, bone;
    uint start_frame = 0;
    uint end_frame = 0;
    float *trans_a, *trans_b;
    float *rot_a, *rot_b;
    float *scale_a, *scale_b;
    vec3 trans_out;
    versor rot_out;
    vec3 scale_out;
    float lerp_percent = 0.0f;

    //time = (time > anim->duration) ? (time - (uint)(time / anim->duration) * anim->duration) : time;
    while (time > anim->duration) {
        time -= anim->duration;
    }

    for (i = 0; i < anim->frame_count; i++) {
        start_frame = (i == 0) ? 0 : i - 1;
        end_frame = i;
        const float start_time = anim->frame_times[start_frame];
        const float end_time = anim->frame_times[end_frame];

        if (time < end_time) {
            const float time_diff = end_time - start_time;

            if (time_diff < 0.0f || time_diff > 0.0f) {
                lerp_percent = (time - start_time) / time_diff;
            }
            break;
        }
    }

    if (anim->frame_count > 0 &&
            anim->bones != NULL &&
            anim->frame_poses != NULL) {

        /* Bake bind pose translation into mesh */
        /*if (model->pose_init == 0) {
            for (m = 0; m < model->mesh_count; m++) {
                Mesh *mesh = &model->meshes[m];
                int v_counter = 0;
                int bone_counter = 0;
                int bone_id = 0;
                float bone_weight = 0.0f;
                vec3 anim_vert = {0};

                for (i = 0; i < model->meshes[m].vert_count; i++) {
                    glm_vec3_zero(&mesh->anim_verts[v_counter]);

                    for (j = 0; j < 4; j++, bone_counter++) {
                        bone_weight = mesh->bone_weights[bone_counter];

                        if (bone_weight == 0.0f) {
                            continue;
                        }
                        bone_id = mesh->bone_ids[bone_counter];

                        glm_vec3_sub(&mesh->verts[v_counter], model->bind_pose[bone_id].translation, anim_vert);
                        glm_vec3_muladds(anim_vert, bone_weight, &mesh->anim_verts[v_counter]);
                    }
                    v_counter += 3;
                }
                atra_gl_vbo_update(model->meshes[m].vbo_ids[ATRA_SHADER_LOC_V_POSITION],
                        model->meshes->anim_verts, model->meshes[m].vert_count * 3 * sizeof(float), 0);
            }
            model->pose_init = 1;
        }*/


        /* Initialise bind pose transform for each bone */
        /*if (model->pose_init == FALSE) {

            for (bone = 0; bone < model->bone_count; bone++) {
                glm_scale_make(local_mat, model->bind_pose[bone].scale);
                glm_quat_rotate(local_mat, model->bind_pose[bone].rotation);
                glm_translate(local_mat, model->bind_pose);
                
                if (model->bones[bone].parent < 0) {
                    glm_mat4_copy(local_mat, model->bone_tf[bone].world_mat);
                }
            }

            model->pose_init = TRUE;
        }*/
/*
    completed_bones = calloc(model->bone_count, sizeof(int));
    completed_count = 0;
    
    vec4 *invbind, *world, *parent_world;
    int parent;

    while (completed_count < model->bone_count) {
        for (i = 0; i < (uint)model->bone_count; i++) {
            if (completed_bones[i]) {
                continue;
            }
            parent = model->bones[i].parent;
            parent = (parent < model->bone_count) ? parent : 0;
            invbind = (vec4 *)(model->invbind_mats + i * 16);
            world = (vec4 *)(model->world_mats + i * 16);

            glm_scale_make(local_mat, model->bind_pose[i].scale);
            glm_quat_rotate(local_mat, model->bind_pose[i].rotation, local_mat);
            glm_translate(local_mat, model->bind_pose[i].translation);

            if (parent < 0) {
                glm_mat4_copy(local_mat, world);
            } else if (completed_bones[parent] == 0) {
                continue;
            } else {
                parent_world = (vec4 *)(model->invbind_mats + parent * 16);
                glm_mat4_mul(parent_world, local_mat, world);
            }

            glm_mat4_inv(world, invbind);
            completed_bones[i] = 1;
            completed_count++;
        }
    }
    free(completed_bones);
*/
        /* Calculate final transform for each bone */
        int completed_bones[256] = {0};
        int completed_count = 0;

        vec4 *parent_world, *world, *invbind, *result;
        mat4 local_mat;
        int parent;

        while (completed_count < model->bone_count) {
            for (bone = 0; bone < model->bone_count; bone++) {
                if (completed_bones[bone]) {
                    continue;
                }
                parent = model->bones[bone].parent;
                parent = (parent < model->bone_count) ? parent : 0;

                invbind = (vec4 *)(model->invbind_mats + bone * 16);
                result = (vec4 *)(model->bone_mats + bone * 16);
                world = (vec4 *)(model->world_mats + bone * 16);

                trans_a = anim->frame_poses[start_frame][bone].translation;
                trans_b = anim->frame_poses[end_frame][bone].translation;
                rot_a = anim->frame_poses[start_frame][bone].rotation;
                rot_b = anim->frame_poses[end_frame][bone].rotation;
                scale_a = anim->frame_poses[start_frame][bone].scale;
                scale_b = anim->frame_poses[end_frame][bone].scale;

                glm_vec3_lerp(trans_a, trans_b, lerp_percent, trans_out);
                glm_quat_slerp(rot_a, rot_b, lerp_percent, rot_out);
                glm_vec3_lerp(scale_a, scale_b, lerp_percent, scale_out);

                glm_translate_make(local_mat, trans_out);
                glm_quat_rotate(local_mat, rot_out, local_mat);
                glm_scale(local_mat, scale_out);

                if (parent < 0) {
                    glm_mat4_copy(local_mat, world);
                } else if (completed_bones[parent] == 0) {
                    continue;
                } else {
                    parent_world = (vec4 *)(model->world_mats + parent * 16);
                    glm_mat4_mul(parent_world, local_mat, world);
                }
                glm_mat4_mul(world, invbind, result);

                completed_bones[bone] = 1;
                completed_count++;
            }
        }
    }
}


/* Deletes model animation data */
ATRA_API
void
atra_rawanim_delete(ModelAnimRaw *anim)
{
    int i;

    for (i = 0; i < anim->frame_count; i++) {
        free(anim->frame_poses[i]);
    }
    free(anim->frame_times);
    free(anim->frame_poses);
    free(anim->bones);
}

/* Deletes model animation array data */
ATRA_API
void
atra_rawanim_deletearray(ModelAnimRaw *anims, int count)
{
    int i;

    for (i = 0; i < count; i++) {
        atra_rawanim_delete(anims + i);
    }
    free(anims);
}

/* Returns 1 if the model animation skeleton is a match */
ATRA_API
int
atra_rawanim_modelskeletonmatch(Model *model, ModelAnimRaw *anim)
{
    int result = 1;
    int i;

    if (model->bone_count != anim->bone_count) {
        result = 0;
    } else {
        for (i = 0; i < model->bone_count; i++) {
            if (model->bones[i].parent != anim->bones[i].parent) {
                result = 0;
                break;
            }
        }
    }

    return result;
}


/*
 *  Immediate-style drawing functions
 */

/* Renders a bounding box to the screen as a wireframe */
ATRA_API
void
atra_draw_aabb(AABB box, Color color)
{
    ATRA_UNUSED(box);
    ATRA_UNUSED(color);
}

/* Draws a texture as a billboard */
ATRA_API
void
atra_draw_billboardtex(Camera *cam, Texture *tex, vec3 centre,
        float size, Color tint)
{
    ATRA_UNUSED(cam);
    ATRA_UNUSED(tex);
    ATRA_UNUSED(centre);
    ATRA_UNUSED(size);
    ATRA_UNUSED(tint);
}

/* Draws a subtexture as a billboard */
ATRA_API
void
atra_draw_billboardsubtex(Camera *cam, Texture *tex, IQuad rect,
        vec3 centre, float size, Color tint)
{
    ATRA_UNUSED(cam);
    ATRA_UNUSED(tex);
    ATRA_UNUSED(rect);
    ATRA_UNUSED(centre);
    ATRA_UNUSED(size);
    ATRA_UNUSED(tint);
}


/*
 *  Static function definitions
 */

ATRA_INLINE
Model
atra_gltf_load(const char *path)
{
    Model model = {0};
    size_t data_size = 0;
    mat4 static_transform;
    u8 *file_data;
    int prim_count;
    int prim_index;
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
        prim_count = 0;

        for (i = 0; i < data->scene->nodes_count; i++) {
            atra_gltf_getprimcount(data->scene->nodes[i], &prim_count);
        }
        model.mesh_count = prim_count;
        model.meshes = calloc(model.mesh_count, sizeof(Mesh));
        model.material_count = data->materials_count + 1;
        model.materials = malloc(model.material_count * sizeof(Material));
        model.mesh_materials = malloc(model.mesh_count * sizeof(int));
        model.bone_count = data->nodes_count;
        model.bones = calloc(model.bone_count, sizeof(BoneInfo));
        model.bind_pose = calloc(model.bone_count, sizeof(Transform));
        model.invbind_mats = malloc(model.bone_count * 16 * sizeof(float));
        model.world_mats = malloc(model.bone_count * 16 * sizeof(float));
        model.bone_mats = malloc(model.bone_count * 16 * sizeof(float));

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
    int *completed_bones, completed_count, parent;
    vec4 *invbind, *world, *parent_world;
    mat4 local_mat;
    uint i;

    for (i = 0; i < data->nodes_count; i++) {
        strcpy(model->bones[i].name, data->nodes[i].name == 0 ? "ANIMJOINT" : data->nodes[i].name);
        model->bones[i].parent =
                (data->nodes[i].parent != NULL) ?
                    (int)(data->nodes[i].parent - data->nodes) : -1;
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
            parent = model->bones[i].parent;
            parent = (parent < model->bone_count) ? parent : 0;
            invbind = (vec4 *)(model->invbind_mats + i * 16);
            world = (vec4 *)(model->world_mats + i * 16);

            glm_translate_make(local_mat, model->bind_pose[i].translation);
            glm_quat_rotate(local_mat, model->bind_pose[i].rotation, local_mat);
            glm_scale(local_mat, model->bind_pose[i].scale);

            if (parent < 0) {
                glm_mat4_copy(local_mat, world);
            } else if (completed_bones[parent] == 0) {
                continue;
            } else {
                parent_world = (vec4 *)(model->world_mats + parent * 16);
                glm_mat4_mul(parent_world, local_mat, world);
            }
            glm_mat4_inv(world, invbind);

            completed_bones[i] = 1;
            completed_count++;
        }
    }
    free(completed_bones);
}

ATRA_INLINE
Image
atra_gltf_loadimage(cgltf_image *image, const char *tex_path, Color tint)
{
    Option_Image rimage = {0};

    if (image->uri) {
        if (strlen(image->uri) > 5   &&
                image->uri[0] == 'd' &&
                image->uri[1] == 'a' &&
                image->uri[2] == 't' &&
                image->uri[3] == 'a' &&
                image->uri[4] == ':') {
            int i = 0;

            while (image->uri[i] != ',' && image->uri != 0) {
                i++;
            }

            if (image->uri[i] == 0) {
                atra_log_print(LOG_WARN, "model: glTF data URI is not a valid image");
            } else {
                int size = 0;
                int width, height;
                uchar *data = atra_base64_decode(image->uri + i + 1, &size);
                uchar *raw = stbi_load_from_memory(data, size, &width, &height, NULL, 4);
                
                free(data);

                rimage.some.data = raw;
                rimage.some.width = width;
                rimage.some.height = height;
                rimage.some.mipmaps = 1;

                atra_image_applytint(&rimage.some, tint);
            }
        } else {
            const int path_size = strlen(tex_path) + strlen(image->uri) + 2;
            char *path = malloc(path_size * sizeof(char));
            
            sprintf(path, "%s/%s", tex_path, image->uri);
            rimage = atra_image_loadfile(path);

            atra_image_applytint(&rimage.some, tint);
        }
    } else if (image->buffer_view) {
        u8 *data = malloc(image->buffer_view->size);
        int n = image->buffer_view->offset;
        int stride = image->buffer_view->stride ? image->buffer_view->stride : 1;

        for (uint i = 0; i < image->buffer_view->size; i++) {
            data[i] = ((u8 *)image->buffer_view->buffer->data)[n];
            n += stride;
        }
        int width, height;
        u8 *raw = stbi_load_from_memory(data, image->buffer_view->size, &width, &height, NULL, 4);
        
        free(data);

        rimage.some.data = raw;
        rimage.some.width = width;
        rimage.some.height = height;
        rimage.some.mipmaps = 1;

        atra_image_applytint(&rimage.some, tint);
    } else {
        rimage.some = atra_image_gencolor(tint, 1, 1);
    }

    return rimage.some;
}

ATRA_INLINE
void
atra_gltf_loadmaterial(Model *model, const char *path, const cgltf_data *data)
{
    const char *tex_path = atra_util_getdirpath(path);

    Color tint;
    int i;

    for (i = 0; i < model->material_count - 1; i++) {
        tint = (Color){255, 255, 255, 255};
        model->materials[i] = atra_material_loaddefault();

        if (data->materials[i].has_pbr_metallic_roughness) {
            tint.r = data->materials[i].pbr_metallic_roughness.base_color_factor[0] * 255;
            tint.g = data->materials[i].pbr_metallic_roughness.base_color_factor[1] * 255;
            tint.b = data->materials[i].pbr_metallic_roughness.base_color_factor[2] * 255;
            tint.a = data->materials[i].pbr_metallic_roughness.base_color_factor[3] * 255;

            model->materials[i].maps[ATRA_TEXTURE_ALBEDO].color = tint;

            if (data->materials[i].pbr_metallic_roughness.base_color_texture.texture) {
                Image albedo = atra_gltf_loadimage(
                        data->materials[i].pbr_metallic_roughness.base_color_texture.texture->image,
                        tex_path, tint);
                model->materials[i].maps[ATRA_TEXTURE_ALBEDO].tex = atra_texture_loadimage(&albedo);
                atra_image_destroy(&albedo);
            }
            tint = (Color){0xFF, 0xFF, 0xFF, 0xFF};

            if (data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture) {
                Image metallic_roughness = atra_gltf_loadimage(
                        data->materials[i].pbr_metallic_roughness.base_color_texture.texture->image,
                        tex_path, tint);
                model->materials[i].maps[ATRA_TEXTURE_ROUGHNESS].tex = atra_texture_loadimage(&metallic_roughness);

                float roughness = data->materials[i].pbr_metallic_roughness.roughness_factor;
                model->materials[i].maps[ATRA_TEXTURE_ROUGHNESS].value = roughness;

                float metalness = data->materials[i].pbr_metallic_roughness.metallic_factor;
                model->materials[i].maps[ATRA_TEXTURE_SPECULAR].value = metalness;

                atra_image_destroy(&metallic_roughness);
            }

            if (data->materials[i].normal_texture.texture) {
                Image normal = atra_gltf_loadimage(
                        data->materials[i].normal_texture.texture->image,
                        tex_path, tint);
                model->materials[i].maps[ATRA_TEXTURE_NORMAL].tex = atra_texture_loadimage(&normal);
                atra_image_destroy(&normal);
            }

            if (data->materials[i].occlusion_texture.texture) {
                Image occlusion = atra_gltf_loadimage(
                        data->materials[i].occlusion_texture.texture->image,
                        tex_path, tint);
                model->materials[i].maps[ATRA_TEXTURE_OCCLUSION].tex = atra_texture_loadimage(&occlusion);
                atra_image_destroy(&occlusion);
            }

            if (data->materials[i].emissive_texture.texture) {
                Image emission = atra_gltf_loadimage(
                        data->materials[i].emissive_texture.texture->image,
                        tex_path, tint);
                model->materials[i].maps[ATRA_TEXTURE_EMISSION].tex = atra_texture_loadimage(&emission);
                tint.r = (uchar)(data->materials[i].emissive_factor[0] * 255);
                tint.g = (uchar)(data->materials[i].emissive_factor[1] * 255);
                tint.b = (uchar)(data->materials[i].emissive_factor[2] * 255);
                model->materials[i].maps[ATRA_TEXTURE_EMISSION].color = tint;
                atra_image_destroy(&emission);
            }
        }
    }
    model->materials[model->material_count - 1] = atra_material_loaddefault();
}

ATRA_INLINE
void
atra_gltf_bindprimstobones(Model *model, const cgltf_data *data, int prim_index)
{
    uint node_id;
    int b;

    for (node_id = 0; node_id < data->nodes_count; node_id++) {
        if (data->nodes[node_id].mesh == &(data->meshes[prim_index])) {
            if (model->meshes[prim_index].bone_ids == NULL) {
                model->meshes[prim_index].bone_ids =
                        calloc(model->meshes[prim_index].vert_count * 4, sizeof(float));
                model->meshes[prim_index].bone_weights =
                        calloc(model->meshes[prim_index].vert_count * 4, sizeof(float));

                for (b = 0; b < model->meshes[prim_index].vert_count * 4; b++) {
                    if (b % 4 == 0) {
                        model->meshes[prim_index].bone_ids[b] = node_id;
                        model->meshes[prim_index].bone_weights[b] = 1.0f;
                    } else {
                        model->meshes[prim_index].bone_ids[b] = 0;
                        model->meshes[prim_index].bone_weights[b] = 0.0f;
                    }
                }
            }
        }
    }
}

ATRA_INLINE
ModelAnim *
atra_gltf_loadmodelanims(const char *path, int *anim_count)
{
    size_t data_size = 0;
    u8 *file_data = atra_util_loadfile(path, &data_size);
    uint a, i;
    int j;

    ModelAnim *anims = NULL;

    if (file_data == NULL) {
        return NULL;
    }

    cgltf_options options = {0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse(&options, file_data, data_size, &data);
    
    if (result == cgltf_result_success) {
        atra_log_print(LOG_INFO, "model: [%s] glTF animations (%s) count: %i",
                path, (data->file_type == 2) ? "glb" : "gltf",
                data->animations_count);
        
        result = cgltf_load_buffers(&options, data, path);

        if (result != cgltf_result_success) {
            atra_log_print(LOG_WARN, "model: [%s] unable to load glTF animations data", path);
        }
        anims = malloc(data->animations_count * sizeof(ModelAnim));
        *anim_count = data->animations_count;

        for (a = 0; a < data->animations_count; a++) {
            cgltf_animation *anim = data->animations + a;
            ModelAnim *output = anims + a;

            const float timestep = (1.0f / 60.0f);
            float anim_duration = 0.0f;

            for (i = 0; i < anim->channels_count; i++) {
                cgltf_animation_channel *channel = anim->channels + i;
                int framecounts = channel->sampler->input->count;
                float lastframetime = 0.0f;

                if (atra_gltf_readvalue(channel->sampler->input,
                        framecounts - 1, &lastframetime)) {
                    anim_duration = fmaxf(lastframetime, anim_duration);
                }
            }
            output->frame_count = (int)(anim_duration / timestep);
            output->bone_count = data->nodes_count;
            output->bones = malloc(output->bone_count * sizeof(BoneInfo));
            output->frame_poses = malloc(output->frame_count * sizeof(Transform *));
            //output->framerate TODO: Use framerate instead of const timestep

            for (j = 0; j < output->bone_count; j++) {
                strcpy(output->bones[j].name, data->nodes[j].name == 0 ? "ANIMJOINT" : data->nodes[j].name);
                output->bones[j].parent = (data->nodes[j].parent != NULL) ? data->nodes[j].parent - data->nodes : -1;
            }

            for (int frame = 0; frame < output->frame_count; frame++) {
                output->frame_poses[frame] = malloc(output->bone_count * sizeof(Transform));

                for (i = 0; i < (uint)output->bone_count; i++) {
                    if (data->nodes[i].has_translation) {
                        memcpy(&output->frame_poses[frame][i].translation,
                                data->nodes[i].translation, 3 * sizeof(float));
                    } else {
                        glm_vec3_zero(output->frame_poses[frame][i].translation);
                    }

                    if (data->nodes[i].has_rotation) {
                        memcpy(&output->frame_poses[frame][i].rotation,
                                data->nodes[i].rotation, 4 * sizeof(float));
                    } else {
                        glm_quat_identity(output->frame_poses[frame][i].rotation);
                    }
                    glm_quat_normalize(output->frame_poses[frame][i].rotation);

                    if (data->nodes[i].has_scale) {
                        memcpy(&output->frame_poses[frame][i].scale,
                                data->nodes[i].scale, 3 * sizeof(float));
                    } else {
                        glm_vec3_one(output->frame_poses[frame][i].scale);
                    }
                }
            }

            for (uint channel_id = 0; channel_id < anim->channels_count; channel_id++) {
                cgltf_animation_channel *channel = anim->channels + channel_id;
                cgltf_animation_sampler *sampler = channel->sampler;

                int bone_id = channel->target_node - data->nodes;

                for (int frame = 0; frame < output->frame_count; frame++) {
                    int shouldskip = 1;
                    int output_min = 0;
                    int output_max = 0;
                    float frametime = frame * timestep;
                    float lerp_percent = 0.0f;

                    for (j = 0; (ulong)j < sampler->input->count; j++) {
                        float inputframetime;

                        if (atra_gltf_readvalue(sampler->input, j, &inputframetime)) {
                            if (frametime < inputframetime) {
                                shouldskip = 0;
                                output_min = (j == 0) ? 0 : j - 1;
                                output_max = j;

                                float previnputtime = 0.0f;

                                if (atra_gltf_readvalue(sampler->input, output_min, &previnputtime)) {
                                    const float frametimediff = inputframetime - previnputtime;
                                    if (frametimediff < 0.0f || frametimediff > 0.0f) {
                                        lerp_percent = (frametime - previnputtime) / frametimediff;
                                    }
                                }
                                break;
                            }
                        } else {
                            break;
                        }
                    }

                    if (shouldskip) {
                        continue;
                    }

                    if (channel->target_path == cgltf_animation_path_type_translation) {
                        vec3 trans_start;
                        vec3 trans_end;
                        int success;

                        success = atra_gltf_readvalue(sampler->output, output_min, trans_start);
                        success = atra_gltf_readvalue(sampler->output, output_max, trans_end) || success;

                        if (success) {
                            glm_vec3_lerp(trans_start, trans_end, lerp_percent,
                                    output->frame_poses[frame][bone_id].translation);
                        }
                    }

                    if (channel->target_path == cgltf_animation_path_type_rotation) {
                        versor rot_start;
                        versor rot_end;
                        int success;

                        success = atra_gltf_readvalue(sampler->output, output_min, rot_start);
                        success = atra_gltf_readvalue(sampler->output, output_max, rot_end) || success;

                        if (success) {
                            glm_quat_slerp(rot_start, rot_end, lerp_percent,
                                    output->frame_poses[frame][bone_id].rotation);
                        }
                    }
                        
                    if (channel->target_path == cgltf_animation_path_type_scale) {
                        vec3 scale_start;
                        vec3 scale_end;
                        int success;

                        success = atra_gltf_readvalue(sampler->output, output_min, scale_start);
                        success = atra_gltf_readvalue(sampler->output, output_max, scale_end) || success;

                        if (success) {
                            glm_vec3_lerp(scale_start, scale_end, lerp_percent,
                                    output->frame_poses[frame][bone_id].scale);
                        }
                    }
                }
            }

            for (int frame = 0; frame < output->frame_count; frame++) {
                int *completed_bones = calloc(output->bone_count, sizeof(int));
                int completed_bone_count = 0;

                while (completed_bone_count < output->bone_count) {
                    for (i = 0; i < (uint)output->bone_count; i++) {
                        if (completed_bones[i]) {
                            continue;
                        }

                        if (output->bones[i].parent < 0) {
                            completed_bones[i] = 1;
                            completed_bone_count++;
                            continue;
                        }

                        if (!completed_bones[output->bones[i].parent]) {
                            continue;
                        }

                        glm_quat_mul(output->frame_poses[frame][output->bones[i].parent].rotation,
                                output->frame_poses[frame][i].rotation,
                                output->frame_poses[frame][i].rotation);
                        glm_quat_rotatev(output->frame_poses[frame][output->bones[i].parent].rotation,
                                output->frame_poses[frame][i].translation,
                                output->frame_poses[frame][i].translation);
                        glm_vec3_add(output->frame_poses[frame][output->bones[i].parent].translation,
                                output->frame_poses[frame][i].translation,
                                output->frame_poses[frame][i].translation);
                        glm_vec3_mul(output->frame_poses[frame][output->bones[i].parent].scale,
                                output->frame_poses[frame][i].scale,
                                output->frame_poses[frame][i].scale);
                            
                        completed_bones[i] = 1;
                        completed_bone_count++;
                    }
                }
                free(completed_bones);
            }
        }
        cgltf_free(data);
    } else {
        atra_log_print(LOG_WARN, "model: Failed to load glTF animation data: %s", path);
    }
    free(file_data);

    return anims;
}

ATRA_INLINE
ModelAnimRaw *
atra_gltf_loadmodelanimsraw(const char *path, int *anim_count)
{
    size_t data_size = 0;
    u8 *file_data = atra_util_loadfile(path, &data_size);
    uint a, i;
    int j;

    ModelAnimRaw *anims = NULL;

    if (file_data == NULL) {
        return NULL;
    }

    cgltf_options options = {0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse(&options, file_data, data_size, &data);
    
    if (result == cgltf_result_success) {
        atra_log_print(LOG_INFO, "model: [%s] glTF animations (%s) count: %i",
                path, (data->file_type == 2) ? "glb" : "gltf",
                data->animations_count);
        
        result = cgltf_load_buffers(&options, data, path);

        if (result != cgltf_result_success) {
            atra_log_print(LOG_WARN, "model: [%s] unable to load glTF animations data", path);
        }
        anims = malloc(data->animations_count * sizeof(ModelAnimRaw));
        *anim_count = data->animations_count;

        for (a = 0; a < data->animations_count; a++) {
            cgltf_animation *anim = data->animations + a;
            ModelAnimRaw *output = anims + a;

            float anim_duration = 0.0f;

            for (i = 0; i < anim->channels_count; i++) {
                cgltf_animation_channel *channel = anim->channels + i;
                int framecounts = channel->sampler->input->count;
                float lastframetime = 0.0f;

                if (atra_gltf_readvalue(channel->sampler->input, framecounts - 1, &lastframetime)) {
                    anim_duration = fmaxf(lastframetime, anim_duration);
                }
            }
            output->duration = anim_duration;
            output->frame_count = anim->channels->sampler->input->count;
            output->bone_count = data->nodes_count;
            output->frame_times = malloc(output->frame_count * sizeof(float));
            output->bones = malloc(output->bone_count * sizeof(BoneInfo));
            output->frame_poses = malloc(output->frame_count * sizeof(Transform *));

            for (j = 0; j < output->bone_count; j++) {
                strcpy(output->bones[j].name, data->nodes[j].name == 0 ? "ANIMJOINT" : data->nodes[j].name);
                output->bones[j].parent = (data->nodes[j].parent != NULL) ? data->nodes[j].parent - data->nodes : -1;
            }

            for (int frame = 0; frame < output->frame_count; frame++) {
                output->frame_poses[frame] = malloc(output->bone_count * sizeof(Transform));

                for (i = 0; i < (uint) output->bone_count; i++) {
                    if (data->nodes[i].has_translation) {
                        memcpy(&output->frame_poses[frame][i].translation,
                                data->nodes[i].translation, 3 * sizeof(float));
                    } else {
                        glm_vec3_zero(output->frame_poses[frame][i].translation);
                    }

                    if (data->nodes[i].has_rotation) {
                        memcpy(&output->frame_poses[frame][i].rotation,
                                data->nodes[i].rotation, 4 * sizeof(float));
                    } else {
                        glm_quat_identity(output->frame_poses[frame][i].rotation);
                    }
                    glm_quat_normalize(output->frame_poses[frame][i].rotation);

                    if (data->nodes[i].has_scale) {
                        memcpy(&output->frame_poses[frame][i].scale,
                                data->nodes[i].scale, 3 * sizeof(float));
                    } else {
                        glm_vec3_one(output->frame_poses[frame][i].scale);
                    }
                }
            }

            for (uint channel_id = 0; channel_id < anim->channels_count; channel_id++) {
                cgltf_animation_channel *channel = anim->channels + channel_id;
                cgltf_animation_sampler *sampler = channel->sampler;

                int bone_id = channel->target_node - data->nodes;

                for (j = 0; (ulong)j < sampler->input->count; j++) {
                    float inputframetime;

                    if (atra_gltf_readvalue(sampler->input, j, &inputframetime)) {
                        output->frame_times[j] = inputframetime;
                    } else {
                        break;
                    }
                }

                for (int frame = 0; frame < output->frame_count; frame++) {
                    
                    if (channel->target_path == cgltf_animation_path_type_translation) {
                        vec3 trans;

                        if (atra_gltf_readvalue(sampler->output, frame, trans)) {
                            memcpy(output->frame_poses[frame][bone_id].translation, trans, 3 * sizeof(float));
                        }
                    }

                    if (channel->target_path == cgltf_animation_path_type_rotation) {
                        versor rot;

                        if (atra_gltf_readvalue(sampler->output, frame, rot)) {
                            memcpy(output->frame_poses[frame][bone_id].rotation, rot, 4 * sizeof(float));
                        }
                    }
                        
                    if (channel->target_path == cgltf_animation_path_type_scale) {
                        vec3 scale;

                        if (atra_gltf_readvalue(sampler->output, frame, scale)) {
                            memcpy(output->frame_poses[frame][bone_id].scale, scale, 3 * sizeof(float));
                        }
                    }
                }
            }

            for (int frame = 0; frame < output->frame_count; frame++) {
                int *completed_bones = calloc(output->bone_count, sizeof(int));
                int completed_bone_count = 0;

                while (completed_bone_count < output->bone_count) {
                    for (i = 0; i < (uint)output->bone_count; i++) {
                        if (completed_bones[i]) {
                            continue;
                        }

                        if (output->bones[i].parent < 0) {
                            completed_bones[i] = 1;
                            completed_bone_count++;
                            continue;
                        }

                        if (completed_bones[output->bones[i].parent] == 0) {
                            continue;
                        }
                        /*
                        glm_quat_mul(output->frame_poses[frame][output->bones[i].parent].rotation,
                                output->frame_poses[frame][i].rotation,
                                output->frame_poses[frame][i].rotation);
                        glm_quat_rotatev(output->frame_poses[frame][output->bones[i].parent].rotation,
                                output->frame_poses[frame][i].translation,
                                output->frame_poses[frame][i].translation);
                        glm_vec3_add(output->frame_poses[frame][output->bones[i].parent].translation,
                                output->frame_poses[frame][i].translation,
                                output->frame_poses[frame][i].translation);
                        glm_vec3_mul(output->frame_poses[frame][output->bones[i].parent].scale,
                                output->frame_poses[frame][i].scale,
                                output->frame_poses[frame][i].scale);
                        */    
                        completed_bones[i] = 1;
                        completed_bone_count++;
                    }
                }
                free(completed_bones);
            }
        }
        cgltf_free(data);
    } else {
        atra_log_print(LOG_WARN, "model: Failed to load glTF animation data: %s", path);
    }
    free(file_data);

    return anims;
}

ATRA_INLINE
void
atra_gltf_loadmesh(cgltf_data *data, cgltf_mesh *mesh, Model *out_model,
        mat4 curr_transform, int *prim_index, const char *path)
{
    uint p, j, a, k;
    int v;

    ATRA_UNUSED(path);

    for (p = 0; p < mesh->primitives_count; p++) {
        for (j = 0; j < mesh->primitives[p].attributes_count; j++) {
            if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_position) {
                /* Vertex positions */
                cgltf_accessor *acc = mesh->primitives[p].attributes[j].data;
                int buff_size = acc->count * 3 * sizeof(float);

                out_model->meshes[*prim_index].vert_count = acc->count;
                out_model->meshes[*prim_index].anim_verts = malloc(buff_size);
                out_model->meshes[*prim_index].verts = atra_gltf_readvalueas(acc, cgltf_component_type_r_32f, 0);

                for (v = 0; v < out_model->meshes[*prim_index].vert_count; v++) {
                    float *vert = out_model->meshes[*prim_index].verts + (v * 3);
                    
                    glm_mat4_mulv3(curr_transform, vert, 1.0f, vert);
                }
                memcpy(out_model->meshes[*prim_index].anim_verts,
                        out_model->meshes[*prim_index].verts, buff_size);
            } else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_normal) {
                /* Vertex normals */
                cgltf_accessor *acc = mesh->primitives[p].attributes[j].data;
                int buff_size = acc->count * 3 * sizeof(float);

                out_model->meshes[*prim_index].anim_norms = malloc(buff_size);
                out_model->meshes[*prim_index].norms = atra_gltf_readvalueas(acc, cgltf_component_type_r_32f, 0);

                for (v = 0; v < out_model->meshes[*prim_index].vert_count; v++) {
                    float *norm = out_model->meshes[*prim_index].norms + (v * 3);

                    glm_mat4_mulv3(curr_transform, norm, 1.0f, norm);
                }
                memcpy(out_model->meshes[*prim_index].anim_norms,
                        out_model->meshes[*prim_index].norms, buff_size);
            } else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_texcoord) {
                /* Vertex texture UV coordinates */
                cgltf_accessor *acc = mesh->primitives[p].attributes[j].data;
                out_model->meshes[*prim_index].uvs = atra_gltf_readvalueas(acc, cgltf_component_type_r_32f, 0);
            } else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_joints) {
                /* Joints */
                cgltf_accessor *acc = mesh->primitives[p].attributes[j].data;
                uint bone_count = acc->count;
                uint total_bone_weights = bone_count * 4;

                out_model->meshes[*prim_index].bone_ids = malloc(total_bone_weights * sizeof(float));
                short *bones = atra_gltf_readvalueas(acc, cgltf_component_type_r_16, 0);

                for (a = 0; a < total_bone_weights; a++) {
                    out_model->meshes[*prim_index].bone_ids[a] = 0;

                    if (bones[a] < 0) {
                        continue;
                    }
                    cgltf_node *skin_joint = data->skins->joints[bones[a]];

                    for (k = 0; k < data->nodes_count; k++) {
                        if (data->nodes + k == skin_joint) {
                            out_model->meshes[*prim_index].bone_ids[a] = k;
                            break;
                        }
                    }
                }
                free(bones);
            } else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_weights) {
                /* Bone weights */
                cgltf_accessor *acc = mesh->primitives[p].attributes[j].data;
                out_model->meshes[*prim_index].bone_weights = atra_gltf_readvalueas(acc, cgltf_component_type_r_32f, 0);
            } else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_color) {
                /* Vertex color */
                cgltf_accessor *acc = mesh->primitives[p].attributes[j].data;
                out_model->meshes[*prim_index].colors = atra_gltf_readvalueas(acc, cgltf_component_type_r_8u, 1);
            }
        }
        cgltf_accessor *acc = mesh->primitives[p].indices;

        if (acc) {
            /* Indexed mesh */
            out_model->meshes[*prim_index].tri_count = acc->count / 3;
            out_model->meshes[*prim_index].indices = atra_gltf_readvalueas(acc, cgltf_component_type_r_32u, 0);
        } else {
            /* Unindexed mesh */
            out_model->meshes[*prim_index].tri_count = out_model->meshes[*prim_index].vert_count / 3;
        }

        if (mesh->primitives[p].material) {
            out_model->mesh_materials[*prim_index] = mesh->primitives[p].material - data->materials;
        } else {
            out_model->mesh_materials[*prim_index] = out_model->material_count - 1;
        }
        atra_gltf_bindprimstobones(out_model, data, *prim_index);
        *prim_index = *prim_index + 1;
    }
}

ATRA_INLINE
void
atra_gltf_getnodetransform(cgltf_node *node, mat4 current, vec4 *dest)
{
    glm_mat4_copy(current, dest);

    if (node->has_matrix) {
        mat4 node_tform = {
            {node->matrix[0], node->matrix[4], node->matrix[8], node->matrix[12]},
            {node->matrix[1], node->matrix[5], node->matrix[9], node->matrix[13]},
            {node->matrix[2], node->matrix[6], node->matrix[10], node->matrix[14]},
            {node->matrix[3], node->matrix[7], node->matrix[11], node->matrix[15]},
        };
        glm_mat4_mul(dest, node_tform, dest);
    }
    
    if (node->has_translation) {
        glm_translate(dest, node->translation);
    }

    if (node->has_rotation) {
        mat4 rot;
        versor rot_q = {node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]};
        glm_quat_mat4(rot_q, rot);
        glm_mat4_mul(dest, rot, dest);
    }

    if (node->has_scale) {
        glm_scale(dest, node->scale);
    }
}

static
void
atra_gltf_loadnode(cgltf_data *data, cgltf_node *node, Model *out_model,
        mat4 curr_transform, int *prim_index, const char *path)
{
    mat4 identity;
    mat4 tform;
    mat4 local_tform;
    uint i;

    glm_mat4_identity(identity);
    atra_gltf_getnodetransform(node, identity, local_tform);
    glm_mat4_mul(curr_transform, local_tform, tform);

    if (node->mesh != NULL) {
        mat4 vert_tform;

        glm_mat4_copy(tform, vert_tform);

        if (node->skin != NULL && node->parent != NULL) {
            glm_mat4_copy(local_tform, vert_tform);
            atra_log_print(LOG_WARN, "model: glTF node %s is skinned, but not root node: " \
                    "Parent transforms will be ignored", node->name);
        }
        atra_gltf_loadmesh(data, node->mesh, out_model, vert_tform, prim_index, path);
    }

    for (i = 0; i < node->children_count; i++) {
        atra_gltf_loadnode(data, node->children[i], out_model, tform, prim_index, path);
    }
}

static
void
atra_gltf_getprimcount(cgltf_node *node, int *out_count)
{
    uint i;

    if (node->mesh != NULL) {
        *out_count += node->mesh->primitives_count;
    }

    for (i = 0; i < node->children_count; i++) {
        atra_gltf_getprimcount(node->children[i], out_count);
    }
}

ATRA_INLINE
int
atra_gltf_readvalue(cgltf_accessor *acc, uint index, void *variable)
{
    uint typeElements = 0;

    switch (acc->type) {
        case cgltf_type_scalar: typeElements = 1; break;
        case cgltf_type_vec2: typeElements = 2; break;
        case cgltf_type_vec3: typeElements = 3; break;
        case cgltf_type_vec4:
        case cgltf_type_mat2: typeElements = 4; break;
        case cgltf_type_mat3: typeElements = 9; break;
        case cgltf_type_mat4: typeElements = 16; break;
        case cgltf_type_invalid: typeElements = 0; break;
        default: break;
    }

    uint typeSize = 0;

    switch (acc->component_type) {
        case cgltf_component_type_r_8u:
        case cgltf_component_type_r_8: typeSize = 1; break;
        case cgltf_component_type_r_16u:
        case cgltf_component_type_r_16: typeSize = 2; break;
        case cgltf_component_type_r_32f:
        case cgltf_component_type_r_32u: typeSize = 4; break;
        case cgltf_component_type_invalid: typeSize = 0; break;
        default: break;
    }

    uint singleElementSize = typeSize*typeElements;

    if (acc->count == 2) {
        if (index > 1) {
            return 0;
        }

        memcpy(variable, index == 0 ? acc->min : acc->max, singleElementSize);
        return 1;
    }

    memset(variable, 0, singleElementSize);

    if (acc->buffer_view == NULL || acc->buffer_view->buffer == NULL || acc->buffer_view->buffer->data == NULL) return false;

    if (!acc->buffer_view->stride) {
        void *readPosition = ((char *)acc->buffer_view->buffer->data) + (index*singleElementSize) + acc->buffer_view->offset + acc->offset;
        memcpy(variable, readPosition, singleElementSize);
    } else {
        void *readPosition = ((char *)acc->buffer_view->buffer->data) + (index*acc->buffer_view->stride) + acc->buffer_view->offset + acc->offset;
        memcpy(variable, readPosition, singleElementSize);
    }

    return 1;
}

ATRA_INLINE
void *
atra_gltf_readvalueas(cgltf_accessor *acc, cgltf_component_type type, int adjust_on_down_casting)
{
    uint count = acc->count;
    uint typeSize = 0;
    
    switch (type) {
        case cgltf_component_type_r_8u:
        case cgltf_component_type_r_8:      typeSize = 1; break;
        case cgltf_component_type_r_16u:
        case cgltf_component_type_r_16:     typeSize = 2; break;
        case cgltf_component_type_r_32f:
        case cgltf_component_type_r_32u:    typeSize = 4; break;
        case cgltf_component_type_invalid:  typeSize = 0; break;
        default: break;
    }
    uint typeElements = 0;

    switch (acc->type) {
        case cgltf_type_scalar:  typeElements = 1; break;
        case cgltf_type_vec2:    typeElements = 2; break;
        case cgltf_type_vec3:    typeElements = 3; break;
        case cgltf_type_vec4:
        case cgltf_type_mat2:    typeElements = 4; break;
        case cgltf_type_mat3:    typeElements = 9; break;
        case cgltf_type_mat4:    typeElements = 16; break;
        case cgltf_type_invalid: typeElements = 0; break;
        default: break;
    }

    if (acc->component_type == type) {
        void *array = malloc(count * typeElements * typeSize);

        for (uint i = 0; i < count; i++) {
            atra_gltf_readvalue(acc, i, (char *)array + i * typeElements * typeSize);
        }

        return array;
    } else {
        uint accTypeSize = 0;
        switch (acc->component_type) {
            case cgltf_component_type_r_8u:
            case cgltf_component_type_r_8: accTypeSize = 1; break;
            case cgltf_component_type_r_16u:
            case cgltf_component_type_r_16: accTypeSize = 2; break;
            case cgltf_component_type_r_32f:
            case cgltf_component_type_r_32u: accTypeSize = 4; break;
            case cgltf_component_type_invalid: accTypeSize = 0; break;
            default: break;
        }

        void *array = malloc(count * typeElements * typeSize);
        void *additionalArray = malloc(count * typeElements * accTypeSize);

        for (uint i = 0; i < count; i++) {
            atra_gltf_readvalue(acc, i, (char *)additionalArray + i * typeElements * accTypeSize);
        }

        switch (acc->component_type)
        {
            case cgltf_component_type_r_8u:
            {
                uchar *typedAdditionalArray = (uchar *)additionalArray;
                switch (type)
                {
                    case cgltf_component_type_r_8:
                    {
                        char *typedArray = (char *)array;
                        for (uint i = 0; i < count*typeElements; i++)
                        {
                            if (adjust_on_down_casting) typedArray[i] = (char)(typedAdditionalArray[i]/(UCHAR_MAX/CHAR_MAX));
                            else typedArray[i] = (char)typedAdditionalArray[i];
                        }

                    } break;
                    case cgltf_component_type_r_16u:
                    {
                        ushort *typedArray = (ushort *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (ushort)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_16:
                    {
                        short *typedArray = (short *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (short)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32f:
                    {
                        float *typedArray = (float *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (float)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32u:
                    {
                        uint *typedArray = (uint *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (uint)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_8u:
                    case cgltf_component_type_invalid:
                    default:
                    {
                        free(array);
                        free(additionalArray);
                        return NULL;
                    } break;
                }
            } break;
            case cgltf_component_type_r_8:
            {
                char *typedAdditionalArray = (char *)additionalArray;
                switch (type)
                {
                    case cgltf_component_type_r_8u:
                    {
                        uchar *typedArray = (uchar *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (uchar)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_16u:
                    {
                        ushort *typedArray = (ushort *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (ushort)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_16:
                    {
                        short *typedArray = (short *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (short)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32f:
                    {
                        float *typedArray = (float *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (float)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32u:
                    {
                        uint *typedArray = (uint *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (uint)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_8:
                    case cgltf_component_type_invalid:
                    default:
                    {
                        free(array);
                        free(additionalArray);
                        return NULL;
                    } break;
                }
            } break;
            case cgltf_component_type_r_16u:
            {
                ushort *typedAdditionalArray = (ushort *)additionalArray;
                switch (type)
                {
                    case cgltf_component_type_r_8u:
                    {
                        uchar *typedArray = (uchar *)array;
                        for (uint i = 0; i < count*typeElements; i++)
                        {
                            if (adjust_on_down_casting) typedArray[i] = (uchar)(typedAdditionalArray[i]/(USHRT_MAX/UCHAR_MAX));
                            else typedArray[i] = (uchar)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_8:
                    {
                        char *typedArray = (char *) array;
                        for (uint i = 0; i < count*typeElements; i++)
                        {
                            if (adjust_on_down_casting) typedArray[i] = (char)(typedAdditionalArray[i]/(USHRT_MAX/CHAR_MAX));
                            else typedArray[i] = (char)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_16:
                    {
                        short *typedArray = (short *)array;
                        for (uint i = 0; i < count*typeElements; i++)
                        {
                            if (adjust_on_down_casting) typedArray[i] = (short)(typedAdditionalArray[i]/(USHRT_MAX/SHRT_MAX));
                            else typedArray[i] = (short)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_32f:
                    {
                        float *typedArray = (float *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (float)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32u:
                    {
                        uint *typedArray = (uint *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (uint)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_16u:
                    case cgltf_component_type_invalid:
                    default:
                    {
                        free(array);
                        free(additionalArray);
                        return NULL;
                    } break;
                }
            } break;
            case cgltf_component_type_r_16:
            {
                short *typedAdditionalArray = (short *)additionalArray;
                switch (type)
                {
                    case cgltf_component_type_r_8u:
                    {
                        uchar *typedArray = (uchar *)array;
                        for (uint i = 0; i < count*typeElements; i++)
                        {
                            if (adjust_on_down_casting) typedArray[i] = (uchar)(typedAdditionalArray[i]/(SHRT_MAX/UCHAR_MAX));
                            else typedArray[i] = (uchar)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_8:
                    {
                        char *typedArray = (char *)array;
                        for (uint i = 0; i < count*typeElements; i++)
                        {
                            if (adjust_on_down_casting) typedArray[i] = (char)(typedAdditionalArray[i]/(SHRT_MAX/CHAR_MAX));
                            else typedArray[i] = (char)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_16u:
                    {
                        ushort *typedArray = (ushort *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (ushort)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32f:
                    {
                        float *typedArray = (float *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (float)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32u:
                    {
                        uint *typedArray = (uint *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (uint)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_16:
                    case cgltf_component_type_invalid:
                    default:
                    {
                        free(array);
                        free(additionalArray);
                        return NULL;
                    } break;
                }
            } break;
            case cgltf_component_type_r_32f:
            {
                float *typedAdditionalArray = (float *)additionalArray;
                switch (type)
                {
                    case cgltf_component_type_r_8u:
                    {
                        uchar *typedArray = (uchar *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (uchar)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_8:
                    {
                        char *typedArray = (char *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (char)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_16u:
                    {
                        ushort *typedArray = (ushort *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (ushort)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_16:
                    {
                        short *typedArray = (short *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (short)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32u:
                    {
                        uint *typedArray = (uint *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (uint)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32f:
                    case cgltf_component_type_invalid:
                    default:
                    {
                        free(array);
                        free(additionalArray);
                        return NULL;
                    } break;
                }
            } break;
            case cgltf_component_type_r_32u:
            {
                uint *typedAdditionalArray = (uint *)additionalArray;
                switch (type)
                {
                    case cgltf_component_type_r_8u:
                    {
                        uchar *typedArray = (uchar *)array;
                        for (uint i = 0; i < count*typeElements; i++)
                        {
                            if (adjust_on_down_casting) typedArray[i] = (uchar)(typedAdditionalArray[i]/(UINT_MAX/UCHAR_MAX));
                            else typedArray[i] = (uchar)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_8:
                    {
                        char *typedArray = (char *)array;
                        for (uint i = 0; i < count*typeElements; i++)
                        {
                            if (adjust_on_down_casting) typedArray[i] = (char)(typedAdditionalArray[i]/(UINT_MAX/CHAR_MAX));
                            else typedArray[i] = (char)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_16u:
                    {
                        ushort *typedArray = (ushort *)array;
                        for (uint i = 0; i < count*typeElements; i++)
                        {
                            if (adjust_on_down_casting) typedArray[i] = (ushort)(typedAdditionalArray[i]/(UINT_MAX/USHRT_MAX));
                            else typedArray[i] = (ushort)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_16:
                    {
                        short *typedArray = (short *)array;
                        for (uint i = 0; i < count*typeElements; i++)
                        {
                            if (adjust_on_down_casting) typedArray[i] = (short)(typedAdditionalArray[i]/(UINT_MAX/SHRT_MAX));
                            else typedArray[i] = (short)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_32f:
                    {
                        float *typedArray = (float *)array;
                        for (uint i = 0; i < count*typeElements; i++) typedArray[i] = (float)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32u:
                    case cgltf_component_type_invalid:
                    default:
                    {
                        free(array);
                        free(additionalArray);
                        return NULL;
                    } break;
                }
            } break;
            case cgltf_component_type_invalid:
            default:
            {
                free(array);
                free(additionalArray);
                return NULL;
            } break;
        }
        free(additionalArray);

        return array;
    }
}

ATRA_INLINE
int
atra_base64_getsize(char *in)
{
    int size = 0;
    int i;

    for (i = 0; in[4 * i] != 0; i++) {
        if (in[4 * i + 3] == '=') {
            if (in[4 * i + 2] == '=') {
                size += 1;
            } else {
                size += 2;
            }
        } else {
            size += 3;
        }
    }

    return size;
}

ATRA_INLINE
uchar *
atra_base64_decode(char *in, int *size)
{
    uchar a, b, c, d;
    uchar *buf;
    int i;

    *size = atra_base64_getsize(in);
    buf = malloc(*size * sizeof(char));

    for (i = 0; i < *size / 3; i++) {
        a = base64_table[(int)in[4 * i + 0]];
        b = base64_table[(int)in[4 * i + 1]];
        c = base64_table[(int)in[4 * i + 2]];
        d = base64_table[(int)in[4 * i + 3]];

        buf[3 * i + 0] = (a << 2) | (b >> 4);
        buf[3 * i + 1] = (b << 4) | (c >> 2);
        buf[3 * i + 2] = (c << 6) | d;
    }

    if (*size % 3 == 1) {
        i = *size / 3;
        a = base64_table[(int)in[4 * i + 0]];
        b = base64_table[(int)in[4 * i + 1]];

        buf[*size - 1] = (a << 2) | (b >> 4);
    } else if (*size % 3 == 2) {
        i = *size / 3;
        a = base64_table[(int)in[4 * i + 0]];
        b = base64_table[(int)in[4 * i + 1]];
        c = base64_table[(int)in[4 * i + 2]];

        buf[*size - 2] = (a << 2) | (b >> 4);
        buf[*size - 1] = (b << 4) | (c >> 2);
    }

    return buf;
}

ATRA_INLINE
void
atra_mat4_make_transrot(vec3 trans, versor rot, mat4 dest)
{
    glm_mat4_identity(dest);
    glm_translate(dest, trans);
    glm_quat_rotate(dest, rot, dest);
}

ATRA_INLINE
void
atra_log_printmatrix(mat4 m)
{
    atra_log_print(LOG_INFO, "\n"
            "[[%.3f, %.3f, %.3f, %.3f],\n" \
            " [%.3f, %.3f, %.3f, %.3f],\n" \
            " [%.3f, %.3f, %.3f, %.3f],\n" \
            " [%.3f, %.3f, %.3f, %.3f]]",
            m[0][0], m[0][1], m[0][2], m[0][3],
            m[1][0], m[1][1], m[1][2], m[1][3],
            m[2][0], m[2][1], m[2][2], m[2][3],
            m[3][0], m[3][1], m[3][2], m[3][3]);
}
