#include "Graphics/PixelUniformBuffer.hpp"
#include "GX2Ex.hpp"

PixelUniformBuffer::PixelUniformBuffer(uint32_t size) : UniformBuffer(size) {}

PixelUniformBuffer::~PixelUniformBuffer() {}

void PixelUniformBuffer::set(WHBGfxShaderGroup* shaderGroup, void* data, std::string name)
{
    GX2RSetPixelUniformBlockEx(shaderGroup, &buffer, data, size, name);
}
