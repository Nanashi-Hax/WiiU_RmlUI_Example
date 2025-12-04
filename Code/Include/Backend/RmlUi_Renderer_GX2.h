/*
 * RmlUi Wii U GX2 Renderer
 * 
 * This is a template implementation for rendering RmlUi using Wii U's GX2 graphics API.
 */

#ifndef RMLUI_BACKENDS_RENDERER_GX2_H
#define RMLUI_BACKENDS_RENDERER_GX2_H

#include <RmlUi/Core/RenderInterface.h>
#include <gx2/texture.h>
#include <gx2/sampler.h>
#include <whb/gfx.h>
#include <cstdint>

class RenderInterface_GX2 : public Rml::RenderInterface {
public:
	// Texture data structure
	struct TextureData {
		GX2Texture* texture;
		GX2Sampler* sampler;
		
		TextureData() : texture(nullptr), sampler(nullptr) {}
	};

	RenderInterface_GX2();
	~RenderInterface_GX2();

	// The viewport should be updated whenever the screen size changes.
	void SetViewport(int viewport_width, int viewport_height);

	// Sets up GX2 states for taking rendering commands from RmlUi.
	void BeginFrame();
	void EndFrame();

	// Optional, can be used to clear the framebuffer.
	void Clear();

	// -- Inherited from Rml::RenderInterface --

	Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;
	void RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) override;

	Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
	Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;
	void ReleaseTexture(Rml::TextureHandle texture_handle) override;

	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(Rml::Rectanglei region) override;

	void EnableClipMask(bool enable) override;
	void RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation) override;

	void SetTransform(const Rml::Matrix4f* transform) override;

	// Helper method for font engine to get texture data
	TextureData* GetTextureData(Rml::TextureHandle texture_handle);

private:
	// Geometry data structure matching RmlUi::Vertex layout
	struct GeometryData {
		void* vertex_buffer;      // GX2 vertex buffer
		void* index_buffer;       // GX2 index buffer
		uint32_t num_vertices;
		uint32_t num_indices;
		
		GeometryData() : vertex_buffer(nullptr), index_buffer(nullptr), 
		                 num_vertices(0), num_indices(0) {}
	};

	int viewport_width = 1280;
	int viewport_height = 720;
	bool scissor_enabled = false;
	bool transform_enabled = false;
    GX2RBuffer projection_buffer = {};
    Rml::Vector<GX2RBuffer> transform_buffer = {};
    int current_transform_buffer_index = 0;
    
    Rml::Matrix4f transform_matrix = Rml::Matrix4f::Identity();

	// Shader group for rendering (similar to ImGui implementation)
	WHBGfxShaderGroup* shader_group = nullptr;
    
    // Default white texture for untextured geometry
    TextureData* default_texture = nullptr;
	
	// Helper to set up render state
	void SetupRenderState();
};

#endif
