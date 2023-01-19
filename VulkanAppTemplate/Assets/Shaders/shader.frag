#version 460

layout (location = 0) in vec2 texcoord;
layout (location = 1) in vec3 fragColor;

layout (binding = 1) uniform sampler2D textureSampler;

layout (location = 0) out vec4 outColor;

void main()
{
    outColor = texture(textureSampler, texcoord);
}
