#type vertex
#version 450

layout(location = 0) in vec3 a_Position;

layout(set = 0, binding = 1) uniform UBO
{
    mat4 u_ViewProjection;
    mat4 u_Transform;
} ubo;

void main()
{
    gl_Position = ubo.u_ViewProjection * ubo.u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450

void main()
{
}
