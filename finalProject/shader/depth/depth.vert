#version 330 core

// Input variable
layout(location = 0) in vec3 vertexPosition;

// Uniform variable
uniform mat4 lightSpaceMatrix;

void main() {
    // Transform vertex
    gl_Position =  lightSpaceMatrix * vec4(vertexPosition, 1);
}