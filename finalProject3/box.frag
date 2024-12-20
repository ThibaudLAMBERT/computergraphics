#version 330 core

in vec3 color;
in vec2 uv;
in vec3 worldPosition;
in vec3 worldNormal;
//in vec4 fragPos;

// TODO: To add UV input to this fragment shader

// TODO: To add the texture sampler
uniform sampler2D buildingSampler;
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 lightDirecction;
//uniform mat4 lightSpaceMatrix;
//uniform sampler2D shadowMap;

out vec3 finalColor;

void main()
{
    vec3 lightDir = normalize(lightPosition - worldPosition);


    float distance = length(lightPosition - worldPosition);

    float cosTheta = dot(normalize(worldNormal), lightDir);


    vec3 irradiance = lightIntensity / (4.0 * 3.14159 * (distance/1000) * (distance/1000));

    vec3 diffuse = (color / 3.14159) * cosTheta * irradiance;

    vec3 colorAfterToneMapping = diffuse / (1 + diffuse);

    float gamma =  2.8;

    vec3 gammaCorrectedColor = pow(colorAfterToneMapping, vec3 (1/gamma));


    finalColor = texture(buildingSampler, uv).rgb * gammaCorrectedColor ;
    //finalColor = gammaCorrectedColor * shadow * color * texture(buildingSampler, uv).rgb;

    // TODO: texture lookup.
}
