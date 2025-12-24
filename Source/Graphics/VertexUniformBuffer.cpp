#include "Graphics/VertexUniformBuffer.hpp"
#include "GX2Ex.hpp"

VertexUniformBuffer::VertexUniformBuffer(uint32_t size) : UniformBuffer(size) {}

VertexUniformBuffer::~VertexUniformBuffer() {}

void VertexUniformBuffer::set(WHBGfxShaderGroup* shaderGroup, void* data, std::string name)
{
    GX2RSetVertexUniformBlockEx(shaderGroup, &buffer, data, size, name);
}
