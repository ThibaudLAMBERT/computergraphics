#version 330 core

// Input variables
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec4 vertexJoint;
layout(location = 4) in vec4 vertexWeight;

// Output data, to be interpolated for each fragment
out vec3 worldPosition;
out vec3 worldNormal;
out vec2 textCoords;

// Uniform variables
uniform mat4 MVP;
uniform mat4 jointMatrices[200];

void main() {
    // Calculate the skinning matrix
    mat4 skinMat = vertexWeight.x  * jointMatrices[int(vertexJoint.x)] +
    vertexWeight.y * jointMatrices[int(vertexJoint.y)]  +
    vertexWeight.z * jointMatrices[int(vertexJoint.z)] +
    vertexWeight.w * jointMatrices[int(vertexJoint.w)];

    // Apply the skinning matrix to the vertex position to calculate the new skinned position
    vec4 skinPosition = skinMat * vec4(vertexPosition, 1.0);

    // Transform the skinned vertex position
    gl_Position =  MVP * skinPosition;

    // World-space geometry
    vec3 skinnedNormal = normalize((mat3(skinMat) * vertexNormal));
    worldNormal = skinnedNormal;
    worldPosition = vec3(skinPosition);
    textCoords = vertexUV;
}
