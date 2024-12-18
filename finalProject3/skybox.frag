#version 330 core

in vec3 color;
in vec2 uv;

// TODO: To add UV input to this fragment shader

// TODO: To add the texture sampler
uniform sampler2D SkyboxSampler;

out vec3 finalColor;

void main()
{
    finalColor = color * texture(SkyboxSampler, uv).rgb;

    // TODO: texture lookup.
}
