#include <wups.h>
#include <memory/mappedmemory.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>

#include <gx2/context.h>
#include <gx2/enum.h>
#include <whb/log_udp.h>
#include <whb/log.h>

#include <RmlUi/Core.h>
#include "RmlUi_Backend.h"
#include "RmlUi_File_WiiU.h"

WUPS_PLUGIN_NAME("RmlUI Example");
WUPS_PLUGIN_DESCRIPTION("Overlay Plugin");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Nanashi-Hax");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE("RmlUI");

// Globals for function_patches.cpp
Rml::Context* g_RmlContext = nullptr;
bool g_RmlInitialized = false;
GX2ContextState* gOverlayContextState = nullptr;

INITIALIZE_PLUGIN()
{
    // Allocate overlay context state early
    // Use MEMAllocFromMappedMemoryForGX2Ex as suggested by reference
    gOverlayContextState = (GX2ContextState *)MEMAllocFromMappedMemoryForGX2Ex
    (
        sizeof(GX2ContextState),
        GX2_CONTEXT_STATE_ALIGNMENT
    );

    if (gOverlayContextState == nullptr)
    {
        WHBLogPrintf("Failed to allocate gOverlayContextState");
    } else {
        // Zero out the memory
        memset(gOverlayContextState, 0, sizeof(GX2ContextState));
    }
}

ON_APPLICATION_START()
{
    WHBLogUdpInit();
    WHBLogPrintf("Initializing RmlUi Plugin...");

    // Initialize Backend (System, Render Interface, Window)
    // Wii U GamePad resolution is 854x480, but we render at 1280x720 and scale down usually.
    if (!Backend::Initialize("RmlUi Example", 1280, 720, true)) {
        WHBLogPrintf("Backend::Initialize failed");
        return;
    }

    // Set RmlUi interfaces
    static FileInterface_WiiU file_interface;
    Rml::SetFileInterface(&file_interface);
    Rml::SetSystemInterface(Backend::GetSystemInterface());
    Rml::SetRenderInterface(Backend::GetRenderInterface());

    // Initialize RmlUi
    Rml::Initialise();

    // Create a context
    g_RmlContext = Rml::CreateContext("main", Rml::Vector2i(1280, 720));
    if (!g_RmlContext) {
        WHBLogPrintf("Rml::CreateContext failed");
        Rml::Shutdown();
        Backend::Shutdown();
        return;
    }

    // Load fonts
    // You need to put a font file at this path!
    if (!Rml::LoadFontFace("fs:/vol/external01/wiiu/plugins/RmlUI/fonts/Lato-Regular.ttf")) {
        WHBLogPrintf("Failed to load font: fs:/vol/external01/wiiu/plugins/RmlUI/fonts/Lato-Regular.ttf");
    } else {
        WHBLogPrintf("Font loaded successfully");
    }

    // Load the Hello World document
    // Using absolute path on SD card for safety
    const char* docPath = "fs:/vol/external01/wiiu/plugins/RmlUI/hello.rml";
    WHBLogPrintf("Loading document from: %s", docPath);
    
    Rml::ElementDocument* document = g_RmlContext->LoadDocument(docPath);
    if (document) {
        document->Show();
        WHBLogPrintf("Document loaded successfully");
        WHBLogPrintf("Document ID: %s", document->GetId().c_str());
        WHBLogPrintf("Document has %d children", document->GetNumChildren());
        
        // Log first few children
        for (int i = 0; i < document->GetNumChildren() && i < 5; i++) {
            auto* child = document->GetChild(i);
            WHBLogPrintf("  Child %d: tag=%s, id=%s", i, child->GetTagName().c_str(), child->GetId().c_str());
        }
        
        // Check for body element
        auto* body = document->GetElementById("body");
        if (!body) {
            // Try getting by tag name
            body = document->GetFirstChild();
            WHBLogPrintf("Body element: %s", body ? "found (first child)" : "NOT FOUND");
        } else {
            WHBLogPrintf("Body element: found by ID");
        }
        
        // Check context dimensions
        auto ctx_dims = g_RmlContext->GetDimensions();
        WHBLogPrintf("Context dimensions: %dx%d", ctx_dims.x, ctx_dims.y);
        
        // Check document dimensions
        WHBLogPrintf("Document offset: %.1f, %.1f", document->GetAbsoluteOffset().x, document->GetAbsoluteOffset().y);
        WHBLogPrintf("Document client size: %.1f x %.1f", document->GetClientWidth(), document->GetClientHeight());
    } else {
        WHBLogPrintf("Document load failed");
    }

    g_RmlInitialized = true;
    WHBLogPrintf("RmlUi Initialized");
}

ON_APPLICATION_REQUESTS_EXIT()
{
    WHBLogPrintf("Shutting down RmlUi Plugin...");
    
    g_RmlInitialized = false;

    // Shutdown
    Rml::Shutdown();
    Backend::Shutdown();

    WHBLogUdpDeinit();
}
