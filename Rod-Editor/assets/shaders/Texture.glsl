#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIdx;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityID;


layout(set = 0, binding = 0) uniform UBO
{
    mat4 u_ViewProjection;
};

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out flat float v_TexIdx;
layout(location = 3) out float v_TilingFactor;
layout(location = 4) out flat int v_EntityID;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_TexIdx = a_TexIdx;
    v_TilingFactor = a_TilingFactor;
    v_EntityID = a_EntityID;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 color;
layout(location = 1) out int entityID;

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in flat float v_TexIdx;
layout(location = 3) in float v_TilingFactor;
layout(location = 4) in flat int v_EntityID;

#ifndef MAX_TEXTURES
#define MAX_TEXTURES 32
#endif

layout(set = 1, binding = 0) uniform sampler2D u_Textures[MAX_TEXTURES];

vec4 SampleTexture(int idx, vec2 uv)
{
    switch (idx)
    {
        case 0:  return texture(u_Textures[0],  uv);
        case 1:  return texture(u_Textures[1],  uv);
        case 2:  return texture(u_Textures[2],  uv);
        case 3:  return texture(u_Textures[3],  uv);
        case 4:  return texture(u_Textures[4],  uv);
        case 5:  return texture(u_Textures[5],  uv);
        case 6:  return texture(u_Textures[6],  uv);
        case 7:  return texture(u_Textures[7],  uv);
        case 8:  return texture(u_Textures[8],  uv);
        case 9:  return texture(u_Textures[9],  uv);
        case 10: return texture(u_Textures[10], uv);
        case 11: return texture(u_Textures[11], uv);
        case 12: return texture(u_Textures[12], uv);
        case 13: return texture(u_Textures[13], uv);
        case 14: return texture(u_Textures[14], uv);
        case 15: return texture(u_Textures[15], uv);
        case 16: return texture(u_Textures[16], uv);
        case 17: return texture(u_Textures[17], uv);
        case 18: return texture(u_Textures[18], uv);
        case 19: return texture(u_Textures[19], uv);
        case 20: return texture(u_Textures[20], uv);
        case 21: return texture(u_Textures[21], uv);
        case 22: return texture(u_Textures[22], uv);
        case 23: return texture(u_Textures[23], uv);
        case 24: return texture(u_Textures[24], uv);
        case 25: return texture(u_Textures[25], uv);
        case 26: return texture(u_Textures[26], uv);
        case 27: return texture(u_Textures[27], uv);
        case 28: return texture(u_Textures[28], uv);
        case 29: return texture(u_Textures[29], uv);
        case 30: return texture(u_Textures[30], uv);
        case 31: return texture(u_Textures[31], uv);
        default: return texture(u_Textures[0],  uv);
    }
}

void main()
{
    int idx = int(v_TexIdx);
    idx = clamp(idx, 0, MAX_TEXTURES - 1);

    vec2 uv = v_TexCoord * v_TilingFactor;
    vec4 texColor = SampleTexture(idx, uv);

    color = texColor * v_Color;
    entityID = v_EntityID;
}