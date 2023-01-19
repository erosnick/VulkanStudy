#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;
layout (location = 3) in vec3 inColor;

layout (binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout (location = 0) out vec3 normal;
layout (location = 1) out vec2 texcoord;
layout (location = 2) out vec3 fragColor;

void main()
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);
    normal = (ubo.model * vec4(inNormal, 0.0)).xyz;
    texcoord = inTexcoord;
    fragColor = inColor;
}