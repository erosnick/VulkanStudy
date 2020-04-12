#version 450
#extension GL_ARB_separate_shader_objects :enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inFragColor;
layout(location = 1) in vec2 inFragTexCoord;
layout(location = 2) in vec3 inLightPosition;
layout(location = 3) in vec4 inWorldPosition;
layout(location = 4) in vec3 inNormal;
layout(location = 5) in flat int layerIndex;

layout(binding = 3) uniform sampler2D geometryTextureSampler;
layout(binding = 4) uniform sampler2D modelTextureSampler;
layout(binding = 5) uniform sampler testSampler;
layout(binding = 6) uniform texture2D textures[60];

void main()
{
    vec3 normal = normalize(inNormal);
    vec3 lightDirection = normalize(inLightPosition - inWorldPosition.xyz);

    float diffuse  = max(dot(normal, lightDirection), 0.0);
    // outColor = vec4(vec3(fragTexCoord.x, fragTexCoord.y, 0.0), 1.0);
    // vec4 baseColor = texture(modelTextureSampler, inFragTexCoord);
    vec4 baseColor = texture(sampler2D(textures[layerIndex], testSampler), inFragTexCoord);

    vec4 finalColor =  baseColor;
    outColor = finalColor;
}