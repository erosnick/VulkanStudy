#version 460

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec3 fragColor;

layout (binding = 0) uniform ParticleUniformBufferObject 
{
    mat4 model;
    mat4 view;
	mat4 projection;
    float deltaTime;
} ubo;

void main() 
{
    gl_PointSize = 14.0;
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition.xy, 0.5, 1.0);
    fragColor = inColor.rgb;
}
