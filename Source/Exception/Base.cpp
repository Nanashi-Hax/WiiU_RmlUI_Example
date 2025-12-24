#include <string>
#include <format>

#include <coreinit/context.h>
#include <coreinit/debug.h>

#include "Exception/Base.hpp"
#include <whb/log.h>

namespace Exception
{
    void Base::panic(std::string name, OSContext* context)
    {
        std::string message = std::format("{0} Exception occurred", name);
        WHBLogPrintf("%s", message.c_str());
        OSFatal(message.c_str());
    }

    bool Base::handle(OSContext* context)
    {
        panic(name(), context);
        return false;
    }
}