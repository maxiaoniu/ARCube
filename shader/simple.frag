#version 410

in vec3 Normal;
in vec3 Position;
in mat4 view;
out vec4 color;

//uniform vec3 cameraPos;
uniform samplerCube envTex;
float coeffs(int i)
{
        return 1.0 / (3.0 + 0.1 * float(i));
}
void main()
{

 //   vec3 I = normalize(Position);
//    vec3 R = reflect(I, normalize(Normal));
//    color = texture(envTex, R);

    vec3 N = normalize(Normal);
    vec3 I = -normalize(Position);
    mat3 V = mat3(view[0].xyz, view[1].xyz, view[2].xyz);
    float IdotN = dot(I, N);
    float scales[6];
    vec3 C[6];
    for (int i = 0; i < 6; ++i) {
        scales[i] = (IdotN - sqrt(1.0 - coeffs(i) + coeffs(i) * (IdotN * IdotN)));
        C[i] = texture(envTex, (-I + coeffs(i) * N) * V).xyz;
    }
    vec4 refractedColor = 0.25 * vec4(C[5].x + 2.0*C[0].x + C[1].x, C[1].y + 2.0*C[2].y + C[3].y,
                          C[3].z + 2.0*C[4].z + C[5].z, 4.0);

    vec3 R = 2.0 * dot(-Position, N) * N + Position;
    vec4 reflectedColor = texture(envTex, R * V);

    color = mix(refractedColor, reflectedColor, 0.4 + 0.6 * pow(1.0 - IdotN, 2.0));

}
