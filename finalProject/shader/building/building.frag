#version 330 core

// Input variables
in vec3 color;
in vec2 uv;
in vec3 worldPosition;
in vec3 worldNormal;
in vec4 fragPos;

// Uniform variables
uniform sampler2D buildingSampler;
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 lightDirection;
uniform mat4 lightSpaceMatrix;
uniform sampler2D shadowMap;
uniform vec3 cameraPosition;

// Output variable
out vec3 finalColor;

void main()
{
    // Diffuse lighting
    vec3 normal = normalize(worldNormal);
    vec3 lightDir = normalize(-lightDirection);
    float cosTheta = max(dot(normal, lightDir),0);
    vec3 irradiance = lightIntensity;
    vec3 diffuse = color * cosTheta * irradiance;

    // Ambient lighting
    vec3 ambientColor = 50 * vec3(0.01, 0.01, 0.01);

    // Combined lighting
    vec3 combinedColor = diffuse + ambientColor;
    vec3 colorAfterToneMapping = combinedColor / (1 + combinedColor);
    float gamma =  2.2;
    vec3 gammaCorrectedColor = pow(colorAfterToneMapping, vec3 (1/gamma));

    // Specular lighting
    vec3 viewDir = normalize(cameraPosition - fragPos.xyz);
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specularColor = vec3(0.1, 0.1, 0.1) * 0.2f;

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
    finalColor = (gammaCorrectedColor + spec * specularColor) * texture(buildingSampler, uv).rgb * shadow;

}
