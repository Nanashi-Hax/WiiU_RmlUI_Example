#include "DrawSystem.hpp"
#include "gx2/enum.h"
#include <stdexcept>
#include <memory/mappedmemory.h>
#include <gx2/display.h>
#include <gx2/registers.h>
#include <coreinit/cache.h>

DrawSystem::DrawSystem() : originalContext(nullptr), pluginContext(nullptr), isInitialized(false)
{
    pluginContext = reinterpret_cast<GX2ContextState*>(MEMAllocFromMappedMemoryForGX2Ex(sizeof(GX2ContextState), GX2_CONTEXT_STATE_ALIGNMENT));
    if (pluginContext == nullptr)
    {
        throw std::runtime_error("Failed to allocate pluginContext");
    }
}

DrawSystem::~DrawSystem()
{
    MEMFreeToMappedMemory(pluginContext);
}

GX2ContextState* DrawSystem::getOriginalContext()
{
    return originalContext;
}

GX2ContextState* DrawSystem::getPluginContext()
{
    return pluginContext;
}

void DrawSystem::setOriginalContext(GX2ContextState* context)
{
    originalContext = context;
}

void DrawSystem::update(const GX2ColorBuffer *colorBuffer)
{
    GX2SetDefaultState();

    GX2SetColorBuffer(colorBuffer, GX2_RENDER_TARGET_0);
    GX2SetViewport(0.0f, 0.0f, colorBuffer->surface.width, colorBuffer->surface.height, 0.0f, 1.0f);
    GX2SetScissor(0, 0, colorBuffer->surface.width, colorBuffer->surface.height);
    GX2SetDepthOnlyControl(GX2_FALSE, GX2_FALSE, GX2_COMPARE_FUNC_NEVER);
    GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_FUNC_GREATER, 0.0f);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, GX2_ENABLE, GX2_DISABLE, GX2_ENABLE);
    GX2SetBlendControl
    (
        GX2_RENDER_TARGET_0,
        GX2_BLEND_MODE_ONE,
        GX2_BLEND_MODE_INV_SRC_ALPHA,
        GX2_BLEND_COMBINE_MODE_ADD,
        TRUE,
        GX2_BLEND_MODE_ONE,
        GX2_BLEND_MODE_INV_SRC_ALPHA,
        GX2_BLEND_COMBINE_MODE_ADD
    );
}

void DrawSystem::initialize()
{
    GX2SetupContextStateEx(pluginContext, GX2_TRUE);
    DCInvalidateRange(pluginContext, sizeof(GX2ContextState));
    isInitialized = true;
}