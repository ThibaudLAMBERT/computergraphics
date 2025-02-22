#version 330 core

// Input variables
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec2 vertexUV;
layout(location = 4) in vec3 Position;
layout(location = 5) in vec3 Scale;


// Output data, to be interpolated for each fragment
out vec3 worldPosition;
out vec3 worldNormal;
out vec4 fragPos;
out vec3 color;
out vec2 uv;


// Uniform variables
uniform mat4 MVP;
uniform mat4 lightSpaceMatrix;

void main() {

    // Transform vertex
    gl_Position =  MVP * vec4(vertexPosition, 1);

    // Pass vertex color to the fragment shader
    color = vertexColor;

    // World-space geometry
    worldPosition = vertexPosition;
    worldNormal = vertexNormal;

    // Pass UV to the fragment shader
    uv = vertexUV;

    fragPos = lightSpaceMatrix * vec4(vertexPosition, 1.0);
}
