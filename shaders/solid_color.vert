#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec3 viewPos;

const vec3 lightPos = vec3(2.0, 5.0, 0.0);

layout (location = 0) in vec3 glVertex;
layout (location = 1) in vec3 glNormal;

out vec3 vNormal;
out vec3 vLightDir;
out vec3 vViewDir;

void main() 
{
    mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
    mat4 mv = viewMatrix * modelMatrix;

    vec4 eye_coord_pos = viewMatrix * modelMatrix * vec4(glVertex, 1.0);
    vNormal = mat3(transpose(inverse(mv))) * glNormal;
    vLightDir = lightPos - eye_coord_pos.xyz;
    vViewDir = viewPos - eye_coord_pos.xyz;

    gl_Position = mvp * vec4(glVertex, 1.0);
}
