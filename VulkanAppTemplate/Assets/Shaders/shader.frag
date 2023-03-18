#version 460

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 fragColor;

layout (binding = 2) uniform MaterialUniformBufferObject
{
    vec4 diffuseColor;
    vec4 padding;
} materialUniformBufferObject;

layout (binding = 3) uniform sampler2D textureSampler;
layout (binding = 4) uniform sampler2D alphaTextureSampler;

layout (location = 0) out vec4 outColor;

void main()
{
    vec3 n = normalize(normal);

    vec3 l = normalize(vec3(-1.0, -1.0, -1.0));

    float diffuse = max(0.0, dot(n, -l));

    vec3 albedo = vec3(1.0);

    if (materialUniformBufferObject.diffuseColor.a > 0.0)
    {
        albedo = texture(textureSampler, texcoord).rgb;
    }
    else
    {
        albedo = materialUniformBufferObject.diffuseColor.rgb;
    }

    if (materialUniformBufferObject.diffuseColor.a > 1.0)
    {
        float alpha = texture(alphaTextureSampler, texcoord).r;

        if (alpha < 0.1)
        {
            discard;
        }
    }

    vec3 ambient = vec3(0.3) * albedo;

    vec3 finalColor = ambient + albedo;

    outColor = vec4(vec3(finalColor), 1.0);
    // outColor = vec4(materialUniformBufferObject.diffuseColor.rgb, 1.0);
    // outColor = vec4(vec3(materialUniformBufferObject.diffuseColor.r, 0.0, 0.0), 1.0);
}
