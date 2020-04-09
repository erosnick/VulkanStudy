#version 450
#extension GL_ARB_separate_shader_objects :enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D geometryTextureSampler;
layout(binding = 2) uniform sampler2D modelTextureSampler;

void main()
{
    // outColor = vec4(vec3(fragTexCoord.x, fragTexCoord.y, 0.0), 1.0);
    outColor = texture(modelTextureSampler, fragTexCoord);
}