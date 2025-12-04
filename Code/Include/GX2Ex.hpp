#pragma once

#include <cstdint>
#include <string>
#include <gx2r/buffer.h>
#include <whb/gfx.h>

// Create shader group from shader data
WHBGfxShaderGroup* WHBGfxCreateShaderGroup(unsigned char* shaderData);

// Get uniform block location by name
template<typename T>
int32_t GX2GetUniformBlockLocation(T* shader, const char* name);

// Get uniform variable location by name
template<typename T>
int32_t GX2GetUniformVarLocation(T* shader, const char* name);

// Initialize uniform buffer
void GX2InitUniformBuffer(GX2RBuffer* buffer, size_t size, uint32_t count);

// Set shader group (vertex + pixel + fetch shaders)
void GX2SetShaderGroup(WHBGfxShaderGroup* shaderGroup);

// Endian swap utilities
constexpr uint32_t SwapUInt32(uint32_t x);
void SwapMemcpy(void* dst, const void* src, size_t size);

// Set vertex uniform block with data (includes lock/unlock and endian swap)
void GX2RSetVertexUniformBlockEx(WHBGfxShaderGroup* shaderGroup, GX2RBuffer* buffer, void* data, size_t size, std::string name);

// Set pixel uniform block with data (includes lock/unlock and endian swap)
void GX2RSetPixelUniformBlockEx(WHBGfxShaderGroup* shaderGroup, GX2RBuffer* buffer, void* data, size_t size, std::string name);
