#version 330 core

in vec3 color;
in vec2 uv;
in vec3 worldPosition;
in vec3 worldNormal;
in vec4 fragPos;

// TODO: To add UV input to this fragment shader

// TODO: To add the texture sampler
uniform sampler2D buildingSampler;
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 lightDirection;
uniform mat4 lightSpaceMatrix;
uniform sampler2D shadowMap;

out vec3 finalColor;

void main()
{
    vec3 normal = normalize(worldNormal);
    vec3 lightDir = normalize(-lightDirection);


    float cosTheta = max(dot(normal, lightDir), 0.0);

    vec3 irradiance = lightIntensity;

    vec3 diffuse = color * cosTheta * irradiance;

    vec3 colorAfterToneMapping = diffuse / (1 + diffuse);

    float gamma =  2.2;

    vec3 gammaCorrectedColor = pow(colorAfterToneMapping, vec3 (1/gamma));


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


    finalColor =  texture(buildingSampler, uv).rgb * gammaCorrectedColor * shadow;
    //finalColor = normalize(worldNormal) * 0.5 + 0.5; // [-1, 1] -> [0, 1]
    // TODO: texture lookup.
}
