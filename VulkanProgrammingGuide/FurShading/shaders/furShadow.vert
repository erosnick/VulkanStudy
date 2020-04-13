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
}dynamicUniformBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord1;
layout(location = 2) out vec2 fragTexCoord2;
layout(location = 3) out vec3 lightPosition;
layout(location = 4) out vec4 worldPosition;
layout(location = 5) out vec3 normal;
layout(location = 6) out int layerIndex;

float UVScale = 1.0;

void main()
{
    vec3 position = inPosition + inNormal * dynamicUniformBuffer.furLength;

    float k = pow(dynamicUniformBuffer.layer, 3);

    vec3 gravity = vec3(0.0, dynamicUniformBuffer.gravity, 0.0);

    gravity = (ubo.model * vec4(gravity, 1.0)).xyz;

    // gravity = vec3(0.0, 0.0, 0.0);
    
    position += gravity * k;

    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(position, 1.0);

    fragColor = inColor;
    fragTexCoord1 = inTexCoord * UVScale;

    vec4 znormal = vec4(1.0, 1.0, 1.0, 0.0) - dot(vec4(normal, 0.0), vec4(0.0, 0.0, 1.0, 0.0));

    fragTexCoord2 = inTexCoord + znormal.xy * 0.0011;
    lightPosition = lightDataBuffer.lightPosition;
    worldPosition = ubo.model * vec4(inPosition, 1.0);
    normal = (ubo.model * vec4(inNormal, 0.0)).xyz;
    layerIndex = dynamicUniformBuffer.layerIndex;
}
