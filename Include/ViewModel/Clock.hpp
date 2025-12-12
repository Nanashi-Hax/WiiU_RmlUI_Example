#pragma once

#include "ViewModel/Interface.hpp"

namespace ViewModel
{
    class Clock : Interface
    {
    private:
        Rml::String hour;
        Rml::String minute;
        Rml::String second;
        
    public:
        Clock();

        void bind(Rml::DataModelConstructor& constructor) override;
        void update(Rml::DataModelHandle& handle) override;
    };
}