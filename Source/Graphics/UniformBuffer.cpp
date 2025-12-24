#include "Graphics/UniformBuffer.hpp"
#include "GX2Ex.hpp"

UniformBuffer::UniformBuffer(uint32_t size) : size(size)
{
    GX2InitUniformBuffer(&buffer, size, 1);
}

UniformBuffer::~UniformBuffer()
{
    if (buffer.flags)
    {
        GX2RDestroyBufferEx(&buffer, GX2R_RESOURCE_BIND_NONE);
    }
}

UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    : buffer(other.buffer), size(other.size)
{
    other.buffer = {};
    other.size = 0;
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept
{
    if (this != &other)
    {
        if (buffer.flags)
        {
            GX2RDestroyBufferEx(&buffer, GX2R_RESOURCE_BIND_NONE);
        }

        buffer = other.buffer;
        size = other.size;

        other.buffer = {};
        other.size = 0;
    }
    return *this;
}
