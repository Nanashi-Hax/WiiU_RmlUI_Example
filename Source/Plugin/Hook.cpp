#include <wups.h>

#include <whb/log.h>
#include <whb/log_udp.h>

#include <gx2/context.h>
#include <gx2/state.h>

#include "DrawSystem.hpp"
#include "Exception/Patch.hpp"
#include "RmlSystem.hpp"
#include "Backend/RmlUi_Backend.h"
#include "lifecycle.hpp"

INITIALIZE_PLUGIN()
{
    Exception::Patch::apply();
}

ON_APPLICATION_START()
{
    WHBLogUdpInit();
}

ON_APPLICATION_REQUESTS_EXIT()
{
    WHBLogPrintf("Application requests exit");
    GetApp().shutdown();
    WHBLogUdpDeinit();
}

DECL_FUNCTION(void, GX2SetContextState, GX2ContextState *context)
{
    real_GX2SetContextState(context);
    GetApp().getDrawSystem()->setOriginalContext(context);
}

DECL_FUNCTION(void, GX2Init, uint32_t attributes)
{
    real_GX2Init(attributes);
    GetApp().initialize();
    GetApp().getDrawSystem()->initialize();
}

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, GX2ScanTarget scan_target)
{
    App& app = GetApp();
    RmlSystem* rmlSystem = app.getRmlSystem();
    DrawSystem* drawSystem = app.getDrawSystem();
    if (rmlSystem->isInitialized() && rmlSystem->getContext())
    {
        real_GX2SetContextState(drawSystem->getPluginContext());

        drawSystem->update(colorBuffer);
        rmlSystem->draw();
        GX2Flush();

        real_GX2SetContextState(drawSystem->getOriginalContext());
    }
    real_GX2CopyColorBufferToScanBuffer(colorBuffer, scan_target);
}

DECL_FUNCTION(int32_t, VPADRead, VPADChan chan, VPADStatus* buffers, uint32_t count, VPADReadError* error)
{
    App& app = GetApp();
    RmlSystem* rmlSystem = app.getRmlSystem();

    VPADReadError real_error;
    int32_t result = real_VPADRead(chan, buffers, count, &real_error);

    if (result > 0 && real_error == VPAD_READ_SUCCESS && rmlSystem->isInitialized() && rmlSystem->getContext())
    {
        bool consumed = !Backend::ProcessEvents(rmlSystem->getContext(), nullptr, false); 
        (void)consumed;
        
        rmlSystem->getContext()->Update();
    }

    if (error)
    {
        *error = real_error;
    }
    return result;
}

WUPS_MUST_REPLACE(GX2SetContextState, WUPS_LOADER_LIBRARY_GX2, GX2SetContextState);
WUPS_MUST_REPLACE(GX2CopyColorBufferToScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2CopyColorBufferToScanBuffer);
WUPS_MUST_REPLACE(GX2Init, WUPS_LOADER_LIBRARY_GX2, GX2Init);
WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);