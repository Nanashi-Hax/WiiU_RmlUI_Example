#include <stdexcept>
#include <wups.h>
#include <memory/mappedmemory.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>

#include <gx2/context.h>
#include <gx2/enum.h>
#include <whb/log_udp.h>
#include <whb/log.h>

#include <RmlUi/Core.h>

WUPS_PLUGIN_NAME("RmlUI Example");
WUPS_PLUGIN_DESCRIPTION("Overlay Plugin");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Nanashi-Hax");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE("RmlUI");
