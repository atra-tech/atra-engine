#include "atra.h"

static const char *vshader_code =
    "#version 120                       \n"

    "#define MAX_BONES 32               \n"

    "attribute vec3 vert_pos;           \n"
    "attribute vec2 vert_uv;            \n"
    "attribute vec3 vert_norm;          \n"
    "attribute vec4 vert_color;         \n"

    "attribute vec4 vert_boneids;       \n"
    "attribute vec4 vert_boneweights;   \n"


    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "varying vec3 fragNormal;           \n"
    "varying vec3 fragPos;              \n"

    "uniform mat4 bone_mats[MAX_BONES]; \n"

    "uniform mat4 viewmat;              \n"
    "uniform mat4 projmat;              \n"
    "uniform mat4 modelmat;             \n"
    "uniform mat4 mvp;                  \n"

    "mat4 inverse(mat4 m) {"
      "float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3];"
      "float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3];"
      "float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3];"
      "float a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3];"

      "float b00 = a00 * a11 - a01 * a10;"
      "float b01 = a00 * a12 - a02 * a10;"
      "float b02 = a00 * a13 - a03 * a10;"
      "float b03 = a01 * a12 - a02 * a11;"
      "float b04 = a01 * a13 - a03 * a11;"
      "float b05 = a02 * a13 - a03 * a12;"
      "float b06 = a20 * a31 - a21 * a30;"
      "float b07 = a20 * a32 - a22 * a30;"
      "float b08 = a20 * a33 - a23 * a30;"
      "float b09 = a21 * a32 - a22 * a31;"
      "float b10 = a21 * a33 - a23 * a31;"
      "float b11 = a22 * a33 - a23 * a32;"

      "float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;"

    "return mat4("
      "a11 * b11 - a12 * b10 + a13 * b09,"
      "a02 * b10 - a01 * b11 - a03 * b09,"
      "a31 * b05 - a32 * b04 + a33 * b03,"
      "a22 * b04 - a21 * b05 - a23 * b03,"
      "a12 * b08 - a10 * b11 - a13 * b07,"
      "a00 * b11 - a02 * b08 + a03 * b07,"
      "a32 * b02 - a30 * b05 - a33 * b01,"
      "a20 * b05 - a22 * b02 + a23 * b01,"
      "a10 * b10 - a11 * b08 + a13 * b06,"
      "a01 * b08 - a00 * b10 - a03 * b06,"
      "a30 * b04 - a31 * b02 + a33 * b00,"
      "a21 * b02 - a20 * b04 - a23 * b00,"
      "a11 * b07 - a10 * b09 - a12 * b06,"
      "a00 * b09 - a01 * b07 + a02 * b06,"
      "a31 * b01 - a30 * b03 - a32 * b00,"
      "a20 * b03 - a21 * b01 + a22 * b00) / det;"
    "}\n"

    "mat4 bone_transform() { \n"
        "mat4 ret; \n"
        "float norm_fac = 1.0 / (vert_boneweights.x + vert_boneweights.y + vert_boneweights.z + vert_boneweights.w); \n"
        "ret = norm_fac * vert_boneweights.w * bone_mats[int(vert_boneids.w)]"
        "    + norm_fac * vert_boneweights.z * bone_mats[int(vert_boneids.z)]"
        "    + norm_fac * vert_boneweights.y * bone_mats[int(vert_boneids.y)]"
        "    + norm_fac * vert_boneweights.x * bone_mats[int(vert_boneids.x)]; \n"
        "return ret; \n"
    "}\n"

    "const vec2 res = vec2(320.0, 240.0); \n"

    "void main()                        \n"
    "{                                  \n"
    "    mat4 bone_tf = bone_transform();"
    "    vec4 pos = bone_tf * vec4(vert_pos, 1.0);"
    "    vec3 norm = mat3(transpose(inverse(bone_tf))) * vert_norm;"

    "    fragTexCoord = vert_uv;        "
    "    fragColor = vert_color;        "
    "    fragNormal = mat3(transpose(inverse(modelmat))) * norm;"
    "    fragPos = vec3(modelmat * pos);   "

    "    vec4 snap_pix = mvp * pos;                  "
    "    vec4 clip_pos = snap_pix;                              "
    "    clip_pos.xyz = snap_pix.xyz / snap_pix.w;              "
    "    clip_pos.x = floor(res.x * clip_pos.x) / res.x;        "
    "    clip_pos.y = floor(res.y * clip_pos.y) / res.y;        "
    "    clip_pos.xyz = clip_pos.xyz * snap_pix.w;              "

    "    gl_Position = mvp * pos;                 "
    //"    gl_Position = clip_pos;"
    "}                                  \n";

static const char *fshader_code =
    "#version 120                       \n"

    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "varying vec3 fragNormal;           \n"
    "varying vec3 fragPos;              \n"

    "uniform sampler2D albedo_map;      \n"
    "uniform vec4 color;                \n"
    "uniform vec3 viewPos;              \n"

    "const vec3 lightPos = vec3(20.0, 5.0, -5.0);     \n"
    "const vec3 lightColor = vec3(1.0, 0.8, 0.4);   \n"
    

    "void main()                        \n"
    "{                                  \n"
    "   /* Ambient */                                                   \n"
    "   float ambient_str = 0.2;                                        \n"
    "   vec3 ambient = ambient_str * lightColor;                        \n"
    
    "   /* Diffuse */                                                   \n"
    "   vec3 norm = normalize(fragNormal);                              \n"
    "   vec3 lightDir = normalize(lightPos - fragPos);                  \n"
    "   float diff = max(dot(norm, lightDir), 0.0);                     \n"
    "   vec3 diffuse = diff * lightColor;                               \n"
    
    "   /* Specular */                                                  \n"
    "   float specular_str = 0.5;                                       \n"
    "   vec3 viewDir = normalize(viewPos - fragPos);                    \n"
    "   vec3 reflectDir = reflect(-lightDir, norm);                     \n"
    "   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);       \n"
    "   vec3 specular = specular_str * spec * lightColor;               \n"

    "   vec4 texelColor = texture2D(albedo_map, fragTexCoord);          \n"
    "   gl_FragColor = texelColor * color * fragColor * vec4(ambient + diffuse + specular, 1.0);    \n"
    "}                                                                  \n";

#define INST_COUNT 16

static Material material;
static Mesh mesh;

static Camera camera;
static Camera camera2D;

static Model jerry;
static ModelAnimRaw *dance_anims;
static int anim_count;

static float times[INST_COUNT];
static float modelmats[INST_COUNT * 16];

static GUIWindow *gui_window;
static GUIElem *gui_elem_base;
static GUIElem *gui_elem_buttons[4];
static GUIElem *gui_elem_text;
static GUIElem *gui_elem_field;


void
init(void)
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

    //atra_gui_handle();
}

void
update(u32 dt)
{
    const float player_speed = 0.002f;
    const float speed = dt * player_speed * (1 + atra_key_isdown(ATRA_KEY_LSHIFT));
    //const vec3 speed_vec = {speed, speed, speed};

    vec3 velocity = {0.0f, 0.0f, 0.0f};

    if (atra_key_isdown(ATRA_KEY_W)) {
        glm_vec3_muladds(camera.dir, speed, velocity);
    }
    if (atra_key_isdown(ATRA_KEY_S)) {
        glm_vec3_muladds(camera.dir, -speed, velocity);
    }
    if (atra_key_isdown(ATRA_KEY_D)) {
        glm_vec3_muladds(camera.right, speed, velocity);
    }
    if (atra_key_isdown(ATRA_KEY_A)) {
        glm_vec3_muladds(camera.right, -speed, velocity);
    }
    glm_vec3_add(velocity, camera.pos, camera.pos);

    atra_camera_update(&camera);

    //atra_gui_update(dt);
}

void
init_gui(void)
{
    static char sample_text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.";

    GUIQuad base_quad;
    GUIQuad button_quad;
    GUIQuad text_quad;
    GUIQuad field_quad;

    base_quad.types[0] = ATRA_GUI_CONS_PIXEL;
    base_quad.types[1] = ATRA_GUI_CONS_PERCENT;
    base_quad.types[2] = ATRA_GUI_CONS_PERCENT;
    base_quad.types[3] = ATRA_GUI_CONS_PERCENT;

    button_quad.types[0] = ATRA_GUI_CONS_PERCENT;
    button_quad.types[1] = ATRA_GUI_CONS_PERCENT;
    button_quad.types[2] = ATRA_GUI_CONS_PERCENT;
    button_quad.types[3] = ATRA_GUI_CONS_PERCENT;

    text_quad.types[0] = ATRA_GUI_CONS_PERCENT;
    text_quad.types[1] = ATRA_GUI_CONS_PERCENT;
    text_quad.types[2] = ATRA_GUI_CONS_PERCENT;
    text_quad.types[3] = ATRA_GUI_CONS_PERCENT;

    field_quad.types[0] = ATRA_GUI_CONS_PERCENT;
    field_quad.types[1] = ATRA_GUI_CONS_PERCENT;
    field_quad.types[2] = ATRA_GUI_CONS_PERCENT;
    field_quad.types[3] = ATRA_GUI_CONS_PIXEL;


    base_quad.x = 32;
    base_quad.y = 0.025f;
    base_quad.w = 0.30f;
    base_quad.h = 0.95f;

    gui_window = atra_gui_window_create(0, 0, "test_window");
    gui_elem_base = atra_gui_elem_create("test_elem",
            ATRA_GUI_ELEM_SCISSORTEST |
            ATRA_GUI_ELEM_HASBORDER |
            ATRA_GUI_ELEM_KEEPRATIO_H,
            base_quad);

    button_quad.x = 0.08f;
    button_quad.y = 0.025f;
    button_quad.w = 0.15f;
    button_quad.h = 0.15f;

    gui_elem_buttons[0] = atra_gui_elem_create_button("button_0",
            ATRA_GUI_ELEM_CLICKABLE |
            ATRA_GUI_ELEM_HASBORDER |
            ATRA_GUI_ELEM_KEEPRATIO_W,
            button_quad);

    button_quad.x += 0.15f + 0.08;

    gui_elem_buttons[1] = atra_gui_elem_create_button("button_1",
            ATRA_GUI_ELEM_CLICKABLE |
            ATRA_GUI_ELEM_HASBORDER |
            ATRA_GUI_ELEM_KEEPRATIO_W,
            button_quad);

    button_quad.x += 0.15f + 0.08;

    gui_elem_buttons[2] = atra_gui_elem_create_button("button_2",
            ATRA_GUI_ELEM_CLICKABLE |
            ATRA_GUI_ELEM_HASBORDER |
            ATRA_GUI_ELEM_KEEPRATIO_W,
            button_quad);

    button_quad.x += 0.15f + 0.08;

    gui_elem_buttons[3] = atra_gui_elem_create_button("button_3",
            ATRA_GUI_ELEM_CLICKABLE |
            ATRA_GUI_ELEM_HASBORDER |
            ATRA_GUI_ELEM_KEEPRATIO_W,
            button_quad);

    text_quad.x = 0.08f;
    text_quad.y = 0.20f;
    text_quad.w = 0.84f;
    text_quad.h = 0.775f;

    gui_elem_text = atra_gui_elem_create_textbox("text_box",
            ATRA_GUI_ELEM_HASBORDER |
            ATRA_GUI_ELEM_SCISSORTEST |
            ATRA_GUI_ELEM_SCROLLABLE,
            text_quad);
    gui_elem_text->smooth_scroll = 1;

    field_quad.x = 0.08f;
    field_quad.y = 0.10f;
    field_quad.w = 0.84f;
    field_quad.h = 32;

    gui_elem_field = atra_gui_elem_create_textfield("text_field",
            ATRA_GUI_ELEM_HASBORDER, field_quad);

    atra_gui_elem_settext(gui_elem_text, sample_text);
    atra_gui_elem_settext(gui_elem_field, "Enter text...");
    atra_gui_elem_settextalign(gui_elem_text, ATRA_GUI_TEXTALIGN_CENTRE | ATRA_GUI_TEXTALIGN_MIDDLE);
    atra_gui_elem_settextalign(gui_elem_field, ATRA_GUI_TEXTALIGN_MIDDLE);

    atra_gui_window_setelem(gui_window, gui_elem_base);
    atra_gui_elem_addelem(gui_elem_base, gui_elem_buttons[0]);
    atra_gui_elem_addelem(gui_elem_base, gui_elem_buttons[1]);
    atra_gui_elem_addelem(gui_elem_base, gui_elem_buttons[2]);
    atra_gui_elem_addelem(gui_elem_base, gui_elem_buttons[3]);
    atra_gui_elem_addelem(gui_elem_base, gui_elem_text);
    atra_gui_elem_addelem(gui_elem_base, gui_elem_field);
}

void
delete_gui(void)
{

}

void
build_mesh(void)
{
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
render(void)
{
    static u32 tick = 0;
    int width, height;

    width = atra_window_getwidth();
    height = atra_window_getheight();

    tick += atra_time_getdelta();

    //atra_gl_clearscreenbuffers();
    atra_gl_loadidentity();
    atra_gl_batch_settexture(material.maps[0].tex.id);

    atra_gl_begin(ATRA_QUADS);
    {
        atra_gl_matrixmode(ATRA_MODELVIEW);
        atra_gl_loadidentity();

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

        atra_gl_translate(0.6f, 0.6f, -1.5f);
        atra_gl_rotate(DEG_TO_RAD(tick / 10.0), 0.0, 1.0, 0.0);
        atra_gl_scale(0.5, 0.5, 0.5);
        atra_gl_translate(-0.6f, -0.6f, 1.5f);

        atra_gl_matrixmode(ATRA_PROJECTION);

        atra_gl_loadidentity();

        mat4 proj;
        glm_perspective(DEG_TO_RAD(90.0f), width / (float)height, 0.01, 1000.0, proj);
        atra_gl_multmatrix(proj);

    }
    atra_gl_end();

    atra_gl_disabledepthtest();
    atra_gl_batch_drawactive();
    atra_gl_enabledepthtest();

}

void
render_mesh(void)
{
    mat4 transform;

    glm_mat4_identity(transform);

    atra_mesh_draw(atra_mesh_getactive(), &material, transform);
}

void
load_jerry(void)
{
    const uint row_size = sqrt(INST_COUNT);

    int i, x, z;
    vec4 *modelmat;

    jerry = atra_model_loadfile("res/model/jerry.glb", 1);
    dance_anims = atra_rawanim_loadfile("res/model/jerry.glb", &anim_count);

    for (i = 0; i < INST_COUNT; i++) {
        x = i % row_size;
        z = i / row_size;
        modelmat = (vec4 *)(modelmats + i * 16);
        
        glm_mat4_identity(modelmat);
        glm_translate_x(modelmat, x * 6);
        glm_translate_z(modelmat, z * 6);
    }
}

void
draw_jerry(void)
{
    vec3 pos = {0.0f, 0.0f, 0.0f};
    float scale = 1.0f;
    Color tint = {0xFF, 0xFF, 0xFF, 0xFF};
    uint i;


    //render_mesh();
    //render();

    //atra_gl_loadidentity();
    //atra_gl_matrix_setmodelview(camera.view);
    //atra_gl_matrix_setprojection(camera.proj);
    
    atra_gl_clearscreenbuffers();

    atra_gl_begin3D(&camera);
    {
        atra_shader_setactive(&material.shader);
        atra_shader_setuniform(atra_shader_getuniformloc(&material.shader, "viewPos"),
                camera.pos, ATRA_SHADER_ATTRIB_VEC3, 1);

        //atra_anim_updatemodel(&dance_anims[0], &jerry, frame);
        for (i = 0; i < INST_COUNT; i++) {
            glm_mat4_copy((vec4 *)(modelmats + i * 16), jerry.transform);

            atra_rawanim_updatemodelext(&dance_anims[0], &jerry, times[i]);
            atra_model_draw(&jerry, pos, scale, tint);
            times[i] += (atra_time_getdelta() / 1000.0) * ((i + 1) / 256.0);
        }
        //atra_model_draw(&jerry, pos, scale, tint);
    }
    atra_gl_end3D();

    //draw_text();
    /*mat4 view = GLM_MAT4_IDENTITY_INIT;
    vec3 view_scale = {1.0 / camera.width, -1.0 / camera.height, 0};
    glm_scale(view, view_scale);
    glm_translate_z(view, -0.1);
    atra_gl_matrix_setmodelview(view);*/

    /*atra_gl_disabledepthtest();
    atra_gui_render();
    atra_gl_enabledepthtest();*/
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

    Image perlin_img;
    int rand_num;

    atra_engine_start(1280, 720, "Atra Engine Test");
    atra_state_push(init, handle, update, draw_jerry, stop);

    atra_window_setminsize(500, 400);
    //atra_mouse_lock();

    init_gui();
    
    rand_num = atra_util_randomint(0, 1000000);
    perlin_img = atra_image_genperlinnoise(512, 512, rand_num, rand_num, 1.0);
    material.maps[0].tex = atra_texture_loadimage(&perlin_img);
    material.maps[0].color = (Color){0xFF, 0xFF, 0xFF, 0xFF};
    material.shader = atra_shader_loadbuffer(vshader_code, fshader_code);
    atra_image_destroy(&perlin_img);

    camera = atra_camera_create(ATRA_CAMERA_PROJECTION, 1280, 720);
    camera2D = atra_camera_create(ATRA_CAMERA_ORTHOGRAPHIC, 1280, 720);
    //font = atra_font_loadfile("res/font/Avara.ttf", 64, NULL, ATRA_FONT_DEFAULT_GLYPHCOUNT);
    
    //build_mesh();

    load_jerry();
    jerry.material_count = 1;
    jerry.materials[0] = material;

    while (atra_window_shouldclose() == 0) {
        atra_state_handle();
        atra_state_update();
        atra_state_render();
    }
    atra_model_delete(&jerry);
    atra_rawanim_deletearray(dance_anims, anim_count);
    //atra_font_delete(&font);

    //atra_mesh_delete(&mesh);
    atra_engine_stop();

    return 0;
}
