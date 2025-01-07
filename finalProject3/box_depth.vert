#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 instancePosition;
layout(location = 2) in vec3 instanceScale;

uniform mat4 lightSpaceMatrix;

mat4 scaleMatrix(vec3 scale) {
    mat4 result = mat4(1.0);
    result[0][0] = scale.x;
    result[1][1] = scale.y;
    result[2][2] = scale.z;
    return result;
}


void main() {

    //Creation of the model matrix
    mat4 model = mat4(1.0);
    model[3] = vec4(instancePosition, 1.0); // Translation
    model *= scaleMatrix(instanceScale);


    gl_Position =  lightSpaceMatrix * model * vec4(vertexPosition, 1);
}