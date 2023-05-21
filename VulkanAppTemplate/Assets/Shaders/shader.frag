#version 460

#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 fragColor;
layout (location = 3) in vec3 cameraPosition;
layout (location = 4) in vec3 worldPosition;
layout (location = 5) in mat3 TBN;

const int LightCount = 16;

layout (binding = 2) uniform MaterialUniformBufferObject
{
    vec4 albedo;
    float metallic;
	float roughness;
	float ao;
	int diffuseTextureIndex;
    int normalTextureIndex;
    int roughnessTextureIndex;
    int metallicTextureIndex;
    int alphaTextureIndex;

} materialUBO;

layout (binding = 3) uniform LightUniformBufferObject
{
    vec4 lightPositions[LightCount];
    vec4 lightColors[LightCount];
    uint turnOnLightCount;
} lightUBO;

const int TextureUnits = 64;

layout (binding = 4) uniform sampler2D renderTextureSampler;
layout (binding = 5) uniform sampler2D textureSampler[];

layout (location = 0) out vec4 outColor;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    vec3 albedo = vec3(1.0);

    if (materialUBO.diffuseTextureIndex >= 0)
    {
        albedo = pow(texture(textureSampler[materialUBO.diffuseTextureIndex], texcoord).rgb, vec3(2.2));
    }
    else
    {
        albedo = materialUBO.albedo.rgb;
    }

    vec3 testColor = vec3(0.0, 0.0, 0.0);

    if (materialUBO.alphaTextureIndex > 0)
    {
        float alpha = texture(textureSampler[materialUBO.alphaTextureIndex], texcoord).a;

        if (alpha < 0.1)
        {
            testColor = vec3(1.0, 0.0, 0.0);

            discard;
        }
    }

    vec3 N = normalize(normal);

    if (materialUBO.normalTextureIndex > 0)
    {
        N = texture(textureSampler[materialUBO.normalTextureIndex], texcoord).rgb;
        N = N * 2.0 - 1.0;
        N = normalize(TBN * N);
    }

    vec3 V = normalize(cameraPosition - worldPosition);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    float roughness = materialUBO.roughness;

    if (materialUBO.roughnessTextureIndex > 0)
    {
        roughness = texture(textureSampler[materialUBO.roughnessTextureIndex], texcoord).r;
    }

    float metallic = materialUBO.metallic;

    if (materialUBO.metallicTextureIndex > 0)
    {
        metallic = texture(textureSampler[materialUBO.metallicTextureIndex], texcoord).r;
    }

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    
    for(int i = 0; i < lightUBO.turnOnLightCount; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lightUBO.lightPositions[i].xyz - worldPosition);
        vec3 H = normalize(V + L);
        float distance = length(lightUBO.lightPositions[i].xyz - worldPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightUBO.lightColors[i].xyz * attenuation;

        // Cook-Torrance BRDF
        float NDF = distributionGGX(N, H, roughness);
        float G   = geometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
        Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL;
    }   
    
    // // ambient lighting (note that the next IBL tutorial will replace 
    // // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo * materialUBO.ao;

    vec3 finalColor = ambient + Lo * fragColor;

    // HDR tonemapping
    finalColor = finalColor / (finalColor + vec3(1.0));

    // gamma correct
    // 交换链使用VK_FORMAT_B8G8R8A8_SRGB格式，shader里不用对最终结果进行gamma correction，这一步会自动执行(和OpenGL不同)
    // finalColor = pow(finalColor, vec3(1.0 / 2.2));

    outColor = vec4(vec3(finalColor), 1.0);
    // outColor = vec4(testColor, 1.0);
    // outColor = vec4(diffuse, 1.0);            // Visualize diffuse
    // outColor = vec4(specular, 1.0);           // Visualize specular term
    // outColor = vec4(F, 1.0);                  // Visualize Fresnel
    // outColor = vec4(vec3(roughness), 1.0);    // Visualize roughness
    // outColor = vec4(vec3(metallic), 1.0);     // Visualize metallic
    // outColor = vec4(N, 1.0); // Visualize normal
    // outColor = vec4(V, 1.0); // Visualize view direction
    // outColor = vec4(lightDirection, 1.0); // Visualize light direction
    // // outColor = vec4(materialUniformBufferObject.diffuseColor.rgb, 1.0);
    // // outColor = vec4(vec3(materialUniformBufferObject.diffuseColor.r, 0.0, 0.0), 1.0);
}
