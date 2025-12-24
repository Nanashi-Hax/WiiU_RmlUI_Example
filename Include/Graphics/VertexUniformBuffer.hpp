#pragma once

#include "Graphics/UniformBuffer.hpp"

class VertexUniformBuffer : UniformBuffer
{
public:
    explicit VertexUniformBuffer(uint32_t size);
    ~VertexUniformBuffer();

    void set(WHBGfxShaderGroup* shaderGroup, void* data, std::string name) override;
};
