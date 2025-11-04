#version 330 core

uniform int numContours; // specifies the number of contours we want to darw
const vec3 contourColor = vec3(0,0,0);

const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float ambientStrength = 0.8;
const float diffuseStrength = 0.2;
const float specularStrength = 0.1;
const float shininess = 512.0;

in vec3 vNormal;
in vec3 vLightDir;
in vec3 vViewDir;
in float vScalar;

out vec4 fColor;

void main() 
{

    float spacing = 1.0 / float(numContours);
    int num_contours_below = int(vScalar / spacing);
    float contour_below = float(num_contours_below) * spacing;
    float dist_to_below = vScalar - contour_below;
    float dist_to_above = (contour_below + spacing) - vScalar;
    float alpha = (dist_to_below / spacing) - 0.5;
    alpha = clamp(alpha, 0.0, 1.0);

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
    
    // set the color of the fragment by the scalar value and lighting
    fColor = vec4((ambient + diffuse + specular) * contourColor , alpha);
}
