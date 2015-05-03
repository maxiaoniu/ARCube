#version 410

in vec3 Normal;
in vec3 Position;
out vec4 color;

//uniform vec3 cameraPos;
uniform samplerCube envTex;

void main()
{

    vec3 I = normalize(Position);
    vec3 R = reflect(I, normalize(Normal));
    color = texture(envTex, R);
}
