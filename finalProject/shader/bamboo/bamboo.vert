#version 330 core

// Input variables
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 6) in vec3 Position;
layout(location = 7) in vec3 Scale;

// Output data, to be interpolated for each fragment
out vec3 worldPosition;
out vec3 worldNormal;
out vec2 textCoords;

// Uniform variable
uniform mat4 MVP;

void main() {

    // Creationf od model matrix
    mat4 model = mat4(1.0);
    model[3] = vec4(Position, 1.0); // Translation
    model[0][0] = model[0][0] * Scale.x;
    model[1][1] = model[1][1] * Scale.y;
    model[2][2] = model[2][2] * Scale.z;

    // Transform vertex
    gl_Position =  MVP * model * vec4(vertexPosition, 1.0);

    // World-space geometry
    worldPosition = vertexPosition;
    worldNormal = vertexNormal;
    textCoords = vertexUV;
}
