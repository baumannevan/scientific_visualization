#version 330 core

const vec3 objectColor = vec3(1.0, 0.0, 0.0);

out vec4 FragColor;

void main() 
{
    FragColor = vec4(objectColor, 1.0);
}
