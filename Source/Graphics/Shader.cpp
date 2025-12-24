#include "Graphics/Shader.hpp"
#include "Graphics/UniformBuffer.hpp"
#include "ShaderMem.hpp"
#include <stdexcept>

Shader::Shader(const void* file)
{
	if(!WHBGfxLoadGFDShaderGroupMappedMem(&shaderGroup, 0, file))
	{
		throw std::invalid_argument("Invalid shader file");
	}
    
	WHBGfxInitShaderAttribute(&shaderGroup, "Position", 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);
	WHBGfxInitShaderAttribute(&shaderGroup, "Color", 0, 8, GX2_ATTRIB_FORMAT_UNORM_8_8_8_8);
	WHBGfxInitShaderAttribute(&shaderGroup, "TexCoord", 0, 12, GX2_ATTRIB_FORMAT_FLOAT_32_32);
	WHBGfxInitFetchShaderMappedMem(&shaderGroup);
}

Shader::~Shader()
{
    WHBGfxFreeShaderGroupMappedMem(&shaderGroup);
}

void Shader::set()
{
	GX2SetFetchShader(&shaderGroup.fetchShader);
	GX2SetVertexShader(shaderGroup.vertexShader);
	GX2SetPixelShader(shaderGroup.pixelShader);

    GX2SetShaderMode(GX2_SHADER_MODE_UNIFORM_BLOCK);
}

void Shader::setUniform(UniformBuffer* buffer, void* data, std::string name)
{
    buffer->set(&shaderGroup, data, name);
}