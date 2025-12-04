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
#include <gx2/state.h>
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
    for (auto& buffer : transform_buffer) {
        if (buffer.buffer) {
            GX2RDestroyBufferEx(&buffer, GX2R_RESOURCE_BIND_NONE);
        }
    }
    transform_buffer.clear();
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
		
        size_t size;
        uint32_t count;

		size = sizeof(float) * 16;
        count = 1;
		GX2InitUniformBuffer(&projection_buffer, size, count);

	}
    
    current_transform_buffer_index = 0;
    
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
	
    // Combine translation and transform into a single matrix
    // Combine translation and transform into a single matrix
    if (shader_group) {
        // Ensure we have a buffer for this draw call
        if ((size_t)current_transform_buffer_index >= transform_buffer.size()) {
            GX2RBuffer new_buffer = {};
            GX2InitUniformBuffer(&new_buffer, sizeof(float) * 16, 1);
            transform_buffer.push_back(new_buffer);
        }

        GX2RBuffer* current_buffer = &transform_buffer[current_transform_buffer_index];

        // Create translation matrix (column-major: translation goes in column 3)
        Rml::Matrix4f translation_matrix = Rml::Matrix4f::Identity();
        translation_matrix[3][0] = translation.x;
        translation_matrix[3][1] = translation.y;
        
        // Combine: first translate, then apply transform
        // Note: matrix multiplication order for column-major is reversed
        Rml::Matrix4f combined = transform_matrix * translation_matrix;
        
        // Send combined matrix to shader
        GX2RSetVertexUniformBlockEx(shader_group, current_buffer, 
            (void*)combined.data(), sizeof(float) * 16, "TransformBlock");
            
        current_transform_buffer_index++;
    }
	
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
	Rml::FileInterface* file_interface = Rml::GetFileInterface();
	Rml::FileHandle file_handle = file_interface->Open(source);
	if (!file_handle) {
		return 0;
	}

	file_interface->Seek(file_handle, 0, SEEK_END);
	size_t file_size = file_interface->Tell(file_handle);
	file_interface->Seek(file_handle, 0, SEEK_SET);

	if (file_size < 18) {
		file_interface->Close(file_handle);
		return 0;
	}

	Rml::byte* buffer = new Rml::byte[file_size];
	file_interface->Read(buffer, file_size, file_handle);
	file_interface->Close(file_handle);

	// TGA Header
	// 0: ID length
	// 1: Color map type
	// 2: Image type (2 = uncompressed RGB)
	// 12: Width (lo)
	// 13: Width (hi)
	// 14: Height (lo)
	// 15: Height (hi)
	// 16: Pixel depth (24 or 32)
	// 17: Image descriptor

	Rml::byte id_length = buffer[0];
	Rml::byte color_map_type = buffer[1];
	Rml::byte image_type = buffer[2];
	
	if (image_type != 2 && image_type != 3) { // Only support uncompressed RGB/RGBA or Grayscale
		delete[] buffer;
		return 0;
	}

	int width = buffer[12] | (buffer[13] << 8);
	int height = buffer[14] | (buffer[15] << 8);
	int pixel_depth = buffer[16];
	int image_descriptor = buffer[17];

	if (width <= 0 || height <= 0 || (pixel_depth != 24 && pixel_depth != 32 && pixel_depth != 8)) {
		delete[] buffer;
		return 0;
	}

	int bytes_per_pixel = pixel_depth / 8;
	size_t image_data_offset = 18 + id_length;
	
	// Skip color map if present (though type 2 shouldn't have one usually, but spec says check)
	if (color_map_type == 1) {
		int color_map_len = buffer[5] | (buffer[6] << 8);
		int color_map_entry_size = buffer[7];
		image_data_offset += color_map_len * (color_map_entry_size / 8);
	}

	if (image_data_offset + width * height * bytes_per_pixel > file_size) {
		delete[] buffer;
		return 0;
	}

	Rml::byte* image_data = buffer + image_data_offset;
	
	// Convert to RGBA8
	int dest_bytes_per_pixel = 4;
	Rml::byte* dest_buffer = new Rml::byte[width * height * dest_bytes_per_pixel];

	bool flip_vertical = !(image_descriptor & 0x20); // Bit 5 set = top-to-bottom

	for (int y = 0; y < height; y++) {
		int src_y = flip_vertical ? (height - 1 - y) : y;
		Rml::byte* src_row = image_data + (src_y * width * bytes_per_pixel);
		Rml::byte* dest_row = dest_buffer + (y * width * dest_bytes_per_pixel);

		for (int x = 0; x < width; x++) {
			Rml::byte* src_pixel = src_row + (x * bytes_per_pixel);
			Rml::byte* dest_pixel = dest_row + (x * dest_bytes_per_pixel);

			if (bytes_per_pixel == 3) {
				// BGR -> RGBA
				dest_pixel[0] = src_pixel[2]; // R
				dest_pixel[1] = src_pixel[1]; // G
				dest_pixel[2] = src_pixel[0]; // B
				dest_pixel[3] = 255;          // A
			} else if (bytes_per_pixel == 4) {
				// BGRA -> RGBA
				dest_pixel[0] = src_pixel[2]; // R
				dest_pixel[1] = src_pixel[1]; // G
				dest_pixel[2] = src_pixel[0]; // B
				dest_pixel[3] = src_pixel[3]; // A
			} else if (bytes_per_pixel == 1) {
				// Grayscale -> RGBA
				dest_pixel[0] = src_pixel[0];
				dest_pixel[1] = src_pixel[0];
				dest_pixel[2] = src_pixel[0];
				dest_pixel[3] = 255;
			}
		}
	}

	texture_dimensions.x = width;
	texture_dimensions.y = height;

	Rml::TextureHandle handle = GenerateTexture(Rml::Span<const Rml::byte>(dest_buffer, width * height * 4), texture_dimensions);

	delete[] dest_buffer;
	delete[] buffer;

	return handle;
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
	
	// Standard RGBA mapping
	tex->compMap = GX2_COMP_MAP(GX2_SQ_SEL_R, GX2_SQ_SEL_G, GX2_SQ_SEL_B, GX2_SQ_SEL_A);
	
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
	
    if (transform) {
        transform_matrix = *transform;
    } else {
        transform_matrix = Rml::Matrix4f::Identity();
    }
}

RenderInterface_GX2::TextureData* RenderInterface_GX2::GetTextureData(Rml::TextureHandle texture_handle) {
	if (!texture_handle)
		return nullptr;
	return reinterpret_cast<TextureData*>(texture_handle);
}

