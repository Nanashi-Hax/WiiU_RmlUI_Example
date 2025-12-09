#pragma once

#include <RmlUi/Core.h>
#include "RmlUi/Core/Context.h"
#include "Backend/RmlUi_File_WiiU.h"

class RmlSystem
{
public:
    RmlSystem(int width, int height);
    ~RmlSystem();

private:
    Rml::Context* context;
    bool initialized;
    FileInterface_WiiU fileInterface;

public:
    void draw();

    Rml::Context* getContext();
    bool isInitialized();
};