#version 450
#extension GL_ARB_separate_shader_objects :enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
}ubo;

layout(binding = 2) uniform LightDataBuffer
{
    vec3 lightPosition;
}lightDataBuffer;

layout(binding = 3) uniform DynamicUniformBuffer
{
    mat4 model;
    float furLength;
    float layer;
    float gravity;
    int layerIndex;
    float time;
}dynamicUniformBuffer;

float uvScale = 1.0;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 lightPosition;
layout(location = 3) out vec4 worldPosition;
layout(location = 4) out vec3 normal;
layout(location = 5) out float time;

void main()
{
    vec3 position = inPosition + inNormal * dynamicUniformBuffer.time * 0.1;
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord * uvScale;
    lightPosition = lightDataBuffer.lightPosition;
    worldPosition = ubo.model * vec4(inPosition, 1.0);
    normal = (ubo.model * vec4(inNormal, 0.0)).xyz;
    time = dynamicUniformBuffer.time;
}