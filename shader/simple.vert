#version 410
layout(location = 0) in vec3 position;
//layout(location = 1) in vec3 normal;

uniform mat4 modelToWorld;
uniform mat4 worldToCamera;
uniform mat4 cameraToView;

void main()
{
    gl_Position = cameraToView * worldToCamera * modelToWorld*vec4(position, 1.0);
    //vColor = basicColor;
}
