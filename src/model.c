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

/* Creates a new, empty mesh */
ATRA_API
Mesh
atra_mesh_create(void)
{

}

/* Deletes mesh data from both the CPU and GPU */
ATRA_API void atra_mesh_delete(Mesh *mesh);

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
    mat4 model, view, proj, modelview;
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
    glm_mat4_identity(model);
    atra_camera_getmainview(view);
    atra_camera_getmainproj(proj);

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

    if (mat->shader.locs[ATRA_SHADER_LOC_M_NORMAL] != -1) {
        mat4 normal;
        glm_mat4_inv(transform, normal);
        glm_mat4_transpose(normal);
        atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_M_NORMAL],
                normal, ATRA_SHADER_UNIFORM_MAT4, 1);
    }

    /* Bind all available texture maps */
    for (i = 0; i < ATRA_TEXTURE_TOTAL; i++) {
        if (mat->maps[i].tex.id > 0) {

            if (i == ATRA_TEXTURE_CUBEMAP ||
                    i == ATRA_TEXTURE_IRRADIANCE ||
                    i == ATRA_TEXTURE_PREFILTER) {
                atra_texture_bindcubemap(&mat->maps[i].tex, i);
            } else {
                atra_texture_bind(&mat->maps[i].tex, i);
            }

            atra_shader_setuniform(mat->shader.locs[ATRA_SHADER_LOC_TEX_ALBEDO + i],
                    &i, ATRA_SHADER_UNIFORM_INT, 1);
        }

    }

    /* Bind VAO/VBOs */
}

/* Instance renders a mesh with a single material and different transforms */
ATRA_API void atra_mesh_drawinstanced(Mesh *mesh, Material *mat, mat4 *transforms, int count);

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
