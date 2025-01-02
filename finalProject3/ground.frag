#version 330 core

in vec4 fragPos;
in vec3 color;

uniform mat4 lightSpaceMatrix;
uniform sampler2D shadowMap;



out vec3 finalColor;

void main()
{
    vec4 shadowCoord = fragPos/ fragPos.w;
    shadowCoord = shadowCoord * 0.5 + 0.5;

    float shadow;

    if (shadowCoord.x < 0 || shadowCoord.x > 1 || shadowCoord.y < 0 || shadowCoord.y > 1 || shadowCoord.z < 0 || shadowCoord.z > 1) {
        shadow = 1.0;
    } else {
        float depth = shadowCoord.z;


        float existingDepth = texture(shadowMap, shadowCoord.xy).r;

        shadow = (depth >= existingDepth + 1e-3) ? 0.2 : 1.0;
    }
    finalColor = color * shadow;

    // TODO: texture lookup.
}
