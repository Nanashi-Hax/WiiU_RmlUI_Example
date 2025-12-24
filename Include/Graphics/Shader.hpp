#pragma once

#include "Graphics/UniformBuffer.hpp"
#include "whb/gfx.h"

class Shader
{
public:
    Shader(const void* file);
    ~Shader();

    void set();
    void setUniform(UniformBuffer* buffer, void* data, std::string name);

private:
    WHBGfxShaderGroup shaderGroup = {};
};