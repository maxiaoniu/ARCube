#version 410

in vec3 Normal;
in vec3 Position;
in vec3 uvposition;
out vec4 color;

//uniform vec3 cameraPos;
uniform sampler3D noise;



void main()
{
    float turbulence = 0.0;
    float scale =1;
    vec4 marbleColors1 = vec4(0.9, 0.9, 0.9, 1);
    vec4 marbleColors2 = vec4(0.6, 0.5, 0.5, 1);


    for (int i = 0; i < 4; ++i)
    {
        turbulence += scale * (texture(noise,0.125*uvposition.xyz/30).x - 0.5);
        scale *= 0.5;
    }

    vec3 N = normalize(Normal);
    // assume directional light

    float NdotL = dot(N, normalize(vec3(0,1,1)));
    float RdotL = dot(reflect(normalize(Position), N), normalize(vec3(0,1,1)));

    vec4 unlitColor = mix(marbleColors1, marbleColors2, exp(-4 * abs(turbulence)));
    color =  vec4(0.2,0.2,0.2,1)+max(NdotL, 0.0)* unlitColor + pow(max(RdotL, 0.1), 20);
}
