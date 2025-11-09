#version 330 core

const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float ambientStrength = 0.8;
const float diffuseStrength = 0.2;
const float specularStrength = 0.1;
const float shininess = 512.0;
const int licSteps = 12;
const float licStepSize = 0.002;

uniform sampler2D noiseTexture;
uniform sampler2D imageTexture;
uniform int useTexture;


in vec3 vNormal;
in vec3 vLightDir;
in vec3 vViewDir;
in float vScalar;
in vec2 vVector;
in vec2 vTexCoord;

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
    if(d > 0.0) // only use specular if the normal is facing the eye position
    {
        vec3 reflectDir = normalize(reflect(-lightDir, normal));
        float cosphi = max(dot(viewDir, reflectDir),0.0);
        if (cosphi > 0.0)
            s = pow(cosphi, shininess);
    }
    vec3 specular = specularStrength * s * lightColor;
    
    vec3 color;
    if (useTexture == 0)
        color = texture(noiseTexture, vTexCoord).rrr; // noise texture
    else
        color = texture(imageTexture, vTexCoord).rgb; // image texture
    
    int num_steps = int(licSteps * vScalar) + 3;

    // blend the texture color in the forward direction
    vec2 tcoord = vTexCoord;
    for (int i = 0; i < num_steps; i++)
    {
        tcoord += vVector * licStepSize;
        if (useTexture == 0){
            color += texture(noiseTexture, tcoord).rrr;
        }
        else {
            color += texture(imageTexture, tcoord).rgb;
        }
    }

    // blend the texture color in the backward direction
    tcoord = vTexCoord;
    for (int i = 0; i < num_steps; i++)
    {
        tcoord -= vVector * licStepSize;
        if (useTexture == 0){
            color += texture(noiseTexture, tcoord).rrr;
        }
        else {
            color += texture(imageTexture, tcoord).rgb;
        }
    }

    // divide the accumulated color by the number of samples taken
    color /= float(2*num_steps+1);

    fColor = vec4((ambient + diffuse + specular) * color, 1.0);
}
