#version 450
#extension GL_ARB_separate_shader_objects :enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inFragColor;
layout(location = 1) in vec2 inFragTexCoord;
layout(location = 2) in vec3 inLightPosition;
layout(location = 3) in vec4 inWorldPosition;
layout(location = 4) in vec3 inNormal;

layout(binding = 3) uniform sampler2D geometryTextureSampler;
layout(binding = 4) uniform sampler2D modelTextureSampler;

void main()
{
    vec3 normal = normalize(inNormal);
    vec3 lightDirection = normalize(inLightPosition - inWorldPosition.xyz);

    float diffuse  = max(dot(normal, lightDirection), 0.0);
    // outColor = vec4(vec3(fragTexCoord.x, fragTexCoord.y, 0.0), 1.0);
    outColor = texture(modelTextureSampler, inFragTexCoord);

    // vec3 finalColor =  inFragColor * diffuse;
    // outColor = vec4(finalColor, 1);
}