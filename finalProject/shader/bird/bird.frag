#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 textCoords;

out vec3 finalColor;

uniform vec3 lightPosition2;
uniform vec3 lightIntensity2;
uniform sampler2D textureSampler;


void main()
{
    // Lighting
    vec3 lightDir = lightPosition2 - worldPosition;
    float lightDist = dot(lightDir, lightDir);
    lightDir = normalize(lightDir);
    vec3 v = lightIntensity2 * clamp(dot(lightDir, worldNormal), 0.0, 1.0) / lightDist;

    // Tone mapping
    v = v / (1.0 + v);

    // Gamma correction
    finalColor = texture(textureSampler, textCoords).rgb;
}
