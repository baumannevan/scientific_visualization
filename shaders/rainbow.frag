#version 330 core

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

vec3 hsv2rgb(float h, float s, float v) {
    h = mod(h, 360.0);  // keep hue in [0, 360)
    float c = v * s;
    float h_prime = h / 60.0;
    float x = c * (1.0 - abs(mod(h_prime, 2.0) - 1.0));

    float r, g, b;
    if (0.0 <= h_prime && h_prime < 1.0) { r = c; g = x; b = 0.0; }
    else if (1.0 <= h_prime && h_prime < 2.0) { r = x; g = c; b = 0.0; }
    else if (2.0 <= h_prime && h_prime < 3.0) { r = 0.0; g = c; b = x; }
    else if (3.0 <= h_prime && h_prime < 4.0) { r = 0.0; g = x; b = c; }
    else if (4.0 <= h_prime && h_prime < 5.0) { r = x; g = 0.0; b = c; }
    else { r = c; g = 0.0; b = x; }

    float m = v - c;
    return vec3(r + m, g + m, b + m);
}

void main() {
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(vLightDir);
    vec3 viewDir = normalize(vViewDir);

    vec3 ambient = ambientStrength * lightColor;

    float d = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * d * lightColor;

    float s = 0.0;
    if (d > 0.0) {
        vec3 reflectDir = normalize(reflect(-lightDir, normal));
        float cosphi = max(dot(viewDir, reflectDir), 0.0);
        if (cosphi > 0.0)
            s = pow(cosphi, shininess);
    }
    vec3 specular = specularStrength * s * lightColor;

    float hue = (1.0 - vScalar) * 270.0; // 0–270° rainbow range
    vec3 color = hsv2rgb(hue, 1.0, 1.0);

    fColor = vec4((ambient + diffuse + specular) * color, 1.0);
}
