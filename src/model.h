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

#ifndef ATRA_MODEL_H
#define ATRA_MODEL_H

#include "common.h"
#include "shader.h"
#include "texture.h"
#include "memory.h"
#include "physics.h"
#include "camera.h"

#define ATRA_MAX_MESH_VERTEX_BUFFERS    8
#define ATRA_DEFAULT_MESH_SIZE          8

enum buffer_flags {
    ATRA_MESH_BUFFER_POSITION   = BIT(0),
    ATRA_MESH_BUFFER_TEXCOORD   = BIT(1),
    ATRA_MESH_BUFFER_NORMALS    = BIT(2),
    ATRA_MESH_BUFFER_COLORS     = BIT(3),
    ATRA_MESH_BUFFER_TANGENTS   = BIT(4),
    ATRA_MESH_BUFFER_TEXCOORD2  = BIT(5)
};

/* A texture which is used to render an object. */
typedef struct MaterialMap {
    Texture tex;
    Color color;
    float value;
} MaterialMap;

/* Collection of related material maps and a shader which renders them */
typedef struct Material {
    Shader shader;                          /* Shader to bind when rendering with material */
    MaterialMap maps[ATRA_TEXTURE_TOTAL];   /* Array of texture maps used by material */
} Material;

/* Info  */
typedef struct IMMeshInfo {
    int limit;              /* Current maximum number of primitives */
    int mode;               /* Drawing mode */
    int prim_size;          

    int vert_count;
    int uv_count;
    int norm_count;
    int color_count;
    int tangent_count;
    int uv2_count; 
} IMMeshInfo;

/* Vertex data and OpenGL vertex array/buffers */
typedef struct Mesh {
    int vert_count;         /* Number of vertices in mesh */
    int tri_count;          /* Number of triangles in mesh */
    
    IMMeshInfo *im;         /* Mesh data unique to immediate-style drawing */

    float *verts;           /* f32: Vertices */
    float *uvs;             /* f32: Vertex UV texture coordinates */
    float *uvs2;            /* f32: Vertex secondary UV texture coordinates */
    float *norms;           /* f32: Vertex normal vectors */
    float *tangents;        /* f32: Tangents */
    u8 *colors;             /* u8:  Vertex color modifier */
    uint *indices;          /* u32: Vertex indices */

    float *anim_verts;      /* Animated vertex positions (post-transform) */
    float *anim_norms;      /* Animated normals (post-transform) */
    int *bone_ids;          /* Vertex bone IDs (max 4 per vertex) */
    float *bone_weights;    /* Vertex bone weight */

    uint vao_id;            /* OpenGL Vertex Array Object ID */
    uint *vbo_ids;          /* OpenGL Vertex Buffer Object IDs */
} Mesh;

/* Transformation data structure */
typedef struct Transform {
    vec3 translation;       /* Translation */
    versor rotation;        /* Rotation */
    vec3 scale;             /* Scale */
} Transform;

/* Model bone information */
typedef struct BoneInfo {
    char name[32];
    int parent;
} BoneInfo;

/* Model animation information */
typedef struct ModelAnim {
    int bone_count;         /* Number of bones */
    int frame_count;        /* Number of animation key-frames */
    BoneInfo *bones;        /* Bone information array */
    Transform **frame_poses;/* Poses array by frame */
} ModelAnim;

/* Collection of meshes and materials in an animated model */
typedef struct Model {
    mat4 transform;         /* Local transform matrix */

    int mesh_count;         /* Number of meshes */
    int material_count;     /* Number of materials */
    Mesh *meshes;           /* Array of meshes comprising the model */
    Material *materials;    /* Array of materials used by model */
    int *mesh_materials;    /* Material indices associated with each mesh */

    int bone_count;         /* Number of bones in model */
    BoneInfo *bones;        /* Bone information array */
    Transform *bind_pose;   /* Initial transforms */
} Model;


/*
 *  Model functions
 */

/* Returns a new model from model data from a file (GLTF) */
ATRA_API Model atra_model_loadfile(const char *path);

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
        float rot_angle, vec3 scale, Color tint);

/* Renders a model wireframe to the screen */
ATRA_API void atra_model_wiredraw(Model *model, vec3 pos, float scale, Color tint);

/* Renders a model wireframe to the screen (extended parameters) */
ATRA_API void atra_model_wiredrawext(Model *model, vec3 pos, vec3 rot_axis,
        float rot_angle, vec3 scale, Color tint);


/*
 *  Mesh functions
 */

/* Sets up a new mesh to be manually drawn to in an immediate-mode style.
 * Sets the new mesh as the currently active mesh.
 * 
 * NOTE: Vertex position and texture UVs are enabled by default.
 *       A vertex position must be specified BEFORE its other attributes.
 *       The mesh expects all enabled buffers to have the same number of entries. 
 *       Failing to meet these requirements may cause unexpected behaviour. */
ATRA_API Mesh atra_mesh_begin(int mode, int buffer_flags);

/* Finalises the currently active mesh and uploads its data to the GPU */
ATRA_API void atra_mesh_end(int dynamic);

/* Adds a vertex to the current mesh */
ATRA_API void atra_mesh_vertex3f(float x, float y, float z);

/* Adds a vertex to the current mesh (vec3 argument) */
ATRA_API void atra_mesh_vertexv(const vec3 pos);

/* Adds a texture UV coordinate to the current mesh */
ATRA_API void atra_mesh_texcoord2f(float x, float y);

/* Adds a texture UV coordinate to the current mesh (vec2 argument) */
ATRA_API void atra_mesh_texcoordv(const vec2 uv);

/* Adds a vertex normal to the current mesh */
ATRA_API void atra_mesh_normal3f(float x, float y, float z);

/* Adds a vertex normal to the current mesh (vec3 argument) */
ATRA_API void atra_mesh_normalv(const vec3 normal);

/* Adds a vertex color to the current mesh */
ATRA_API void atra_mesh_color4ub(u8 r, u8 g, u8 b, u8 a);

/* Adds a vertex color to the current mesh (4-array argument) */
ATRA_API void atra_mesh_colorv(const u8 *color);

/* Adds a vertex tangent to the current mesh */
ATRA_API void atra_mesh_tangent4f(float x, float y, float z, float w);

/* Adds a vertex tangent to the current mesh (vec4 argument) */
ATRA_API void atra_mesh_tangentv(const vec4 tangent);

/* Adds a secondary texture UV to the current mesh */
ATRA_API void atra_mesh_texcoord22f(float x, float y);

/* Adds a secondary texture UV to the current mesh (vec2 argument) */
ATRA_API void atra_mesh_texcoord2v(const vec2 uv2);

/* Sets the currently active mesh */
ATRA_API void atra_mesh_setactive(Mesh *mesh);

/* Returns a pointer to the currently active mesh */
ATRA_API Mesh * atra_mesh_getactive(void);

/*
 *  Non-immediate mesh functions
 */

/* Deletes mesh data from both the CPU and GPU */
ATRA_API void atra_mesh_delete(Mesh *mesh);

/* Uploads mesh vertex data to the GPU */
ATRA_API void atra_mesh_upload(Mesh *mesh, int dynamic);

/* Updates mesh vertex data in the GPU for a specific buffer index */
ATRA_API void atra_mesh_update(Mesh *mesh, int index, void *data, int size, int offset);

/* Renders a mesh with the specified material and transform */
ATRA_API void atra_mesh_draw(Mesh *mesh, Material *mat, mat4 transform);

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
ATRA_API int atra_anim_modelskeletonmatch(Model *model, ModelAnim *anim);


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

#endif /* ATRA_MODEL_H */
