#include "gx2_extra.hpp"

#include <cstdint>
#include <cstring>
#include <string>

#include <coreinit/debug.h>
#include <gx2/mem.h>
#include <gx2/shaders.h>
#include <gx2r/buffer.h>
#include <whb/gfx.h>

#include "gfx_shader_mappedmem.h"

WHBGfxShaderGroup* WHBGfxCreateShaderGroup(unsigned char* shaderData)
{
    WHBGfxShaderGroup* shaderGroup = new WHBGfxShaderGroup();
    WHBGfxLoadGFDShaderGroupMappedMem(shaderGroup, 0, shaderData);
    return shaderGroup;
}

template<typename T>
int32_t GX2GetUniformBlockLocation(T* shader, const char* name)
{
    for (uint32_t i = 0; i < shader->uniformBlockCount; i++)
    {
        if(strcmp(shader->uniformBlocks[i].name, name) == 0)
        {
            return shader->uniformBlocks[i].offset;
        }
    }
    return -1;
}

template int32_t GX2GetUniformBlockLocation(GX2VertexShader* shader, const char* name);
template int32_t GX2GetUniformBlockLocation(GX2PixelShader* shader, const char* name);

template<typename T>
int32_t GX2GetUniformVarLocation(T* shader, const char* name)
{
    for (uint32_t i = 0; i < shader->uniformVarCount; i++)
    {
        if(strcmp(shader->uniformVars[i].name, name) == 0)
        {
            return shader->uniformVars[i].offset;
        }
    }
    return -1;
}

template int32_t GX2GetUniformVarLocation(GX2VertexShader* shader, const char* name);
template int32_t GX2GetUniformVarLocation(GX2PixelShader* shader, const char* name);

void GX2InitUniformBuffer(GX2RBuffer* buffer, size_t size, uint32_t count)
{
    buffer->flags =
    GX2R_RESOURCE_BIND_UNIFORM_BLOCK | GX2R_RESOURCE_USAGE_CPU_READ |
    GX2R_RESOURCE_USAGE_CPU_WRITE | GX2R_RESOURCE_USAGE_GPU_READ;
    buffer->elemSize = size;
    buffer->elemCount = count;
    GX2RCreateBuffer(buffer);

    void* lockedBuffer = GX2RLockBufferEx(buffer, GX2R_RESOURCE_BIND_UNIFORM_BLOCK);
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_UNIFORM_BLOCK, lockedBuffer, buffer->elemSize * buffer->elemCount);
    GX2RUnlockBufferEx(buffer, GX2R_RESOURCE_BIND_UNIFORM_BLOCK);
}

void GX2SetShaderGroup(WHBGfxShaderGroup* shaderGroup)
{
    GX2SetVertexShader(shaderGroup->vertexShader);
    GX2SetPixelShader(shaderGroup->pixelShader);
    GX2SetFetchShader(&shaderGroup->fetchShader);
}

constexpr uint32_t SwapUInt32(uint32_t x)
{
    return ((x >> 24) & 0x000000FF) |
           ((x >> 8)  & 0x0000FF00) |
           ((x << 8)  & 0x00FF0000) |
           ((x << 24) & 0xFF000000);
}

void SwapMemcpy(void* dst, const void* src, size_t size)
{
    auto* csrc = static_cast<const uint32_t*>(src);
    auto* cdst = static_cast<uint32_t*>(dst);

    size_t count = size / sizeof(uint32_t);
    for (size_t i = 0; i < count; ++i)
    {
        cdst[i] = SwapUInt32(csrc[i]);
    }
}

void GX2RSetVertexUniformBlockEx(WHBGfxShaderGroup* shaderGroup, GX2RBuffer* buffer, void* data, size_t size, std::string name)
{
    void* lockedBuffer = GX2RLockBufferEx(buffer, GX2R_RESOURCE_BIND_UNIFORM_BLOCK);
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_UNIFORM_BLOCK, lockedBuffer, buffer->elemSize * buffer->elemCount);
    SwapMemcpy(lockedBuffer, data, size);
    GX2RUnlockBufferEx(buffer, GX2R_RESOURCE_BIND_UNIFORM_BLOCK);

    int32_t location = GX2GetUniformBlockLocation(shaderGroup->vertexShader, name.c_str());
    GX2RSetVertexUniformBlock(const_cast<GX2RBuffer *>(buffer), location, 0);
}

void GX2RSetPixelUniformBlockEx(WHBGfxShaderGroup* shaderGroup, GX2RBuffer* buffer, void* data, size_t size, std::string name)
{
    void* lockedBuffer = GX2RLockBufferEx(buffer, GX2R_RESOURCE_BIND_UNIFORM_BLOCK);
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU | GX2_INVALIDATE_MODE_UNIFORM_BLOCK, lockedBuffer, buffer->elemSize * buffer->elemCount);
    SwapMemcpy(lockedBuffer, data, size);
    GX2RUnlockBufferEx(buffer, GX2R_RESOURCE_BIND_UNIFORM_BLOCK);

    int32_t location = GX2GetUniformBlockLocation(shaderGroup->pixelShader, name.c_str());
    GX2RSetPixelUniformBlock(const_cast<GX2RBuffer *>(buffer), location, 0);
}
