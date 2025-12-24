#pragma once

#include "Graphics/UniformBuffer.hpp"

class PixelUniformBuffer : public UniformBuffer
{
public:
    explicit PixelUniformBuffer(uint32_t size);
    ~PixelUniformBuffer();

    void set(WHBGfxShaderGroup* shaderGroup, void* data, std::string name) override;
};
