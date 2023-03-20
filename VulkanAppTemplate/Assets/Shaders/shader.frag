#version 460

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 fragColor;
layout (location = 3) in vec3 cameraPosition;
layout (location = 4) in vec3 worldPosition;

layout (binding = 2) uniform MaterialUniformBufferObject
{
    vec4 albedo;
    float metallic;
	float roughness;
	float ao;
	float padding;
} materialUniformBufferObject;

layout (binding = 3) uniform LightUniformBufferObject
{
    vec4 lightPositions[4];
    vec4 lightColors[4];
} lightUniformBufferObject;

layout (binding = 4) uniform sampler2D textureSampler;
layout (binding = 5) uniform sampler2D alphaTextureSampler;

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
    vec3 textureAlbedo = vec3(1.0);

    if (materialUniformBufferObject.albedo.a > 0.0)
    {
        textureAlbedo = texture(textureSampler, texcoord).rgb;
    }
    else
    {
        textureAlbedo = materialUniformBufferObject.albedo.rgb;
    }

    if (materialUniformBufferObject.albedo.a > 1.0)
    {
        float alpha = texture(alphaTextureSampler, texcoord).r;

        if (alpha < 0.1)
        {
            discard;
        }
    }

    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPosition - worldPosition);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, materialUniformBufferObject.albedo.rgb, materialUniformBufferObject.metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(lightUniformBufferObject.lightPositions[i].xyz - worldPosition);
        vec3 H = normalize(V + L);
        float distance = length(lightUniformBufferObject.lightPositions[i].xyz - worldPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightUniformBufferObject.lightColors[i].xyz * attenuation;

        // Cook-Torrance BRDF
        float NDF = distributionGGX(N, H, materialUniformBufferObject.roughness);
        float G   = geometrySmith(N, V, L, materialUniformBufferObject.roughness);
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
        kD *= 1.0 - materialUniformBufferObject.metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
        Lo += (kD * materialUniformBufferObject.albedo.rgb / PI + specular) * radiance * NdotL;
    }   
    
    // // ambient lighting (note that the next IBL tutorial will replace 
    // // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * textureAlbedo * materialUniformBufferObject.ao;

    vec3 finalColor = ambient + Lo * textureAlbedo;

    // HDR tonemapping
    finalColor = finalColor / (finalColor + vec3(1.0));

    // gamma correct
    finalColor = pow(finalColor, vec3(1.0 / 2.2)); 

    outColor = vec4(vec3(finalColor), 1.0);
    // // outColor = vec4(materialUniformBufferObject.diffuseColor.rgb, 1.0);
    // // outColor = vec4(vec3(materialUniformBufferObject.diffuseColor.r, 0.0, 0.0), 1.0);
}
