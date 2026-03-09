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

#define MAX_LIGHT_COUNT 64

layout(set = 0, binding = 2) uniform LightsUBO
{
    int u_LightCount;
    Light[MAX_LIGHT_COUNT] u_Lights;
} lights;

layout(location = 0) out vec4 FragColor;

void main()
{
    vec3 normal = normalize(v_Normal);
    vec3 albedo = vec3(0.6, 0.2, 0.8);

    vec3 ambient = 0.1 * albedo;
    vec3 totalDiffuse = vec3(0.0);

    for (int i = 0; i < lights.u_LightCount; i++)
    {
        vec3 lightDir = normalize(-lights.u_Lights[i].Direction);
        float NdotL = max(dot(normal, lightDir), 0.0);

        vec3 diffuse = 
            albedo *
            lights.u_Lights[i].Color *
            lights.u_Lights[i].Intensity *
            NdotL;

        totalDiffuse += diffuse;
    }

    vec3 finalColor = ambient + totalDiffuse;

    FragColor = vec4(finalColor, 1.0);
}