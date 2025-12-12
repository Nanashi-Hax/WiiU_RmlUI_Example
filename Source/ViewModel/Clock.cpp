#include "ViewModel/Clock.hpp"
#include "RmlUi/Core/DataModelHandle.h"
#include <chrono>
#include <cstddef>

namespace ViewModel
{
    Clock::Clock() {}

    void Clock::bind(Rml::DataModelConstructor& constructor)
    {
        constructor.Bind("hour", &hour);
        constructor.Bind("minute", &minute);
        constructor.Bind("second", &second);
    }

    void Clock::update(Rml::DataModelHandle& handle)
    {
        auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());

        hour = Rml::String(std::format("{:%H}", now));
        minute = Rml::String(std::format("{:%M}", now));
        second = Rml::String(std::format("{:%S}", now));

        handle.DirtyVariable("hour");
        handle.DirtyVariable("minute");
        handle.DirtyVariable("second");
    }
}