#include <wups.h>
#include <gx2/context.h>
#include <gx2/display.h>
#include <gx2/enum.h>
#include <gx2/mem.h>
#include <gx2/registers.h>
#include <gx2/state.h>
#include <gx2/surface.h>
#include <gx2/swap.h>
#include <gx2/clear.h>
#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <memory/mappedmemory.h>

#include <whb/log.h>
#include <RmlUi/Core.h>
#include "RmlUi_Backend.h"

// External context from main.cpp
extern Rml::Context* g_RmlContext;
extern bool g_RmlInitialized;
extern GX2ContextState* gOverlayContextState;

namespace
{
    GX2ContextState* gOriginalContextState = nullptr;
    bool gOverlayContextInitialized = false;
    bool gFirstCopyCall = false;
}

// GX2SetContextState Hook
DECL_FUNCTION(void, GX2SetContextState, GX2ContextState *curContext)
{
    static bool first_call = true;
    if (first_call) {
        WHBLogPrintf("GX2SetContextState hook called for first time");
        first_call = false;
    }
    real_GX2SetContextState(curContext);
    gOriginalContextState = curContext;
}

// GX2SetupContextStateEx Hook
DECL_FUNCTION(void, GX2SetupContextStateEx, GX2ContextState *state, BOOL unk1)
{
    static bool first_call = true;
    if (first_call) {
        WHBLogPrintf("GX2SetupContextStateEx hook called for first time");
        first_call = false;
    }
    real_GX2SetupContextStateEx(state, unk1);
    gOriginalContextState = state;
}

static void InitOverlayContext() {
    if (gOverlayContextState && !gOverlayContextInitialized) {
        // Initialize the context state
        real_GX2SetupContextStateEx(gOverlayContextState, GX2_TRUE);
        
        // Invalidate cache to ensure GPU sees the data
        DCInvalidateRange(gOverlayContextState, sizeof(GX2ContextState));
        
        gOverlayContextInitialized = true;
    }
}

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, GX2ScanTarget scan_target) {
    if (!gFirstCopyCall) {
        gFirstCopyCall = true;
    }
    
    // Draw our overlay before the copy happens
    if (g_RmlInitialized && g_RmlContext) {
        // Initialize overlay context on first call
        if (!gOverlayContextInitialized) {
            InitOverlayContext();
        }

        // Set our overlay context
        real_GX2SetContextState(gOverlayContextState);

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

        // Render RmlUi
        Backend::BeginFrame();
        g_RmlContext->Update();
        g_RmlContext->Render();
        Backend::PresentFrame();

        GX2Flush();

        // Restore original context
        real_GX2SetContextState(gOriginalContextState);
    }
    
    real_GX2CopyColorBufferToScanBuffer(colorBuffer, scan_target);
}

// GX2SwapScanBuffers Hook (alternative render point)
DECL_FUNCTION(void, GX2SwapScanBuffers)
{
    static bool first_call = true;
    if (first_call) {
        WHBLogPrintf("GX2SwapScanBuffers hook called for first time");
        first_call = false;
    }
    real_GX2SwapScanBuffers();
}

// GX2Init Hook
DECL_FUNCTION(void, GX2Init, uint32_t attributes)
{
    WHBLogPrintf("GX2Init hook called");
    real_GX2Init(attributes);
}

// VPADRead Hook
DECL_FUNCTION(int32_t, VPADRead, VPADChan chan, VPADStatus* buffers, uint32_t count, VPADReadError* error)
{
    static bool first_call = true;
    if (first_call) {
        WHBLogPrintf("VPADRead hook called for first time");
        first_call = false;
    }
    
    VPADReadError real_error;
    int32_t result = real_VPADRead(chan, buffers, count, &real_error);

    if (result > 0 && real_error == VPAD_READ_SUCCESS && g_RmlInitialized && g_RmlContext)
    {
        // Feed input to RmlUi
        bool consumed = !Backend::ProcessEvents(g_RmlContext, nullptr, false); 
        (void)consumed; 
        
        g_RmlContext->Update();
    }

    if (error)
    {
        *error = real_error;
    }
    return result;
}

WUPS_MUST_REPLACE(GX2SetContextState, WUPS_LOADER_LIBRARY_GX2, GX2SetContextState);
WUPS_MUST_REPLACE(GX2SetupContextStateEx, WUPS_LOADER_LIBRARY_GX2, GX2SetupContextStateEx);
WUPS_MUST_REPLACE(GX2CopyColorBufferToScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2CopyColorBufferToScanBuffer);
WUPS_MUST_REPLACE(GX2SwapScanBuffers, WUPS_LOADER_LIBRARY_GX2, GX2SwapScanBuffers);
WUPS_MUST_REPLACE(GX2Init, WUPS_LOADER_LIBRARY_GX2, GX2Init);
WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);
