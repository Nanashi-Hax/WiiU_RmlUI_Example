#pragma once

#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include "Backend/RmlUi_File_WiiU.h"
#include "RmlUi/Config/Config.h"
#include "RmlUi/Core/DataModelHandle.h"
#include "ViewModel/Clock.hpp"

class RmlSystem
{ 
public:
    RmlSystem(int width, int height);
    ~RmlSystem();

private:
    Rml::Context* context;
    bool initialized;
    FileInterface_WiiU fileInterface;

    ViewModel::Clock* clock;
    Rml::DataModelHandle clockModel;

public:
    void draw();

    Rml::Context* getContext();
    bool isInitialized();
};