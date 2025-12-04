#include <wups.h>
#include <whb/log.h>
#include <whb/log_udp.h>
#include "Plugin/lifecycle.hpp"

ON_APPLICATION_START()
{
    WHBLogUdpInit();
    GetApp().initialize();
}

ON_APPLICATION_REQUESTS_EXIT()
{
    GetApp().shutdown();
    WHBLogUdpDeinit();
}