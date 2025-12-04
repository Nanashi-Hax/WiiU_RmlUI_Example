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

#include "Draw.hpp"
#include "Plugin/lifecycle.hpp"

namespace
{    
    bool gOverlayContextInitialized = false;
}

DECL_FUNCTION(void, GX2SetContextState, GX2ContextState *context)
{
    real_GX2SetContextState(context);
    UpdateOriginalContext(context);
}

DECL_FUNCTION(void, GX2SetupContextStateEx, GX2ContextState *context, BOOL unk1)
{
    real_GX2SetupContextStateEx(context, unk1);
    UpdateOriginalContext(context);
}

static void InitOverlayContext()
{
    if (GetApp().getDrawSystem()->getPlugin() && !gOverlayContextInitialized)
    {
        // Initialize the context state
        real_GX2SetupContextStateEx(GetApp().getDrawSystem()->getPlugin(), GX2_TRUE);
        
        // Invalidate cache to ensure GPU sees the data
        DCInvalidateRange(GetApp().getDrawSystem()->getPlugin(), sizeof(GX2ContextState));
        
        gOverlayContextInitialized = true;
    }
}

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, GX2ScanTarget scan_target) {
    
    // Draw our overlay before the copy happens
    if (GetApp().getRmlSystem()->isInitialized() && GetApp().getRmlSystem()->getContext()) {
        // Initialize overlay context on first call
        if (!gOverlayContextInitialized) {
            InitOverlayContext();
        }

        // Set our overlay context
        real_GX2SetContextState(GetApp().getDrawSystem()->getPlugin());

        SetDrawSettings(colorBuffer);

        // Render RmlUi
        GetApp().getRmlSystem()->draw();

        GX2Flush();

        // Restore original context
        real_GX2SetContextState(GetApp().getDrawSystem()->getOriginal());
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

    if (result > 0 && real_error == VPAD_READ_SUCCESS && GetApp().getRmlSystem()->isInitialized() && GetApp().getRmlSystem()->getContext())
    {
        // Feed input to RmlUi
        bool consumed = !Backend::ProcessEvents(GetApp().getRmlSystem()->getContext(), nullptr, false); 
        (void)consumed; 
        
        GetApp().getRmlSystem()->getContext()->Update();
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
