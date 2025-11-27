#version 450

layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec4 Color;

layout(binding = 0) uniform ProjectionBlock
{
    mat4 Projection;
};

out vec2 vUV;
out vec4 vColor;

void main()
{
    vUV = UV;
    vColor = Color;
    gl_Position = Projection * vec4(Position, 0.0, 1.0);
}