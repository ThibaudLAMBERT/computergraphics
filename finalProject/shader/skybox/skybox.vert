#version 330 core

// Input variables
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexUV;

// Output data, to be interpolated for each fragment
out vec3 color;
out vec2 uv;
out vec4 fragPos;

// Uniform variables
uniform mat4 MVP;
uniform mat4 lightSpaceMatrix;

void main() {
    // Transform vertex
    gl_Position =  MVP * vec4(vertexPosition, 1);

    // Pass vertex color to the fragment shader
    color = vertexColor;

    // Pass UV to the fragment shader
    uv = vertexUV;

    fragPos = lightSpaceMatrix * vec4(vertexPosition, 1.0);
}
