#version 330 core

const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float ambientStrength = 0.8;
const float diffuseStrength = 0.2;
const float specularStrength = 0.1;
const float shininess = 512.0;

// two colors to interpolate between
const vec3 lowColor  = vec3(15.0/255.0, 67.0/255.0, 146.0/255.0);
const vec3 highColor = vec3(1.0, 73.0/255.0, 73.0/255.0);

in vec3 vNormal;
in vec3 vLightDir;
in vec3 vViewDir;
in float vScalar;

out vec4 fColor;

void main() 
{
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(vLightDir);
    vec3 viewDir = normalize(vViewDir);

    vec3 ambient = ambientStrength * lightColor;

    float d = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * d * lightColor;

    float s = 0.0;
    if (d > 0.0)
    {
        vec3 reflectDir = normalize(reflect(-lightDir, normal));
        float cosphi = max(dot(viewDir, reflectDir), 0.0);
        if (cosphi > 0.0)
            s = pow(cosphi, shininess);
    }
    vec3 specular = specularStrength * s * lightColor;

    // interpolate between two colors based on vScalar
    vec3 fcolor = (lowColor * (1.0 - vScalar)) + (highColor * vScalar);

    // final fragment color
    fColor = vec4((ambient + diffuse + specular) * fcolor, 1.0);
}
