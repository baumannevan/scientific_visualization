#version 330 core

const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 objectColor = vec3(0.6, 0.6, 0.6);
const float ambientStrength = 0.1;
const float diffuseStrength = 1.0;
const float specularStrength = 0.1;
const float shininess = 512.0;

in vec3 vNormal;
in vec3 vLightDir;
in vec3 vViewDir;

out vec4 FragColor;

void main() 
{
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(vLightDir);
    vec3 viewDir = normalize(vViewDir);

    vec3 ambient = ambientStrength * lightColor;

    float d = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * d * lightColor;

    float s = 0.0;
    if(d > 0.0) // only use specular if the normal is facing the eye position
    {
        vec3 reflectDir = normalize(reflect(-lightDir, normal));
        float cosphi = max(dot(viewDir, reflectDir),0.0);
        if (cosphi > 0.0)
            s = pow(cosphi, shininess);
    }
    vec3 specular = specularStrength * s * lightColor;

    FragColor = vec4((ambient + diffuse + specular) * objectColor, 1.0);
}
