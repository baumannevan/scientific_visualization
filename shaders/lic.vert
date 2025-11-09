#version 330 core

// uniform variables are passed in from the application, and are the same for each vertex
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec3 viewPos;
uniform float minScalar;
uniform float maxScalar;
uniform float minX, maxX, minY, maxY;

// const variables are local to the shader and cannot be changed by the application
const vec3 lightPos = vec3(2.0, 5.0, 0.0);

// in variables are per-vertex attributes that are passed in the from the application
layout (location = 0) in vec3 glVertex;
layout (location = 1) in vec3 glNormal;
layout (location = 2) in float glScalar;
layout (location = 3) in vec3 glVector;

// out variables are interpolated and passed ot the fragment shader
out vec3 vNormal;
out vec3 vLightDir;
out vec3 vViewDir;
out float vScalar;
out vec2 vVector;
out vec2 vTexCoord;


void main() 
{
    // model-view-projection matrix and model-view matrix tell us how to transform
    // the vertices so they are in the right place on the screen
    mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
    mat4 mv = viewMatrix * modelMatrix;
    
    // eye_coord_pos is the vertex position relative to the camera position and view direction
    // this is needed accuratly compute light reflections that reach the camera
    vec4 eye_coord_pos = viewMatrix * modelMatrix * vec4(glVertex, 1.0);
    vNormal = mat3(transpose(inverse(mv))) * glNormal;
    vLightDir = lightPos - eye_coord_pos.xyz;
    vViewDir = viewPos - eye_coord_pos.xyz;

    // gl_Position is a built-in mandatory output variable that holds the transformed vertex position
    gl_Position = mvp * vec4(glVertex, 1.0);

    vVector = normalize(glVector.xy);
    
    vScalar = (glScalar - minScalar) / (maxScalar - minScalar);
    vScalar = clamp(vScalar, 0.0, 1.0);


    float tx = (maxX > minX) ? (glVertex.x - minX) / (maxX - minX) : 0.0;
    float ty = (maxY > minY) ? (glVertex.y - minY) / (maxY - minY) : 0.0;
    vTexCoord = clamp(vec2(tx, ty), 0.0, 1.0);
}
