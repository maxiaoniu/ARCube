#version 410
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 TexCoord;

uniform mat4 worldToCamera;
uniform mat4 cameraToView;

void main()
{
    gl_Position = cameraToView * worldToCamera * vec4(position, 1.0f);
    TexCoord = texCoord;
}
