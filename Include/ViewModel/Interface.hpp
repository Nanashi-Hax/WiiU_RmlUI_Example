#pragma once

#include "RmlUi/Core/DataModelHandle.h"

namespace ViewModel
{
    class Interface
    {
    public:
        virtual ~Interface() = default;
        virtual void bind(Rml::DataModelConstructor& constructor) = 0;
        virtual void update(Rml::DataModelHandle& handle) = 0;
    };
}