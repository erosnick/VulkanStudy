#version 460

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 fragColor;

layout (binding = 1) uniform sampler2D textureSampler;

layout (location = 0) out vec4 outColor;

void main()
{
    vec3 n = normalize(normal);

    vec3 l = normalize(vec3(-1.0, -1.0, -1.0));

    float diffuse = max(0.0, dot(n, -l));

    vec3 albeo = texture(textureSampler, texcoord).rgb;

    outColor = vec4(vec3(diffuse), 1.0);
    // outColor = vec4(n, 1.0);
}
