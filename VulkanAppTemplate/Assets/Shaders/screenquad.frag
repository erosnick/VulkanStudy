#version 460

layout (binding = 0) uniform sampler2D samplerColor0;
layout (binding = 1) uniform sampler2D samplerColor1;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout (constant_id = 0) const int tonemapping = 0;

void main() 
{
    vec3 finalColor = texture(samplerColor0, inUV).rgb;

    if (tonemapping == 1)
    {
        // HDR tonemapping
        finalColor = finalColor / (finalColor + vec3(1.0));
    }
	
    outColor = vec4(finalColor, 1.0);
}