#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec3 v_WorldPos;

layout(set = 0, binding = 1) uniform UBO
{
    mat4 u_ViewProjection;
    mat4 u_Transform;
} ubo;

void main()
{
    vec4 worldPos = ubo.u_Transform * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(ubo.u_Transform)));
    v_Normal = normalize(normalMatrix * a_Normal);

    gl_Position = ubo.u_ViewProjection * worldPos;
}

#type fragment
#version 450

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec3 v_WorldPos;

struct Light
{
    vec3 Direction;   
    vec3 Color;
    float Intensity;
};

#define MAX_LIGHT_COUNT 4

layout(set = 0, binding = 2) uniform LightsUBO
{
    int u_LightCount;
    Light u_Lights[MAX_LIGHT_COUNT];
} lights;

layout(set = 0, binding = 3) uniform MaterialUBO
{
    vec4 u_Albedo;
    vec3 u_Emissive;
    float _Padding0;
} material;

layout(set = 0, binding = 4) uniform DrawUBO
{
    int u_EntityID;
    int _Padding1;
    int _Padding2;
    int _Padding3;
} draw;

layout(set = 0, binding = 5) uniform ShadowUBO
{
    mat4 u_LightSpaceMatrices[MAX_LIGHT_COUNT];
} shadows;

layout(set = 0, binding = 0) uniform sampler2D u_ShadowMaps[MAX_LIGHT_COUNT];

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int EntityID;

float SampleShadowMap(int lightIndex, vec2 uv)
{
    switch (lightIndex)
    {
        case 0: return texture(u_ShadowMaps[0], uv).r;
        case 1: return texture(u_ShadowMaps[1], uv).r;
        case 2: return texture(u_ShadowMaps[2], uv).r;
        case 3: return texture(u_ShadowMaps[3], uv).r;
        default: return 1.0;
    }
}

float ShadowCalculation(int lightIndex, vec3 normal, vec3 lightDir)
{
    vec4 fragPosLightSpace = shadows.u_LightSpaceMatrices[lightIndex] * vec4(v_WorldPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z <= 0.0 || projCoords.z > 1.0)
        return 0.0;

    if (any(lessThan(projCoords.xy, vec2(0.0))) || any(greaterThan(projCoords.xy, vec2(1.0))))
        return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - max(dot(normal, lightDir), 0.0)), 0.0005);

    vec2 texelSize = 1.0 / textureSize(u_ShadowMaps[0], 0);
    float shadow = 0.0;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            vec2 sampleCoord = projCoords.xy + vec2(x, y) * texelSize;
            float pcfDepth = SampleShadowMap(lightIndex, sampleCoord);
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

void main()
{
    vec3 normal = normalize(v_Normal);
    vec3 albedo = material.u_Albedo.rgb;

    vec3 ambient = 0.1 * albedo;
    vec3 totalDiffuse = vec3(0.0);

    int lightCount = min(lights.u_LightCount, MAX_LIGHT_COUNT);
    for (int i = 0; i < lightCount; i++)
    {
        vec3 lightDir = normalize(-lights.u_Lights[i].Direction);
        float NdotL = max(dot(normal, lightDir), 0.0);

        vec3 diffuse = 
            albedo *
            lights.u_Lights[i].Color *
            lights.u_Lights[i].Intensity *
            NdotL;

        float shadow = ShadowCalculation(i, normal, lightDir);
        totalDiffuse += (1.0 - shadow) * diffuse;
    }

    vec3 finalColor = ambient + totalDiffuse + material.u_Emissive;

    FragColor = vec4(finalColor, material.u_Albedo.a);
    EntityID = draw.u_EntityID;
}
