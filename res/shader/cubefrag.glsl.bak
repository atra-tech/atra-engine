#version 330

in vec2 uvout;
in vec3 pos_world;
in vec3 normal_camera;
in vec3 eyedir_camera;
in vec3 lightdir_camera;

out vec4 Color;

uniform sampler2D texture;
uniform mat4 modelmat;
uniform mat4 viewmat;

void main()
{
    vec3 lightpos_world = vec3(14.0, 14.0, 14.0);
    vec3 light_color = vec3(1.0, 0.5, 0.1);
    float light_pow = 100.0f;

    vec4 texdata = texture2D(texture, uvout);
    float texalpha = texdata.a;

    vec3 matdiffuse = texdata.rgb;
    vec3 matambient = vec3(0.3, 0.3, 0.3) * matdiffuse;
    vec3 matspecular = vec3(0.0, 0.0, 0.0);

    float light_dist = length(lightpos_world - pos_world);

    vec3 n = normalize(normal_camera);
    vec3 l = normalize(lightdir_camera);
    float costheta = clamp(dot(n, l), 0, 1);

    vec3 e = normalize(eyedir_camera);
    vec3 r = reflect(-l, n);
    float cosalpha = clamp(dot(e, r), 0, 1);

    Color = vec4(matambient +
            matdiffuse * light_color * light_pow * costheta / (light_dist * light_dist) +
            matspecular * light_color * light_pow * pow(cosalpha, 5) / (light_dist * light_dist), texalpha);
}
