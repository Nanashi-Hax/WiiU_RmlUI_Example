#include "Draw.hpp"
#include <gx2/context.h>
#include <gx2/display.h>
#include <gx2/enum.h>
#include <gx2/mem.h>
#include <gx2/registers.h>
#include <gx2/state.h>
#include <gx2/surface.h>
#include <gx2/swap.h>
#include <gx2/clear.h>

GX2ContextState* gOriginalContextState = nullptr;

void UpdateOriginalContext(GX2ContextState *context)
{
    gOriginalContextState = context;
}

void SetDrawSettings(const GX2ColorBuffer *colorBuffer)
{
    GX2SetDefaultState();

    // Setup render target
    GX2SetColorBuffer(colorBuffer, GX2_RENDER_TARGET_0);
    GX2SetViewport(0.0f, 0.0f, colorBuffer->surface.width, colorBuffer->surface.height, 0.0f, 1.0f);
    GX2SetScissor(0, 0, colorBuffer->surface.width, colorBuffer->surface.height);

    // Setup blending/depth for overlay
    GX2SetDepthOnlyControl(GX2_FALSE, GX2_FALSE, GX2_COMPARE_FUNC_NEVER);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, GX2_ENABLE, GX2_DISABLE, GX2_ENABLE);
    // Enable alpha blending
    GX2SetBlendControl(GX2_RENDER_TARGET_0, 
                        GX2_BLEND_MODE_SRC_ALPHA, GX2_BLEND_MODE_INV_SRC_ALPHA, GX2_BLEND_COMBINE_MODE_ADD,
                        TRUE,
                        GX2_BLEND_MODE_SRC_ALPHA, GX2_BLEND_MODE_INV_SRC_ALPHA, GX2_BLEND_COMBINE_MODE_ADD);
}