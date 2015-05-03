#version 410

in vec2 TexCoords;

layout (location = 0) out vec4 outputF;

uniform sampler2D screenTexture;

void main()
{
    outputF = texture(screenTexture, TexCoords);
}
