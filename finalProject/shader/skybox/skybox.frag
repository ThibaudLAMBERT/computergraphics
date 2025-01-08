#version 330 core

// Input variables
in vec3 color;
in vec2 uv;
in vec4 fragPos;

// Uniform variables
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 lightDirection;
uniform mat4 lightSpaceMatrix;
uniform sampler2D shadowMap;
uniform sampler2D SkyboxSampler;

// Output variable
out vec3 finalColor;

void main()
{
    // Shadow mapping
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

    // Final color
    finalColor = color * texture(SkyboxSampler, uv).rgb * shadow;

}
