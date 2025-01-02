#version 330 core

in vec3 color;
in vec2 uv;
in vec4 fragPos;

// TODO: To add UV input to this fragment shader

// TODO: To add the texture sampler



// TODO: To add UV input to this fragment shader

// TODO: To add the texture sampler
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 lightDirection;
uniform mat4 lightSpaceMatrix;
uniform sampler2D shadowMap;
uniform sampler2D SkyboxSampler;


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
    finalColor = color * texture(SkyboxSampler, uv).rgb;

    // TODO: texture lookup.
}
