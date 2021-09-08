#include "atra.h"

void
init (void)
{

}

void
handle(void)
{

}

void
update(u32 dt)
{

}

void
render(void)
{
    float width = atra_window_getwidth();
    float height = atra_window_getheight();

    atra_gl_clearscreenbuffers();

    atra_gl_loadidentity();

    atra_gl_begin(ATRA_QUADS);
    {
        Image perlin_img;
        uint perlin_tex;

        atra_gl_color4ub(0xFF, 0xFF, 0xFF, 0xFF);

        perlin_img = atra_image_genperlinnoise(512, 512, 0, 0, 1.0f);
        perlin_tex = atra_gl_tex_create(perlin_img.data, 512, 512, 1);
        atra_gl_batch_settexture(perlin_tex);
        
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
        atra_gl_frustum(-0.1f, 0.1f, -0.1f, 0.1f,
                ATRA_CAMERA_NEAR_PLANE,
                ATRA_CAMERA_FAR_PLANE);
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
    ATRA_UNUSED(argc);
    ATRA_UNUSED(argv);

    atra_engine_start(1280, 720, "Atra Engine Test");
    atra_state_push(init, handle, update, render, stop);

    atra_window_setminsize(500, 400);

    while (atra_window_shouldclose() == 0) {
        atra_state_handle();
        atra_state_update();
        atra_state_render();
    }
    atra_engine_stop();

    return 0;
}
