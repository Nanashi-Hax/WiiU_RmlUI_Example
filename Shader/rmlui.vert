#version 450

layout(location = 0) in vec2 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 TexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(binding = 0) uniform ProjectionBlock
{
    mat4 Transform;
};

layout(binding = 1) uniform TransformBlock
{
    mat4 TransformMatrix;
};

void main() {
    // TransformMatrix includes both translation and transform
    gl_Position = Transform * TransformMatrix * vec4(Position, 0.0, 1.0);
    fragColor = Color;
    fragTexCoord = TexCoord;
}
