#version 410

in vec3 Normal;
in vec3 Position;
out vec4 color;

//uniform vec3 cameraPos;
uniform samplerCube envTex;

void main()
{

    vec3 N = normalize(Normal);
    vec3 R = 2.0 * dot(-Position, N) * N + Position;
    color = texture(envTex, R);
}
