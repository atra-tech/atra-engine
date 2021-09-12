#include "atra.h"

static Material material;
static Mesh mesh;
static Camera camera;

void
init (void)
{

}

void
handle(void)
{
    const float mouse_speed = 0.003f;
    const float mouse_max_bound = PI * 0.5f;
    const float mouse_min_bound = -PI * 0.5f;

    int mx, my;

    if (atra_window_isresized()) {
        atra_camera_setsize(&camera, 
                atra_window_getwidth(),
                atra_window_getheight());
        atra_camera_updateprojmat(&camera);
    }

    atra_mouse_getposchange(&mx, &my);

    if (mx || my) {
        camera.rot[0] -= my * mouse_speed;
        camera.rot[1] -= mx * mouse_speed;

        if (camera.rot[0] > mouse_max_bound) {
            camera.rot[0] = mouse_max_bound;
        } else if (camera.rot[0] < mouse_min_bound) {
            camera.rot[0] = mouse_min_bound;
        }

        if (camera.rot[1] > 2 * PI) {
            camera.rot[1] = 0.0f;
        } else if (camera.rot[1] < 0.0f) {
            camera.rot[1] = 2 * PI;
        }
    }
}

void
update(u32 dt)
{
    const float player_speed = 0.002f;
    const float speed = dt * player_speed * (1 + atra_key_isdown(ATRA_KEY_LEFT_SHIFT));
    //const vec3 speed_vec = {speed, speed, speed};

    vec3 velocity = {0.0f, 0.0f, 0.0f};

    if (atra_key_isdown(SDL_SCANCODE_W)) {
        glm_vec3_muladds(camera.dir, speed, velocity);
    }
    if (atra_key_isdown(SDL_SCANCODE_S)) {
        glm_vec3_muladds(camera.dir, -speed, velocity);
    }
    if (atra_key_isdown(SDL_SCANCODE_D)) {
        glm_vec3_muladds(camera.right, speed, velocity);
    }
    if (atra_key_isdown(SDL_SCANCODE_A)) {
        glm_vec3_muladds(camera.right, -speed, velocity);
    }
    //glm_vec3_normalize(velocity);
    //glm_vec3_muladds(velocity, speed, camera.pos);
    glm_vec3_add(velocity, camera.pos, camera.pos);

    atra_camera_update(&camera);
}

void
build_mesh(void)
{
    Image perlin_img;
    int rand_num = atra_util_randomint(0, 1000000);
    GLState *state = atra_gl_getinternalstate();

    perlin_img = atra_image_genperlinnoise(512, 512, rand_num, rand_num, 1.0);
    material.maps[0].tex = atra_texture_loadimage(&perlin_img);
    material.maps[0].color = (Color){0xFF, 0xFF, 0xFF, 0xFF};
    atra_image_destroy(&perlin_img);

    material.shader.prog_id = state->def_shader_id;
    material.shader.locs = state->def_shader_locs;

    mesh = atra_mesh_begin(ATRA_QUADS,
            ATRA_MESH_BUFFER_POSITION |
            ATRA_MESH_BUFFER_TEXCOORD |
            ATRA_MESH_BUFFER_COLORS);
    {
        atra_mesh_setactive(&mesh);

        /* Front */
        atra_mesh_vertex3f(0.3f, 0.9f, -1.0f);
        atra_mesh_vertex3f(0.3f, 0.3f, -1.0f);
        atra_mesh_vertex3f(0.9f, 0.3f, -1.0f);
        atra_mesh_vertex3f(0.9f, 0.9f, -1.0f);
        
        atra_mesh_texcoord2f(0.0f, 1.0f);
        atra_mesh_texcoord2f(0.0f, 0.0f);
        atra_mesh_texcoord2f(1.0f, 0.0f);
        atra_mesh_texcoord2f(1.0f, 1.0f);

        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);

        /* Back */
        atra_mesh_vertex3f(0.9f, 0.9f, -2.0f);
        atra_mesh_vertex3f(0.9f, 0.3f, -2.0f);
        atra_mesh_vertex3f(0.3f, 0.3f, -2.0f);
        atra_mesh_vertex3f(0.3f, 0.9f, -2.0f);

        atra_mesh_texcoord2f(0.0f, 1.0f);
        atra_mesh_texcoord2f(0.0f, 0.0f);
        atra_mesh_texcoord2f(1.0f, 0.0f);
        atra_mesh_texcoord2f(1.0f, 1.0f);

        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);

        /* Right */
        atra_mesh_vertex3f(0.9f, 0.9f, -1.0f);
        atra_mesh_vertex3f(0.9f, 0.3f, -1.0f);
        atra_mesh_vertex3f(0.9f, 0.3f, -2.0f);
        atra_mesh_vertex3f(0.9f, 0.9f, -2.0f);

        atra_mesh_texcoord2f(0.0f, 1.0f);
        atra_mesh_texcoord2f(0.0f, 0.0f);
        atra_mesh_texcoord2f(1.0f, 0.0f);
        atra_mesh_texcoord2f(1.0f, 1.0f);

        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);

        /* Left */
        atra_mesh_vertex3f(0.3f, 0.9f, -2.0f);
        atra_mesh_vertex3f(0.3f, 0.3f, -2.0f);
        atra_mesh_vertex3f(0.3f, 0.3f, -1.0f);
        atra_mesh_vertex3f(0.3f, 0.9f, -1.0f);

        atra_mesh_texcoord2f(0.0f, 1.0f);
        atra_mesh_texcoord2f(0.0f, 0.0f);
        atra_mesh_texcoord2f(1.0f, 0.0f);
        atra_mesh_texcoord2f(1.0f, 1.0f);

        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);

        /* Top */
        atra_mesh_vertex3f(0.3f, 0.9f, -2.0f);
        atra_mesh_vertex3f(0.3f, 0.9f, -1.0f);
        atra_mesh_vertex3f(0.9f, 0.9f, -1.0f);
        atra_mesh_vertex3f(0.9f, 0.9f, -2.0f);

        atra_mesh_texcoord2f(0.0f, 1.0f);
        atra_mesh_texcoord2f(0.0f, 0.0f);
        atra_mesh_texcoord2f(1.0f, 0.0f);
        atra_mesh_texcoord2f(1.0f, 1.0f);

        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);

        /* Bottom */
        atra_mesh_vertex3f(0.3f, 0.3f, -1.0f);
        atra_mesh_vertex3f(0.3f, 0.3f, -2.0f);
        atra_mesh_vertex3f(0.9f, 0.3f, -2.0f);
        atra_mesh_vertex3f(0.9f, 0.3f, -1.0f);

        atra_mesh_texcoord2f(0.0f, 1.0f);
        atra_mesh_texcoord2f(0.0f, 0.0f);
        atra_mesh_texcoord2f(1.0f, 0.0f);
        atra_mesh_texcoord2f(1.0f, 1.0f);

        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
        atra_mesh_color4ub(atra_util_randomint(127, 255),atra_util_randomint(127, 255),atra_util_randomint(127, 255), 0xFF);
    }
    atra_mesh_end(0);
}

void
render_mesh(void)
{
    mat4 transform;
    int width, height;

    width = atra_window_getwidth();
    height = atra_window_getheight();

    atra_gl_clearscreenbuffers();

    glm_mat4_identity(transform);
    //glm_perspective(90.0, width / (float)height, 0.01, 1000.0, transform);

    atra_gl_matrix_setmodelview(camera.view);
    atra_gl_matrix_setprojection(camera.proj);

    //atra_gl_begin3D(&camera);


    atra_mesh_draw(atra_mesh_getactive(), &material, transform);

    //atra_gl_end3D();
}

void
render(void)
{
    int width, height;

    width = atra_window_getwidth();
    height = atra_window_getheight();

    atra_gl_clearscreenbuffers();
    atra_gl_loadidentity();

    atra_gl_begin(ATRA_QUADS);
    {
        atra_gl_color4ub(0xFF, 0xFF, 0xFF, 0xFF);

        /* Front */
        atra_gl_vertex3f(0.3f, 0.9f, -1.0f);
        atra_gl_vertex3f(0.3f, 0.3f, -1.0f);
        atra_gl_vertex3f(0.9f, 0.3f, -1.0f);
        atra_gl_vertex3f(0.9f, 0.9f, -1.0f);
        
        atra_gl_texcoord2f(0.0f, 1.0f);
        atra_gl_texcoord2f(0.0f, 0.0f);
        atra_gl_texcoord2f(1.0f, 0.0f);
        atra_gl_texcoord2f(1.0f, 1.0f);

        /* Back */
        atra_gl_vertex3f(0.9f, 0.9f, -2.0f);
        atra_gl_vertex3f(0.9f, 0.3f, -2.0f);
        atra_gl_vertex3f(0.3f, 0.3f, -2.0f);
        atra_gl_vertex3f(0.3f, 0.9f, -2.0f);

        atra_gl_texcoord2f(0.0f, 1.0f);
        atra_gl_texcoord2f(0.0f, 0.0f);
        atra_gl_texcoord2f(1.0f, 0.0f);
        atra_gl_texcoord2f(1.0f, 1.0f);

        /* Right */
        atra_gl_vertex3f(0.9f, 0.9f, -1.0f);
        atra_gl_vertex3f(0.9f, 0.3f, -1.0f);
        atra_gl_vertex3f(0.9f, 0.3f, -2.0f);
        atra_gl_vertex3f(0.9f, 0.9f, -2.0f);

        atra_gl_texcoord2f(0.0f, 1.0f);
        atra_gl_texcoord2f(0.0f, 0.0f);
        atra_gl_texcoord2f(1.0f, 0.0f);
        atra_gl_texcoord2f(1.0f, 1.0f);

        /* Left */
        atra_gl_vertex3f(0.3f, 0.9f, -2.0f);
        atra_gl_vertex3f(0.3f, 0.3f, -2.0f);
        atra_gl_vertex3f(0.3f, 0.3f, -1.0f);
        atra_gl_vertex3f(0.3f, 0.9f, -1.0f);

        atra_gl_texcoord2f(0.0f, 1.0f);
        atra_gl_texcoord2f(0.0f, 0.0f);
        atra_gl_texcoord2f(1.0f, 0.0f);
        atra_gl_texcoord2f(1.0f, 1.0f);

        /* Top */
        atra_gl_vertex3f(0.3f, 0.9f, -2.0f);
        atra_gl_vertex3f(0.3f, 0.9f, -1.0f);
        atra_gl_vertex3f(0.9f, 0.9f, -1.0f);
        atra_gl_vertex3f(0.9f, 0.9f, -2.0f);

        atra_gl_texcoord2f(0.0f, 1.0f);
        atra_gl_texcoord2f(0.0f, 0.0f);
        atra_gl_texcoord2f(1.0f, 0.0f);
        atra_gl_texcoord2f(1.0f, 1.0f);

        /* Bottom */
        atra_gl_vertex3f(0.3f, 0.3f, -1.0f);
        atra_gl_vertex3f(0.3f, 0.3f, -2.0f);
        atra_gl_vertex3f(0.9f, 0.3f, -2.0f);
        atra_gl_vertex3f(0.9f, 0.3f, -1.0f);

        atra_gl_texcoord2f(0.0f, 1.0f);
        atra_gl_texcoord2f(0.0f, 0.0f);
        atra_gl_texcoord2f(1.0f, 0.0f);
        atra_gl_texcoord2f(1.0f, 1.0f);

        atra_gl_matrixmode(ATRA_PROJECTION);

        mat4 proj;
        glm_perspective(90.0, width / (float)height, 0.01, 1000.0, proj);
        atra_gl_multmatrix(proj);

    }
    atra_gl_end();
    atra_gl_batch_drawactive();

}

void
stop(void)
{

}

int
main(int argc, char **argv)
{
    int rand_num = atra_util_randomint(0, 1000000);

    ATRA_UNUSED(argc);
    ATRA_UNUSED(argv);

    atra_engine_start(1280, 720, "Atra Engine Test");
    atra_state_push(init, handle, update, render_mesh, stop);

    atra_window_setminsize(500, 400);
    atra_mouse_lock();

    camera = atra_camera_create(ATRA_CAMERA_PROJECTION, 1280, 720);
    
    //perlin_img = atra_image_genperlinnoise(1024, 1024, rand_num, rand_num, 1.0);
    //perlin_tex = atra_gl_tex_create(perlin_img.data, 1024, 1024, 1);

    build_mesh();

    while (atra_window_shouldclose() == 0) {
        atra_state_handle();
        atra_state_update();
        
        //atra_gl_batch_settexture(perlin_tex);
        atra_state_render();
    }
    atra_mesh_delete(&mesh);

    atra_engine_stop();

    return 0;
}
