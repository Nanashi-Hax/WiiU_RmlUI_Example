#include <wups.h>

#include <whb/log.h>
#include <whb/log_udp.h>

#include <gx2/context.h>
#include <gx2/state.h>

#include "DrawSystem.hpp"
#include "Exception/Patch.hpp"
#include "InputSystem.hpp"
#include "RmlSystem.hpp"
#include "gx2/enum.h"
#include "lifecycle.hpp"
#include "vpad/input.h"

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
static inline void GX2InitColorBuffer(GX2ColorBuffer* colorBuffer, GX2SurfaceDim dim, uint32_t width, uint32_t height,
                                      uint32_t depth, GX2SurfaceFormat format, GX2AAMode aa, GX2TileMode tilemode,
                                      uint32_t swizzle,
                                      void* aaBuffer, uint32_t aaSize)
{
    colorBuffer->surface.dim = dim;
    colorBuffer->surface.width = width;
    colorBuffer->surface.height = height;
    colorBuffer->surface.depth = depth;
    colorBuffer->surface.mipLevels = 1;
    colorBuffer->surface.format = format;
    WHBLogPrintf("%d", format);
    colorBuffer->surface.aa = aa;
    colorBuffer->surface.use = GX2_SURFACE_USE_COLOR_BUFFER;
    colorBuffer->surface.imageSize = 0;
    colorBuffer->surface.image = nullptr;
    colorBuffer->surface.mipmapSize = 0;
    colorBuffer->surface.mipmaps = nullptr;
    colorBuffer->surface.tileMode = tilemode;
    colorBuffer->surface.swizzle = swizzle;
    colorBuffer->surface.alignment = 0;
    colorBuffer->surface.pitch = 0;
    uint32_t i;
    for (i = 0; i < 13; i++)
        colorBuffer->surface.mipLevelOffset[i] = 0;
    colorBuffer->viewMip = 0;
    colorBuffer->viewFirstSlice = 0;
    colorBuffer->viewNumSlices = depth;
    colorBuffer->aaBuffer = aaBuffer;
    colorBuffer->aaSize = aaSize;
    for (i = 0; i < 5; i++)
        colorBuffer->regs[i] = 0;

    GX2CalcSurfaceSizeAndAlignment(&colorBuffer->surface);
    GX2InitColorBufferRegs(colorBuffer);
}
DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, GX2ScanTarget scanTarget)
{
    App& app = GetApp();
    RmlSystem* rmlSystem = app.getRmlSystem();
    DrawSystem* drawSystem = app.getDrawSystem();
    if (rmlSystem->isInitialized() && rmlSystem->getContextTV() && rmlSystem->getContextDRC())
    {
        real_GX2SetContextState(drawSystem->getPluginContext());

        GX2ColorBuffer cb;
        GX2InitColorBuffer(&cb,
                       colorBuffer->surface.dim,
                       colorBuffer->surface.width,
                       colorBuffer->surface.height,
                       colorBuffer->surface.depth,
                       colorBuffer->surface.format,
                       colorBuffer->surface.aa,
                       colorBuffer->surface.tileMode,
                       colorBuffer->surface.swizzle,
                       colorBuffer->aaBuffer,
                       colorBuffer->aaSize);

        cb.surface.image = colorBuffer->surface.image;

        drawSystem->update(&cb);
        rmlSystem->draw(scanTarget);
        GX2Flush();

        real_GX2SetContextState(drawSystem->getOriginalContext());
        real_GX2CopyColorBufferToScanBuffer(&cb, scanTarget);
        return;
    }
    real_GX2CopyColorBufferToScanBuffer(colorBuffer, scanTarget);
}

DECL_FUNCTION(int32_t, VPADRead, VPADChan chan, VPADStatus* buffers, uint32_t count, VPADReadError* error)
{
    App& app = GetApp();
    RmlSystem* rmlSystem = app.getRmlSystem();
    InputSystem* inputSystem = app.getInputSystem();

    VPADReadError real_error;
    int32_t result = real_VPADRead(chan, buffers, count, &real_error);

    if (result > 0 && real_error == VPAD_READ_SUCCESS && rmlSystem->isInitialized())
    {
        VPADStatus status = buffers[0];
        inputSystem->push(status);

        rmlSystem->pollKeyEvent();
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