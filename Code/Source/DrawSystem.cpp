#include "DrawSystem.hpp"
#include <whb/log.h>
#include <whb/log_udp.h>
#include <memory/mappedmemory.h>
#include <gx2/enum.h>
#include <cstring>

DrawSystem::DrawSystem()
{
    // Allocate overlay context state early
    // Use MEMAllocFromMappedMemoryForGX2Ex as suggested by reference
    plugin = (GX2ContextState *)MEMAllocFromMappedMemoryForGX2Ex
    (
        sizeof(GX2ContextState),
        GX2_CONTEXT_STATE_ALIGNMENT
    );

    if (plugin == nullptr)
    {
        WHBLogPrintf("Failed to allocate gOverlayContextState");
    }
    else
    {
        memset(plugin, 0, sizeof(GX2ContextState));
    }
}

DrawSystem::~DrawSystem()
{
    MEMFreeToMappedMemory(plugin);
}

GX2ContextState* DrawSystem::getOriginal()
{
    return original;
}

GX2ContextState* DrawSystem::getPlugin()
{
    return plugin;
}