#version 460

layout (location = 0) in vec2 texcoord;

layout (binding = 4) uniform sampler2D renderTextureSampler;

layout (location = 0) out vec4 outColor;

void main()
{
    vec3 albedo = texture(renderTextureSampler, texcoord).rgb;

    outColor = vec4(albedo, 1.0);
    // outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
