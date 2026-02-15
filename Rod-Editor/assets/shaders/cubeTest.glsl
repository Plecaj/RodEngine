#type vertex
#version 450

layout(location = 0) in vec3 a_Position;

layout(location = 0) out vec3 v_Color; 
layout(set = 0, binding = 1) uniform UBO
{
    mat4 u_ViewProjection;
    mat4 u_Transform;
} ubo;

void main()
{
    gl_Position = ubo.u_ViewProjection * ubo.u_Transform * vec4(a_Position, 1.0);
    v_Color = vec3(0.6, 0.2, 0.8); 
}

#type fragment
#version 450

layout(location = 0) in vec3 v_Color;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(v_Color, 1.0);
}

