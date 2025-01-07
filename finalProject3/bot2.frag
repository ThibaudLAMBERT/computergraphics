#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 textCoords;

out vec3 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform sampler2D textureSampler;


void main()
{
    // Lighting
    vec3 lightDir = lightPosition - worldPosition;
    float lightDist = dot(lightDir, lightDir);
    lightDir = normalize(lightDir);
    vec3 v = lightIntensity * clamp(dot(lightDir, worldNormal), 0.0, 1.0) / lightDist;

    // Tone mapping
    v = v / (1.0 + v);

    // Gamma correction
    finalColor = texture(textureSampler,textCoords).rgb * pow(v, vec3(1.0 / 2.2));
}
