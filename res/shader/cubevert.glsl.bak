#version 330

layout (location = 0) in vec3 verts;
layout (location = 1) in vec2 uvs;
layout (location = 2) in vec3 norms;

uniform mat4 projviewmat;
uniform mat4 viewmat;
uniform mat4 modelmat;

out vec2 uvout;
out vec3 pos_world;
out vec3 normal_camera;
out vec3 eyedir_camera;
out vec3 lightdir_camera;

void main()
{
    vec3 lightpos_world = vec3(14.0, 14.0, 14.0);
    gl_Position = projviewmat * modelmat * vec4(verts, 1.0);

    pos_world = (modelmat * vec4(verts, 1.0)).xyz;

    //Vertex to camera vector (camera space)
    vec3 vertexpos_camera = (viewmat * modelmat * vec4(verts, 1.0)).xyz;
    eyedir_camera = vec3(0, 0, 0) - vertexpos_camera;

    //Vertex to light vector (camera space)
    vec3 lightpos_camera = (viewmat * vec4(lightpos_world, 1.0)).xyz;
    lightdir_camera = lightpos_camera + eyedir_camera;

    normal_camera = (viewmat * modelmat * vec4(norms, 0.0)).xyz;

    uvout = uvs;
}
