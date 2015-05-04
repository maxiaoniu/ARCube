#version 410
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 modelToWorld;
uniform mat4 worldToCamera;
uniform mat4 cameraToView;
out vec3 Normal;
out vec3 Position;
out mat4 view;
void main()
{
    view = worldToCamera * modelToWorld;
    gl_Position = cameraToView * worldToCamera * modelToWorld*vec4(position, 1.0);
    Normal = mat3(transpose(inverse(view))) * normal;
    Position = vec3(view * vec4(position, 1.0f));
}
