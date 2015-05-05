#version 410
in vec2 TexCoord;

//out vec4 color;
layout (location = 0) out vec4 color;
uniform sampler2D ourTexture;

void main()
{
    color = texture(ourTexture, TexCoord);
    //color =  vec4(0.5,0.5,0.5,1);
}
