#version 330 core

// Input
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

// TODO: To add UV to this vertex shader

// Matrix for vertex transformation
uniform mat4 MVP;
uniform mat4 lightSpaceMatrix;



void main() {

    //Creation of the model matrix
    mat4 model = mat4(1.0);
    model[3] = vec4(Position, 1.0); // Translation
    model[0][0] = model[0][0] * Scale.x;
    model[1][1] = model[1][1] * Scale.y;
    model[2][2] = model[2][2] * Scale.z;



    // Transform vertex
    gl_Position =  MVP * model * vec4(vertexPosition, 1);

    // Pass vertex color to the fragment shader
    color = vertexColor;

    // World-space geometry
    worldPosition = vertexPosition;
    worldNormal = vertexNormal;
    // Pass vertex color to the fragment shader



    // TODO: Pass UV to the fragment shader
    uv = vertexUV;

    fragPos = lightSpaceMatrix * model * vec4(vertexPosition, 1.0);
}
