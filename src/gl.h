/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  gl.h - Provides an interface for OpenGL operations with OpenGL version
 *         checks and extension availability detection.  Also provides a
 *         rendering system which mimics the classic immediate-mode style.
 *
 *  */

#ifndef ATRA_GL_H
#define ATRA_GL_H

#include "common.h"

#define ATRA_OPENGL_VERSION_21

#if !defined(ATRA_OPENGL_VERSION_21) && \
        !defined(ATRA_OPENGL_VERSION_33)
    #define ATRA_OPENGL_VERSION_33
#endif

#if defined(ATRA_OPENGL_VERSION_21)
    #define ATRA_OPENGL_VERSION_STR             "OpenGL v2.1"
#elif defined(ATRA_OPENGL_VERSION_33)
    #define ATRA_OPENGL_VERSION_STR             "OpenGL v3.3"
#endif

#define ATRA_DEFAULT_BATCH_BUFFERS              1
#define ATRA_DEFAULT_BATCH_BUFFER_ELEMS         8192
#define ATRA_DEFAULT_BATCH_DRAWCALLS            256
#define ATRA_DEFAULT_BATCH_MAX_TEXTURE_UNITS    4
#define ATRA_MAX_MATRIX_STACK_SIZE              32
#define ATRA_MAX_SHADER_LOCATIONS               32
#define ATRA_CULL_DISTANCE_NEAR                 0.01
#define ATRA_CULL_DISTANCE_FAR                  1000.0

/* Texture defines (equivalent to OpenGL) */
#define ATRA_TEXTURE_WRAP_S                     0x2802      // GL_TEXTURE_WRAP_S
#define ATRA_TEXTURE_WRAP_T                     0x2803      // GL_TEXTURE_WRAP_T
#define ATRA_TEXTURE_MAG_FILTER                 0x2800      // GL_TEXTURE_MAG_FILTER
#define ATRA_TEXTURE_MIN_FILTER                 0x2801      // GL_TEXTURE_MIN_FILTER

#define ATRA_TEXTURE_FILTER_NEAREST             0x2600      // GL_NEAREST
#define ATRA_TEXTURE_FILTER_LINEAR              0x2601      // GL_LINEAR
#define ATRA_TEXTURE_FILTER_MIP_NEAREST         0x2700      // GL_NEAREST_MIPMAP_NEAREST
#define ATRA_TEXTURE_FILTER_NEAREST_MIP_LINEAR  0x2702      // GL_NEAREST_MIPMAP_LINEAR
#define ATRA_TEXTURE_FILTER_LINEAR_MIP_NEAREST  0x2701      // GL_LINEAR_MIPMAP_NEAREST
#define ATRA_TEXTURE_FILTER_MIP_LINEAR          0x2703      // GL_LINEAR_MIPMAP_LINEAR
#define ATRA_TEXTURE_FILTER_ANISOTROPIC         0x3000      // Anisotropic filter (custom identifier)

#define ATRA_TEXTURE_WRAP_REPEAT                0x2901      // GL_REPEAT
#define ATRA_TEXTURE_WRAP_CLAMP                 0x812F      // GL_CLAMP_TO_EDGE
#define ATRA_TEXTURE_WRAP_MIRROR_REPEAT         0x8370      // GL_MIRRORED_REPEAT
#define ATRA_TEXTURE_WRAP_MIRROR_CLAMP          0x8742      // GL_MIRROR_CLAMP_EXT

/* Matrix modes */
#define ATRA_MODELVIEW                          0x1700      // GL_MODELVIEW
#define ATRA_PROJECTION                         0x1701      // GL_PROJECTION
#define ATRA_TEXTURE                            0x1702      // GL_TEXTURE

/* Drawing modes */
#define ATRA_LINES                              0x0001      // GL_LINES
#define ATRA_TRIANGLES                          0x0004      // GL_TRIANGLES
#define ATRA_QUADS                              0x0007      // GL_QUADS

/* GL data types */
#define ATRA_UNSIGNED_BYTE                      0x1401      // GL_UNSIGNED_BYTE
#define ATRA_FLOAT                              0x1406      // GL_FLOAT


/* Possibly undefined GL extensions */
#ifndef GL_SHADING_LANGUAGE_VERSION
    #define GL_SHADING_LANGUAGE_VERSION         0x8B8C
#endif
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGB_S3TC_DXT1_EXT     0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT    0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT    0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT    0x83F3
#endif
#ifndef GL_ETC1_RGB8_OES
    #define GL_ETC1_RGB8_OES                    0x8D64
#endif
#ifndef GL_COMPRESSED_RGB8_ETC2
    #define GL_COMPRESSED_RGB8_ETC2             0x9274
#endif
#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
    #define GL_COMPRESSED_RGBA8_ETC2_EAC        0x9278
#endif
#ifndef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
    #define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG  0x8C00
#endif
#ifndef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
    #define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
    #define GL_COMPRESSED_RGBA_ASTC_4x4_KHR     0x93b0
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_8x8_KHR
    #define GL_COMPRESSED_RGBA_ASTC_8x8_KHR     0x93b7
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_TEXTURE_MAX_ANISOTROPY_EXT       0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT   0x84FF
#endif

enum gl_extension {
    ATRA_GL_EXT_VAO,                /* Vertex Array Objects */
    ATRA_GL_EXT_INSTANCING,         /* Instance rendering */
    ATRA_GL_EXT_TEX_ARRAY,          /* Texture arrays */
    ATRA_GL_EXT_TEX_NPOT,           /* Non-power-of-two textures */
    ATRA_GL_EXT_TEX_DEPTH,          /* Depth textures */
    ATRA_GL_EXT_TEX_FLOAT32,        /* Float textures */
    ATRA_GL_EXT_TEX_COMP_DXT,       /* DDS texture compression */
    ATRA_GL_EXT_TEX_COMP_ETC1,      /* ETC1 texture compression */
    ATRA_GL_EXT_TEX_COMP_ETC2,      /* ETC2/EAC texture compression */
    ATRA_GL_EXT_TEX_COMP_PVRT,      /* PVR texture compression */
    ATRA_GL_EXT_TEX_COMP_ASTC,      /* ASTC texture compression */
    ATRA_GL_EXT_TEX_MIRRORCLAMP,    /* Texture mirror wrapping */
    ATRA_GL_EXT_TEX_ANISOFILTER,    /* Anisotropic texture filtering */
    ATRA_GL_EXT_TOTAL
};

enum gl_limit {
    ATRA_GL_LIMIT_MAX_TEXTURE_UNITS,
    ATRA_GL_LIMIT_MAX_TEXTURE_DEPTH,
    ATRA_GL_LIMIT_MAX_DEPTH_BITS,
    ATRA_GL_LIMIT_MAX_ANISOTROPY,
    ATRA_GL_LIMIT_TOTAL
};

enum blend_mode {
    ATRA_BLEND_ALPHA,
    ATRA_BLEND_ADDITIVE,
    ATRA_BLEND_MULTIPLIED,
    ATRA_BLEND_ADD_COLORS,
    ATRA_BLEND_SUB_COLORS,
    ATRA_BLEND_CUSTOM
};

enum fbo_attach_type {
    ATRA_ATTACHMENT_COLOR_CHANNEL0 = 0,
    ATRA_ATTACHMENT_COLOR_CHANNEL1,
    ATRA_ATTACHMENT_COLOR_CHANNEL2,
    ATRA_ATTACHMENT_COLOR_CHANNEL3,
    ATRA_ATTACHMENT_COLOR_CHANNEL4,
    ATRA_ATTACHMENT_COLOR_CHANNEL5,
    ATRA_ATTACHMENT_COLOR_CHANNEL6,
    ATRA_ATTACHMENT_COLOR_CHANNEL7,
    ATRA_ATTACHMENT_DEPTH = 100,
    ATRA_ATTACHMENT_STENCIL = 200
};

enum fbo_texture_type {
    ATRA_ATTACHMENT_CUBEMAP_POSITIVE_X = 0,
    ATRA_ATTACHMENT_CUBEMAP_NEGATIVE_X,
    ATRA_ATTACHMENT_CUBEMAP_POSITIVE_Y,
    ATRA_ATTACHMENT_CUBEMAP_NEGATIVE_Y,
    ATRA_ATTACHMENT_CUBEMAP_POSITIVE_Z,
    ATRA_ATTACHMENT_CUBEMAP_NEGATIVE_Z,
    ATRA_ATTACHMENT_TEXTURE_2D = 100,
    ATRA_ATTACHMENT_RENDERBUFFER = 200
};

typedef struct VertexBuffer {
    int elem_count;         /* Number of primitive elements */

    int pos_count;          /* Vertex position count */
    int uv_count;           /* Vertex texture coordinate count */
    int color_count;        /* Vertex color count */

    float *verts;           /* Vertex positions */
    float *uvs;             /* Vertex texture coordinates */
    uchar *colors;          /* Vertex colors */
    uint *indices;          /* Vertex indices */

    uint vao_id;            /* OpenGL Vertex Array Object ID */
    uint vbo_ids[4];        /* OpenGL Vertex Buffer Object IDs */
} VertexBuffer;

typedef struct DrawCall {
    int mode;               /* Drawing mode (LINES, TRIANGLES, QUADS) */
    int vert_count;         /* Number of vertices */
    int vert_align;         /* Vertex alignment */
    uint tex_id;            /* Texture ID */
} DrawCall;

typedef struct RenderBatch {
    int buff_count;         /* Number of vertex buffers */
    int curr_buff;          /* Index of current buffer */
    VertexBuffer *vbuffer;  /* Vertex buffer array */
    DrawCall *draws;        /* Draw call array */
    int draw_count;         /* Number of draw calls */
    float curr_depth;       /* Current depth value for next draw */
} RenderBatch;

typedef struct GLExtSupport {
    int exts[ATRA_GL_EXT_TOTAL];
} GLExtSupport;

typedef struct GLLimits {
    int limits[ATRA_GL_LIMIT_TOTAL];
} GLLimits;

typedef struct GLState {
    int matrix_mode;            /* Current matrix mode */
    mat4 *matrix;               /* Pointer to current matrix*/
    mat4 modelview;             /* Default modelview matrix */
    mat4 proj;                  /* Default projection matrix */
    mat4 transform;             /* Transformation matrix */
    int transform_req;          /* Flag for requirement for transformation */
    mat4 stack[ATRA_MAX_MATRIX_STACK_SIZE];     /* Matrix stack */
    int stack_size;             /* Matrix stack size */

    uint def_tex_id;        /* Default texture for drawing */
    uint active_tex_ids[ATRA_DEFAULT_BATCH_MAX_TEXTURE_UNITS];  /* Texture IDs */
    uint def_vshader_id;    /* Default vertex shader ID */
    uint def_fshader_id;    /* Default fragment shader ID */
    uint def_shader_id;     /* Default shader program ID */
    int *def_shader_locs;   /* Default shader locations */
    uint shader_id;     /* Current shader ID to use for rendering */
    int *shader_locs;   /* Current shader locations to user for rendering*/

    int blend_mode;     /* Blending mode active */
    int blend_src_factor;    /* Blending source factor */
    int blend_dst_factor;    /* Blending destination factor */
    int blend_equation;      /* Blending equation */

    int fbuffer_width;          /* Default framebuffer width */
    int fbuffer_height;         /* Default framebuffer height */
} GLState;


/*
 *  Main module functions
 */

/* Initialises the OpenGL subsystem */
ATRA_API void atra_gl_init(int width, int height);

/* Stops the OpenGL subsystem and frees all allocated memory */
ATRA_API void atra_gl_stop(void);

/* Returns an enumerated ID of the currently used OpenGL version */
ATRA_API int atra_gl_getversion(void);

/* Runs checks to find which OpenGL features are available */
ATRA_API void atra_gl_detectextensions(void);

/* Checks for limits to some GPU capabilities, such as max texture size */
ATRA_API void atra_gl_detectlimits(void);


/*
 *  Render batch functions
 */

/* Creates a new render batch */
ATRA_API RenderBatch atra_gl_batch_create(int num_buffers, int buffer_elems);

/* Unloads a render batch */
ATRA_API void atra_gl_batch_delete(RenderBatch *batch);

/* Draws render batch data (update -> draw -> reset) */
ATRA_API void atra_gl_batch_draw(RenderBatch *batch);

/* Sets the active internal render batch (NULL for default) */
ATRA_API void atra_gl_batch_setactive(RenderBatch *batch);

/* Draws the active internal render batch */
ATRA_API void atra_gl_batch_drawactive(void);

/* Returns 1 if adding `vcount` vertices will not overflow the buffer */
ATRA_API int atra_gl_batch_checklimit(int vcount);

/* Set the current texture for the render batch */
ATRA_API void atra_gl_batch_settexture(uint tex_id);


/*  
 *  Matrix functions
 */

/* Copies the internal projection matrix into `m` */
ATRA_API void atra_gl_matrix_getprojection(vec4 *m);

/* Copies the internal model-view matrix into `m` */
ATRA_API void atra_gl_matrix_getmodelview(vec4 *m);

/* Copies the internal transformation matrix into `m` */
ATRA_API void atra_gl_matrix_gettransform(vec4 *m);

/* Replace the internal projection matrix with the contents of `m` */
ATRA_API void atra_gl_matrix_setprojection(vec4 *m);

/* Replace the internal model-view matrix with the contents of `m` */
ATRA_API void atra_gl_matrix_setmodelview(vec4 *m);


/*
 *  Immediate-mode vertex operations
 */

/* Initialise immediate drawing mode */
ATRA_API void atra_gl_begin(int mode);

/* End immediate drawing mode */
ATRA_API void atra_gl_end(void);

/* Define a 2D vertex position (int) */
ATRA_API void atra_gl_vertex2i(int x, int y);

/* Define a 2D vertex position (float) */
ATRA_API void atra_gl_vertex2f(float x, float y);

/* Define a 3D vertex position (float) */
ATRA_API void atra_gl_vertex3f(float x, float y, float z);

/* Define a vertex texture UV coordinate */
ATRA_API void atra_gl_texcoord2f(float x, float y);

/* Define a vertex normal */
ATRA_API void atra_gl_normal3f(float x, float y, float z);

/* Define a vertex color (RGBA8) */
ATRA_API void atra_gl_color4ub(u8 r, u8 g, u8 b, u8 a);

/* Define a vertex color (RGB) */
ATRA_API void atra_gl_color3f(float r, float g, float b);

/* Define a vertex color (RGBA) */
ATRA_API void atra_gl_color4f(float r, float g, float b, float a);


/* 
 *  Immediate-mode matrix operations
 */

/* Selects the current matrix to manipulate */
ATRA_API void atra_gl_matrixmode(int mode);

/* Pushes the current matrix to the matrix stack */
ATRA_API void atra_gl_pushmatrix(void);

/* Pops the matrix at the top of the matrix stack and makes it current */
ATRA_API void atra_gl_popmatrix(void);

/* Loads the identity matrix as current */
ATRA_API void atra_gl_loadidentity(void);

/* Translates the current matrix */
ATRA_API void atra_gl_translate(float x, float y, float z);

/* Rotates the current matrix */
ATRA_API void atra_gl_rotate(float angle, float x, float y, float z);

/* Scales the current matrix */
ATRA_API void atra_gl_scale(float x, float y, float z);

/* Multiplies the current matrix with another, given as an argument */
ATRA_API void atra_gl_multmatrix(vec4 *mat);

/* Generates a projection matrix from the given camera frustum */
ATRA_API void atra_gl_frustum(double left, double right, double bottom,
        double top, double znear, double zfar);

/* Generates an orthographic matrix from the given camera frustum */
ATRA_API void atra_gl_ortho(double left, double right, double bottom,
        double top, double znear, double zfar);

/* Sets the current viewport area */
ATRA_API void atra_gl_viewport(int x, int y, int width, int height);


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
ATRA_API uint atra_gl_vao_create(void);

/* Creates a new vertex buffer */
ATRA_API uint atra_gl_vbo_create(void *buffer, int size, int dynamic);

/* Creates a new element buffer (usually for indices) */
ATRA_API uint atra_gl_ebo_create(void *buffer, int size, int dynamic);

/* Updates a vertex buffer with new data */
ATRA_API void atra_gl_vbo_update(uint vbo_id, void *data, int size, int offset);

/* Deletes the given vertex array */
ATRA_API void atra_gl_vao_delete(uint vao_id);

/* Deletes the given vertex buffer */
ATRA_API void atra_gl_vbo_delete(uint vbo_id);

/* Enables the given vertex array */
ATRA_API int atra_gl_vao_enable(uint vao_id);

/* Disables the current vertex array */
ATRA_API void atra_gl_vao_disable(void);

/* Enables the given vertex buffer */
ATRA_API void atra_gl_vbo_enable(uint vbo_id);

/* Disables the current vertex buffer */
ATRA_API void atra_gl_vbo_disable(void);

/* Enables the given vertex buffer element */
ATRA_API void atra_gl_ebo_enable(uint ebo_id);

/* Disables the current vertex buffer element */
ATRA_API void atra_gl_ebo_disable(void);

/* Enable the given vertex attribute */
ATRA_API void atra_gl_vao_enableattrib(uint index);

/* Disable the given vertex attribute */
ATRA_API void atra_gl_vao_disableattrib(uint index);

/* Sets the data of a vertex attribute */
ATRA_API void atra_gl_vao_setattrib(uint index, int elem_size, int type,
        int normalised, int stride, void *pointer);

/* Sets the divisor of a vertex attribute */
ATRA_API void atra_gl_vao_setattribdivisor(uint index, int divisor);

/* Sets the vertex attribute default value */
ATRA_API void atra_gl_vao_setattribdefault(int loc, const void *data, int type,
        int count);

/* Draws the current vertex array */
ATRA_API void atra_gl_vao_draw(int offset, int count);

/* Draws the current vertex array with indexed elements */
ATRA_API void atra_gl_vao_drawelements(int offset, int count, void *buffer);

/* Draws the current vertex array (instanced) */
ATRA_API void atra_gl_vao_drawinstanced(int offset, int count, int instances);

/* Draws the current vertex array with indexed elements (instanced) */
ATRA_API void atra_gl_vao_drawelementsinstanced(int offset, int count,
        void *buffer, int instances);


/*
 *  OpenGL wrapper functions (Texture)
 */

/* Creates a new texture on the GPU with the given data */
ATRA_API uint atra_gl_tex_create(void *data, int width, int height, int mip_levels);

/* Creates a new depth buffer texture/renderbuffer for use with a FBO */
ATRA_API uint atra_gl_tex_createdepth(int width, int height, int use_rbuffer);

/* Deletes a texture from the GPU */
ATRA_API void atra_gl_tex_delete(uint tex_id);

/* Creates a new cubemap texture */
ATRA_API uint atra_gl_tex_createcubemap(void *data, int size);

/* Sets the currently active texture unit */
ATRA_API void atra_gl_tex_setactiveunit(int unit);

/* Binds the given texture to the currently active texture unit */
ATRA_API void atra_gl_tex_enable(uint tex_id);

/* Clears the currently active texture unit */
ATRA_API void atra_gl_tex_disable(void);

/* Bind the given cubemap texture to the currently active texture unit */
ATRA_API void atra_gl_tex_enablecubemap(uint tex_id);

/* Clears the currently active texture unit of a cubemap */
ATRA_API void atra_gl_tex_disablecubemap(void);

/* Set the given texture's parameters */
ATRA_API void atra_gl_tex_setparameter(uint tex_id, int param, int value);

/* Updates part of a texture with new data, area specified by a rectangle */
ATRA_API void atra_gl_tex_update(uint tex_id, IQuad rect, const void *data);

/* Generates mipmaps for a texture */
ATRA_API void atra_gl_tex_genmipmaps(uint tex_id, int width, int height, int *mipmaps);

/* Reads pixel data from a texture */
ATRA_API void * atra_gl_tex_readpixels(uint tex_id, int width, int height);

/* Reads pixel data from the screen's color buffer */
ATRA_API u8 * atra_gl_tex_readscreenpixels(int width, int height);

/* Calculates the total size of an image, in bytes */
ATRA_API int atra_gl_tex_calcsize(int width, int height);


/*
 *  OpenGL wrapper functions (Framebuffer)
 */

/* Loads a new, empty framebuffer */
ATRA_API uint atra_gl_fbo_create(void);

/* Deletes an existing framebuffer from GPU */
ATRA_API void atra_gl_fbo_delete(uint fbo_id);

/* Enable rendering to texture */
ATRA_API void atra_gl_fbo_enable(uint fbo_id);

/* Disable rendering to texture */
ATRA_API void atra_gl_fbo_disable(void);

/* Attaches a texture/renderbuffer to a framebuffer */
ATRA_API void atra_gl_fbo_attach(uint fbo_id, uint tex_id, int attach_type,
        int tex_type, int mip_level);

/* Returns true if framebuffer is complete */
ATRA_API int atra_gl_fbo_iscomplete(uint fbo_id);

/* Activate multiple draw color buffers */
ATRA_API void atra_gl_fbo_activatedrawbuffers(int count);


/*
 *  OpenGL wrapper functions (Shader)
 */

/* Returns the ID of a new shader program compiled from two code buffers */
ATRA_API uint atra_gl_shader_load(const char *vs_code, const char *fs_code);

/* Loads the default shader */
ATRA_API void atra_gl_shader_loaddefault(void);

/* Returns the ID of a shader compiled from the given code */
ATRA_API uint atra_gl_shader_compile(const char *code, int type);

/* Returns the ID of a shader program, linking two shaders */
ATRA_API uint atra_gl_shader_link(uint vs_id, uint fs_id);

/* Deletes a shader program */
ATRA_API void atra_gl_shader_delete(uint prog_id);

/* Deletes the default shader program */
ATRA_API void atra_gl_shader_deletedefault(void);

/* Enables a shader program */
ATRA_API void atra_gl_shader_enable(uint shader_id);

/* Disables the current shader program */
ATRA_API void atra_gl_shader_disable(void);

/* Returns the location of a shader uniform */
ATRA_API int atra_gl_shader_getuniformloc(uint prog_id, const char *name);

/* Returns the location of a shader attribute */
ATRA_API int atra_gl_shader_getattribloc(uint prog_id, const char *name);

/* Sets the value of a shader uniform */
ATRA_API void atra_gl_shader_setuniform(int loc, const void *data, int type, int count);

/* Sets the value of a shader attribute */
ATRA_API void atra_gl_shader_setattrib(int loc, const void *data, int type, int count);

/* Set shader currently active (id and locations) */
ATRA_API void atra_gl_shader_setshader(uint prog_id, int *locs);


/*
 *  OpenGL wrapper functions (General rendering)
 */

/* Enables color blending */
ATRA_API void atra_gl_enablecolorblend(void);

/* Disables color blending */
ATRA_API void atra_gl_disablecolorblend(void);

/* Enables depth testing for occlusion based on draw depth */
ATRA_API void atra_gl_enabledepthtest(void);

/* Disables depth testing */
ATRA_API void atra_gl_disabledepthtest(void);

/* Enables writing into the depth buffer */
ATRA_API void atra_gl_enabledepthmask(void);

/* Disables writing into the depth buffer */
ATRA_API void atra_gl_disabledepthmask(void);

/* Enables culling of polygons which face away from the camera */
ATRA_API void atra_gl_enablebackfaceculling(void);

/* Disables culling of polygons which face away from the camera */
ATRA_API void atra_gl_disablebackfaceculling(void);

/* Enables scissor test to prevent drawing outside of a specific screen area */
ATRA_API void atra_gl_enablescissortest(void);

/* Disables scissor test */
ATRA_API void atra_gl_disablescissortest(void);

/* Specifies area of the screen which is rendered to when scissor test is enabled */
ATRA_API void atra_gl_scissor(int x, int y, int w, int h);

/* Enables wire mode */
ATRA_API void atra_gl_enablewiremode(void);

/* Disables wire mode */
ATRA_API void atra_gl_disablewiremode(void);

/* Sets the width that lines drawn using ATRA_LINES should be drawn with */
ATRA_API void atra_gl_setlinewidth(float width);

/* Returns the current line width */
ATRA_API float atra_gl_getlinewidth(void);

/* Enables line anti-aliasing */
ATRA_API void atra_gl_enablesmoothlines(void);

/* Disables line anti-aliasing */
ATRA_API void atra_gl_disablesmoothlines(void);

/* Sets the color that the screen buffer should be cleared with */
ATRA_API void atra_gl_clearcolor(u8 r, u8 g, u8 b, u8 a);

/* Clears the screen's color and depth buffers */
ATRA_API void atra_gl_clearscreenbuffers(void);

/* Sets the blending mode */
ATRA_API void atra_gl_setblendmode(int mode);

/* Sets the blending mode factors and equation */
ATRA_API void atra_gl_setblendfactors(int src_factor, int dst_factor, int equation);


#endif /* ATRA_GL_H */
