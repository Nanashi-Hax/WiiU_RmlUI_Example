#include "texture.hpp"

#include <cstdint>
#include <cstring>
#include <format>
#include <memory>
#include <string>

#include <coreinit/debug.h>
#include <gx2/mem.h>
#include <gx2/sampler.h>
#include <gx2/texture.h>
#include <gx2/utils.h>
#include <gx2r/surface.h>

#include <memory/mappedmemory.h>

#include "imgui.h"

#include "../../imgui_backend/imgui_impl_gx2.h"

ImGui_ImplGX2_Texture* gDummyTexture = nullptr;

ImGui_ImplGX2_Texture* CreateGX2Texture(int width, int height)
{
    ImGui_ImplGX2_Texture* imTexture = new ImGui_ImplGX2_Texture();
    GX2Texture* texture = new GX2Texture();
    GX2Sampler* sampler = new GX2Sampler();
    imTexture->Texture = texture;
    imTexture->Sampler = sampler;

    memset(texture, 0, sizeof(GX2Texture));

    texture->surface.dim = GX2_SURFACE_DIM_TEXTURE_2D;
    texture->surface.use = GX2_SURFACE_USE_TEXTURE;
    texture->surface.width = width;
    texture->surface.height = height;
    texture->surface.depth = 1;
    texture->surface.mipLevels = 1;
    texture->surface.format = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
    texture->surface.aa = GX2_AA_MODE1X;
    texture->surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;

    texture->viewNumSlices = 1;
    texture->viewNumMips = 1;
    texture->compMap = GX2_COMP_MAP(GX2_SQ_SEL_R, GX2_SQ_SEL_G, GX2_SQ_SEL_B, GX2_SQ_SEL_A);

    GX2RCreateSurface
    (
        &texture->surface,
        GX2R_RESOURCE_BIND_TEXTURE |
        GX2R_RESOURCE_USAGE_GPU_WRITE |
        GX2R_RESOURCE_USAGE_GPU_READ |
        GX2R_RESOURCE_USAGE_CPU_WRITE |
        GX2R_RESOURCE_USAGE_CPU_READ
    );
    GX2InitTextureRegs(texture);

    GX2InitSampler(sampler,
                   GX2_TEX_CLAMP_MODE_CLAMP,
                   GX2_TEX_XY_FILTER_MODE_LINEAR);

    return imTexture;
}

ImGui_ImplGX2_Texture* CreateGX2Texture(uint8_t* data, int width, int height)
{
    ImGui_ImplGX2_Texture* imTexture = new ImGui_ImplGX2_Texture();
    GX2Texture* texture = new GX2Texture();
    GX2Sampler* sampler = new GX2Sampler();
    imTexture->Texture = texture;
    imTexture->Sampler = sampler;

    memset(texture, 0, sizeof(GX2Texture));

    texture->surface.dim = GX2_SURFACE_DIM_TEXTURE_2D;
    texture->surface.use = GX2_SURFACE_USE_TEXTURE;
    texture->surface.width = width;
    texture->surface.height = height;
    texture->surface.depth = 1;
    texture->surface.mipLevels = 1;
    texture->surface.format = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
    texture->surface.aa = GX2_AA_MODE1X;
    texture->surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;

    texture->viewNumSlices = 1;
    texture->viewNumMips = 1;
    texture->compMap = GX2_COMP_MAP(GX2_SQ_SEL_R, GX2_SQ_SEL_G, GX2_SQ_SEL_B, GX2_SQ_SEL_A);

    GX2RCreateSurface
    (
        &texture->surface,
        GX2R_RESOURCE_BIND_TEXTURE |
        GX2R_RESOURCE_USAGE_GPU_WRITE |
        GX2R_RESOURCE_USAGE_GPU_READ |
        GX2R_RESOURCE_USAGE_CPU_WRITE |
        GX2R_RESOURCE_USAGE_CPU_READ
    );
    GX2InitTextureRegs(texture);

    uint8_t* dst = (uint8_t*) texture->surface.image;
    uint8_t* src = (uint8_t*) data;

    for (int y = 0; y < height; y++)
    {
        memcpy(dst + y * texture->surface.pitch * 4, src + y * width * 4, width * 4);
    }

    GX2Invalidate(GX2_INVALIDATE_MODE_CPU_TEXTURE, dst, texture->surface.imageSize);

    GX2InitSampler(sampler,
                   GX2_TEX_CLAMP_MODE_CLAMP,
                   GX2_TEX_XY_FILTER_MODE_LINEAR);

    return imTexture;
}

void DestroyGX2Texture(ImGui_ImplGX2_Texture* texture)
{
    if (texture)
    {
        // サーフェスの画像メモリを解放
        if (texture->Texture->surface.image)
        {
            GX2RDestroySurfaceEx
            (
                &texture->Texture->surface,
                GX2R_RESOURCE_BIND_TEXTURE |
                GX2R_RESOURCE_USAGE_GPU_WRITE |
                GX2R_RESOURCE_USAGE_GPU_READ |
                GX2R_RESOURCE_USAGE_CPU_WRITE |
                GX2R_RESOURCE_USAGE_CPU_READ
            );
            texture->Texture->surface.image = nullptr;
        }

        delete texture->Texture;
        texture->Texture = nullptr;

        delete texture->Sampler;
        texture->Sampler = nullptr;

        delete texture;
    }
}

GX2ColorBuffer* CreateGX2ColorBuffer(int width, int height)
{
    GX2ColorBuffer* colorBuffer = new GX2ColorBuffer();
    memset(colorBuffer, 0, sizeof(GX2ColorBuffer));

    colorBuffer->surface.dim = GX2_SURFACE_DIM_TEXTURE_2D;
    colorBuffer->surface.use = GX2_SURFACE_USE_COLOR_BUFFER;
    colorBuffer->surface.width = width;
    colorBuffer->surface.height = height;
    colorBuffer->surface.depth = 1;
    colorBuffer->surface.mipLevels = 1;
    colorBuffer->surface.format = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
    colorBuffer->surface.aa = GX2_AA_MODE1X;
    colorBuffer->surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;

    GX2RCreateSurface
    (
        &colorBuffer->surface,
        GX2R_RESOURCE_BIND_COLOR_BUFFER |
        GX2R_RESOURCE_USAGE_GPU_WRITE |
        GX2R_RESOURCE_USAGE_GPU_READ |
        GX2R_RESOURCE_USAGE_CPU_WRITE |
        GX2R_RESOURCE_USAGE_CPU_READ
    );
    GX2InitColorBufferRegs(colorBuffer);

    return colorBuffer;
}

void DestroyGX2ColorBuffer(GX2ColorBuffer* colorBuffer)
{
    if (colorBuffer)
    {
        // サーフェスの画像メモリを解放
        if (colorBuffer->surface.image)
        {
            GX2RDestroySurfaceEx
            (
                &colorBuffer->surface,
                GX2R_RESOURCE_BIND_TEXTURE |
                GX2R_RESOURCE_USAGE_GPU_WRITE |
                GX2R_RESOURCE_USAGE_GPU_READ |
                GX2R_RESOURCE_USAGE_CPU_WRITE |
                GX2R_RESOURCE_USAGE_CPU_READ
            );
            colorBuffer->surface.image = nullptr;
        }

        delete colorBuffer;
    }
}