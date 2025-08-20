#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIdx;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityID;


// TODO: Make this work
layout(set = 0, binding = 0) uniform UBO
{
    mat4 u_ViewProjection;
};

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out float v_TexIdx;
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
layout(location = 2) in float v_TexIdx;
layout(location = 3) in float v_TilingFactor;
layout(location = 4) in flat int v_EntityID;

#ifndef MAX_TEXTURES
#define MAX_TEXTURES 32  
#endif

layout(set = 1, binding = 0) uniform sampler2D u_Textures[MAX_TEXTURES]; 

void main()
{
    vec4 texColor = vec4(1.0);
    int idx = int(v_TexIdx);
    if (idx < 0 || idx > 31)
        idx = 0;

    texColor = texture(u_Textures[idx], v_TexCoord * v_TilingFactor);
    color = texColor * v_Color;

    entityID = v_EntityID;
}