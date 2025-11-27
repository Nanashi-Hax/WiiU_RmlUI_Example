/*
 * RmlUi Wii U GX2 Renderer Implementation
 * Based on ImGui GX2 backend implementation
 */

#include "RmlUi_Renderer_GX2.h"
#include <RmlUi/Core.h>

// GX2 includes
#include <whb/gfx.h>
#include <whb/log.h>
#include <gx2/registers.h>
#include <gx2/draw.h>
#include <gx2/utils.h>
#include <gx2/mem.h>
#include <gx2/clear.h>
#include <gx2r/buffer.h>
#include <memory/mappedmemory.h>
#include <cstring>
#include "gfx_shader_mappedmem.h"
#include "gx2_extra.hpp"

// Include your shader data
#include "rmlui_gsh.h"

RenderInterface_GX2::RenderInterface_GX2() {
	// Shader group will be initialized in BeginFrame
}

RenderInterface_GX2::~RenderInterface_GX2() {
	if (shader_group) {
		WHBGfxFreeShaderGroupMappedMem(shader_group);
		delete shader_group;
		shader_group = nullptr;
	}
    if (projection_buffer.buffer) {
        GX2RDestroyBufferEx(&projection_buffer, GX2R_RESOURCE_BIND_NONE);
    }
    if (default_texture) {
        ReleaseTexture(reinterpret_cast<Rml::TextureHandle>(default_texture));
        default_texture = nullptr;
    }
}

void RenderInterface_GX2::SetViewport(int width, int height) {
	viewport_width = width;
	viewport_height = height;
	
	// Viewport will be set in SetupRenderState()
}

void RenderInterface_GX2::SetupRenderState() {
	// Based on ImGui implementation
	// Setup render state: alpha-blending enabled, no face culling, no depth testing
	GX2SetColorControl(GX2_LOGIC_OP_COPY, 0xFF, FALSE, TRUE);
	
	// Premultiplied alpha blending (GL_ONE, GL_ONE_MINUS_SRC_ALPHA)
	GX2SetBlendControl(GX2_RENDER_TARGET_0,
		GX2_BLEND_MODE_ONE,                    // src color
		GX2_BLEND_MODE_INV_SRC_ALPHA,         // dst color
		GX2_BLEND_COMBINE_MODE_ADD,
		TRUE,
		GX2_BLEND_MODE_ONE,                    // src alpha
		GX2_BLEND_MODE_INV_SRC_ALPHA,         // dst alpha
		GX2_BLEND_COMBINE_MODE_ADD);
	
	GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, FALSE, FALSE);
	GX2SetDepthOnlyControl(FALSE, FALSE, GX2_COMPARE_FUNC_NEVER);
	
	// Setup viewport
	GX2SetViewport(0, 0, (float)viewport_width, (float)viewport_height, 0.0f, 1.0f);
	
	// Set shaders
	if (shader_group) {
		GX2SetFetchShader(&shader_group->fetchShader);
		GX2SetVertexShader(shader_group->vertexShader);
		GX2SetPixelShader(shader_group->pixelShader);
		
		// CRITICAL: Set shader mode to use uniform blocks
		GX2SetShaderMode(GX2_SHADER_MODE_UNIFORM_BLOCK);
	}
	
	// Setup orthographic projection matrix
	// RmlUi uses top-left origin (0,0) to bottom-right (width, height)
	float L = 0.0f;
	float R = (float)viewport_width;
	float T = 0.0f;
	float B = (float)viewport_height;
	
	const float ortho_projection[4][4] = {
		{ 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
		{ 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
		{ 0.0f,         0.0f,        -1.0f,   0.0f },
		{ (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
	};
	
	// Use helper function to set uniform block (handles lock/unlock and endian swap)
	if (projection_buffer.buffer && shader_group) {
		GX2RSetVertexUniformBlockEx(shader_group, &projection_buffer, (void*)ortho_projection, sizeof(ortho_projection), "ProjectionBlock");
	}
}

void RenderInterface_GX2::BeginFrame() {
	// Initialize shaders on first frame
	if (!shader_group) {
		shader_group = new WHBGfxShaderGroup();
		WHBGfxLoadGFDShaderGroupMappedMem(shader_group, 0, rmlui_gsh);
		
		WHBGfxInitShaderAttribute(shader_group, "Position", 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);
		WHBGfxInitShaderAttribute(shader_group, "Color", 0, 8, GX2_ATTRIB_FORMAT_UNORM_8_8_8_8);
		WHBGfxInitShaderAttribute(shader_group, "TexCoord", 0, 12, GX2_ATTRIB_FORMAT_FLOAT_32_32);
		WHBGfxInitFetchShaderMappedMem(shader_group);
		
		// Initialize projection uniform buffer
		GX2InitUniformBuffer(&projection_buffer, sizeof(float) * 16, 1);
	}

    // Initialize default texture
    if (!default_texture) {
        Rml::byte white_pixel[4] = { 255, 255, 255, 255 };
        default_texture = reinterpret_cast<TextureData*>(GenerateTexture(Rml::Span<const Rml::byte>(white_pixel, 4), Rml::Vector2i(1, 1)));
    }
	
	SetupRenderState();
}

void RenderInterface_GX2::EndFrame() {
	// Restore previous GX2 state if needed
}

void RenderInterface_GX2::Clear() {
	GX2ClearColor(nullptr, 0.0f, 0.0f, 0.0f, 0.0f);
}

Rml::CompiledGeometryHandle RenderInterface_GX2::CompileGeometry(
	Rml::Span<const Rml::Vertex> vertices, 
	Rml::Span<const int> indices) 
{
	GeometryData* geometry = new GeometryData();
	
	// Allocate GX2 vertex buffer
	uint32_t vtx_buffer_size = vertices.size() * sizeof(Rml::Vertex);
	geometry->vertex_buffer = MEMAllocFromMappedMemoryForGX2Ex(vtx_buffer_size, GX2_VERTEX_BUFFER_ALIGNMENT);
	if (!geometry->vertex_buffer) {
		delete geometry;
		return 0;
	}
	
	// Copy vertex data
	std::memcpy(geometry->vertex_buffer, vertices.data(), vtx_buffer_size);
	geometry->num_vertices = vertices.size();
	
	// Allocate GX2 index buffer
	uint32_t idx_buffer_size = indices.size() * sizeof(int);
	geometry->index_buffer = MEMAllocFromMappedMemoryForGX2Ex(idx_buffer_size, GX2_INDEX_BUFFER_ALIGNMENT);
	if (!geometry->index_buffer) {
		MEMFreeToMappedMemory(geometry->vertex_buffer);
		delete geometry;
		return 0;
	}
	
	// Copy index data
	std::memcpy(geometry->index_buffer, indices.data(), idx_buffer_size);
	geometry->num_indices = indices.size();
	
	// Flush CPU cache to GPU
	GX2Invalidate(GX2_INVALIDATE_MODE_CPU_ATTRIBUTE_BUFFER, geometry->vertex_buffer, vtx_buffer_size);
	GX2Invalidate(GX2_INVALIDATE_MODE_CPU, geometry->index_buffer, idx_buffer_size);
	
	return reinterpret_cast<Rml::CompiledGeometryHandle>(geometry);
}

void RenderInterface_GX2::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
	if (!geometry)
		return;
	
	GeometryData* data = reinterpret_cast<GeometryData*>(geometry);
	MEMFreeToMappedMemory(data->vertex_buffer);
	MEMFreeToMappedMemory(data->index_buffer);
	delete data;
}

void RenderInterface_GX2::RenderGeometry(
	Rml::CompiledGeometryHandle geometry, 
	Rml::Vector2f translation, 
	Rml::TextureHandle texture) 
{
	if (!geometry)
		return;
	
	GeometryData* data = reinterpret_cast<GeometryData*>(geometry);
	
	// TODO: Apply translation (via vertex shader uniform or push constant)
	// For now, translation is not applied
	
	// Set vertex attributes
	GX2SetAttribBuffer(0, data->num_vertices * sizeof(Rml::Vertex), sizeof(Rml::Vertex), data->vertex_buffer);
	
	// Bind texture if provided, otherwise use default white texture
    TextureData* tex = nullptr;
	if (texture) {
		tex = reinterpret_cast<TextureData*>(texture);
	} else {
        tex = default_texture;
    }

    if (tex) {
		GX2SetPixelTexture(tex->texture, 0);
		GX2SetPixelSampler(tex->sampler, 0);
    }
	
	// Draw indexed triangles
	GX2DrawIndexedEx(GX2_PRIMITIVE_MODE_TRIANGLES, 
		data->num_indices,
		GX2_INDEX_TYPE_U32,
		data->index_buffer,
		0, 1);
}

Rml::TextureHandle RenderInterface_GX2::LoadTexture(
	Rml::Vector2i& texture_dimensions, 
	const Rml::String& source) 
{
	// TODO: Implement texture loading from file
	// 1. Load image file (use stb_image or similar)
	// 2. Convert to RGBA8
	// 3. Call GenerateTexture
	
	texture_dimensions.x = 0;
	texture_dimensions.y = 0;
	return 0;
}

Rml::TextureHandle RenderInterface_GX2::GenerateTexture(
	Rml::Span<const Rml::byte> source, 
	Rml::Vector2i source_dimensions) 
{
	TextureData* tex_data = new TextureData();
	
	// Create GX2Texture
	GX2Texture* tex = new GX2Texture();
	std::memset(tex, 0, sizeof(GX2Texture));
	tex_data->texture = tex;
	
    // Determine format based on input size
    int bytes_per_pixel = source.size() / (source_dimensions.x * source_dimensions.y);
    
	tex->surface.dim = GX2_SURFACE_DIM_TEXTURE_2D;
	tex->surface.use = GX2_SURFACE_USE_TEXTURE;
	tex->surface.width = source_dimensions.x;
	tex->surface.height = source_dimensions.y;
	tex->surface.depth = 1;
	tex->surface.mipLevels = 1;
	tex->surface.format = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
	tex->surface.aa = GX2_AA_MODE1X;
	tex->surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
	tex->viewNumSlices = 1;
	tex->viewNumMips = 1;
	
	// IMPORTANT: Swizzle for big-endian (ABGR -> RGBA)
	tex->compMap = GX2_COMP_MAP(GX2_SQ_SEL_A, GX2_SQ_SEL_B, GX2_SQ_SEL_G, GX2_SQ_SEL_R);
	
	GX2CalcSurfaceSizeAndAlignment(&tex->surface);
	GX2InitTextureRegs(tex);
	
	// Allocate texture memory
	tex->surface.image = MEMAllocFromMappedMemoryForGX2Ex(tex->surface.imageSize, tex->surface.alignment);
	if (!tex->surface.image) {
		delete tex;
		delete tex_data;
		return 0;
	}
	
	// Copy image data (row by row to account for pitch)
	unsigned char* dst_pixels = (unsigned char*)tex->surface.image;
	const unsigned char* src_pixels = (const unsigned char*)source.data();
	
    if (bytes_per_pixel == 4) {
        // RGBA8 input
        for (int y = 0; y < source_dimensions.y; y++) {
            std::memcpy(dst_pixels + (y * tex->surface.pitch * 4), 
                       src_pixels + (y * source_dimensions.x * 4), 
                       source_dimensions.x * 4);
        }
    } else if (bytes_per_pixel == 1) {
        // A8 input (font) - Expand to RGBA8 (White + Alpha)
        for (int y = 0; y < source_dimensions.y; y++) {
            unsigned char* row_dst = dst_pixels + (y * tex->surface.pitch * 4);
            const unsigned char* row_src = src_pixels + (y * source_dimensions.x);
            
            for (int x = 0; x < source_dimensions.x; x++) {
                unsigned char alpha = row_src[x];
                row_dst[x * 4 + 0] = 255; // R
                row_dst[x * 4 + 1] = 255; // G
                row_dst[x * 4 + 2] = 255; // B
                row_dst[x * 4 + 3] = alpha; // A
            }
        }
    } else {
        WHBLogPrintf("GenerateTexture: Unsupported bytes per pixel: %d", bytes_per_pixel);
        // Fill with magenta to indicate error
        // ...
    }
	
	// Flush CPU cache to GPU
	GX2Invalidate(GX2_INVALIDATE_MODE_CPU_TEXTURE, dst_pixels, tex->surface.imageSize);
	
	// Create sampler
	tex_data->sampler = new GX2Sampler();
	GX2InitSampler(tex_data->sampler, GX2_TEX_CLAMP_MODE_CLAMP, GX2_TEX_XY_FILTER_MODE_LINEAR);
	
	return reinterpret_cast<Rml::TextureHandle>(tex_data);
}

void RenderInterface_GX2::ReleaseTexture(Rml::TextureHandle texture_handle) {
	if (!texture_handle)
		return;
	
	TextureData* data = reinterpret_cast<TextureData*>(texture_handle);
	if (data->texture && data->texture->surface.image) {
		MEMFreeToMappedMemory(data->texture->surface.image);
	}
	delete data->texture;
	delete data->sampler;
	delete data;
}

void RenderInterface_GX2::EnableScissorRegion(bool enable) {
	scissor_enabled = enable;
	// GX2 always has scissor enabled, we'll just set it to full screen when disabled
}

void RenderInterface_GX2::SetScissorRegion(Rml::Rectanglei region) {
	if (scissor_enabled) {
		// GX2 scissor uses same coordinate system as RmlUI (top-left origin)
		GX2SetScissor(region.Left(), region.Top(), region.Width(), region.Height());
	} else {
		// Disable scissor by setting to full viewport
		GX2SetScissor(0, 0, viewport_width, viewport_height);
	}
}

void RenderInterface_GX2::EnableClipMask(bool enable) {
	// TODO: Implement stencil buffer support for clip masking
	// This is an advanced feature, can be left unimplemented initially
}

void RenderInterface_GX2::RenderToClipMask(
	Rml::ClipMaskOperation operation, 
	Rml::CompiledGeometryHandle geometry, 
	Rml::Vector2f translation) 
{
	// TODO: Implement stencil buffer rendering
	// - ClipMaskOperation::Set: Clear stencil and render
	// - ClipMaskOperation::SetInverse: Clear stencil and render inverse
	// - ClipMaskOperation::Intersect: Intersect with existing stencil
}

void RenderInterface_GX2::SetTransform(const Rml::Matrix4f* transform) {
	transform_enabled = (transform != nullptr);
	
	// TODO: Apply transform matrix to vertex shader uniform
	// RmlUi::Matrix4f is column-major by default (unless RMLUI_MATRIX_ROW_MAJOR is defined)
	// if (transform_enabled) {
	//     GX2SetVertexUniformReg(uniform_location, 16, transform->data());
	// } else {
	//     // Use identity matrix
	// }
}

RenderInterface_GX2::TextureData* RenderInterface_GX2::GetTextureData(Rml::TextureHandle texture_handle) {
	if (!texture_handle)
		return nullptr;
	return reinterpret_cast<TextureData*>(texture_handle);
}

