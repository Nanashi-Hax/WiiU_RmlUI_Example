#pragma once

#include <whb/gfx.h>
#include <string>

class UniformBuffer
{
public:
    virtual ~UniformBuffer();

    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

    UniformBuffer(UniformBuffer&& other) noexcept;
    UniformBuffer& operator=(UniformBuffer&& other) noexcept;

    virtual void set(WHBGfxShaderGroup* shaderGroup, void* data, std::string name) = 0;

protected:
    explicit UniformBuffer(uint32_t size);

    GX2RBuffer buffer = {};
    uint32_t size;
};