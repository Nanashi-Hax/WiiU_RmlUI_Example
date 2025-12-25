#pragma once

#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include <vpad/input.h>
#include "Backend/RmlUi_File_WiiU.h"
#include "Backend/RmlUi_Renderer_GX2.h"
#include "Backend/RmlUi_Platform_WiiU.h"
#include "RmlUi/Core/DataModelHandle.h"
#include "ViewModel/Clock.hpp"
#include "gx2/enum.h"

class RmlSystem
{ 
public:
    RmlSystem(int width, int height);
    ~RmlSystem();

private:
    Rml::Context* contextTV;
    Rml::Context* contextDRC;
    bool initialized;
    FileInterface_WiiU fileInterface;
    SystemInterface_WiiU systemInterface;
    RenderInterface_GX2 renderInterfaceTV;
    RenderInterface_GX2 renderInterfaceDRC;

    ViewModel::Clock* clockTV;
    ViewModel::Clock* clockDRC;
    Rml::DataModelHandle clockModelTV;
    Rml::DataModelHandle clockModelDRC;

public:
    void draw(GX2ScanTarget target);
    void pollKeyEvent();
    static void processKey(Rml::Context* context, VPADStatus s);

    Rml::Context* getContextTV();
    Rml::Context* getContextDRC();
    bool isInitialized();

    void tvInit();
    void drcInit();
};