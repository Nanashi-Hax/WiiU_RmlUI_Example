#include "RmlSystem.hpp"
#include "App.hpp"
#include "Backend/RmlUi_File_WiiU.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include "Backend/RmlUi_Renderer_GX2.h"
#include "UI/Document/Main.h"
#include "UI/Style/Main.h"
#include "ViewModel/Clock.hpp"
#include "gx2/enum.h"
#include "lifecycle.hpp"
#include "vpad/input.h"
#include <stdexcept>
#include <format>
#include <whb/log.h>
#include <whb/log_udp.h>

RmlSystem::RmlSystem(int width, int height) : contextTV(nullptr), contextDRC(nullptr), initialized(false), renderInterfaceTV(1280, 720), renderInterfaceDRC(854, 480)
{
    Rml::Initialise();

    std::string fontPath = "fs:/vol/external01/wiiu/fonts/NotoSansJP-Medium.ttf";
    if (!Rml::LoadFontFace(fontPath))
    {
        std::string msg = std::format("Rml::LoadFontFace failed: {}", fontPath);
        throw std::runtime_error(msg);
    }

    Rml::SetFileInterface(&fileInterface);
    Rml::SetSystemInterface(&systemInterface);

    tvInit();
    drcInit();

    initialized = true;
}

RmlSystem::~RmlSystem()
{   
    initialized = false;
    
    delete clockTV;
    delete clockDRC;

    Rml::Shutdown();
}

void RmlSystem::draw(GX2ScanTarget target)
{
    if(target == GX2_SCAN_TARGET_TV)
    {
        renderInterfaceTV.BeginFrame();
        contextTV->Update();
        contextTV->Render();
        renderInterfaceTV.EndFrame();
    }

    if(target == GX2_SCAN_TARGET_DRC)
    {
        renderInterfaceDRC.BeginFrame();
        contextDRC->Update();
        contextDRC->Render();
        renderInterfaceDRC.EndFrame();
    }

    clockTV->update(clockModelTV);
    clockDRC->update(clockModelDRC);
}

void RmlSystem::pollKeyEvent()
{
	// Read VPAD (GamePad) input
	App& app = GetApp();
	InputSystem* inputSystem = app.getInputSystem();
	std::vector<VPADStatus> list = inputSystem->consumeAll();
	
	for(VPADStatus s : list)
	{
        if(contextTV)
        {
            processKey(contextTV, s);
            contextTV->Update();
        }

        if(contextDRC)
        {
            processKey(contextDRC, s);
            contextDRC->Update();
        }
	}
}

void RmlSystem::processKey(Rml::Context* context, VPADStatus s)
{
	if (s.trigger & VPAD_BUTTON_A)
    {
		context->ProcessKeyDown(Rml::Input::KI_RETURN, 0);
	}
	if (s.release & VPAD_BUTTON_A)
    {
		context->ProcessKeyUp(Rml::Input::KI_RETURN, 0);
	}

	if (s.trigger & VPAD_BUTTON_UP)
    {
		context->ProcessKeyDown(Rml::Input::KI_UP, 0);
	}
	if (s.release & VPAD_BUTTON_UP)
    {
		context->ProcessKeyUp(Rml::Input::KI_UP, 0);
	}

	if (s.trigger & VPAD_BUTTON_DOWN)
    {
		context->ProcessKeyDown(Rml::Input::KI_DOWN, 0);
	}
	if (s.release & VPAD_BUTTON_DOWN)
    {
		context->ProcessKeyUp(Rml::Input::KI_DOWN, 0);
	}

	if (s.trigger & VPAD_BUTTON_LEFT)
    {
		context->ProcessKeyDown(Rml::Input::KI_LEFT, 0);
	}
	if (s.release & VPAD_BUTTON_LEFT)
    {
		context->ProcessKeyUp(Rml::Input::KI_LEFT, 0);
	}

	if (s.trigger & VPAD_BUTTON_RIGHT)
    {
		context->ProcessKeyDown(Rml::Input::KI_RIGHT, 0);
	}
	if (s.release & VPAD_BUTTON_RIGHT)
    {
		context->ProcessKeyUp(Rml::Input::KI_RIGHT, 0);
	}

	if (s.trigger & VPAD_BUTTON_B)
    {
		context->ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
	}
	if (s.release & VPAD_BUTTON_B)
    {
		context->ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
	}
}

Rml::Context* RmlSystem::getContextTV()
{
    return contextTV;
}

Rml::Context* RmlSystem::getContextDRC()
{
    return contextDRC;
}

bool RmlSystem::isInitialized()
{
    return initialized;
}

void RmlSystem::tvInit()
{
    Rml::SetRenderInterface(&renderInterfaceTV);

    contextTV = Rml::CreateContext("TV", Rml::Vector2i(1280, 720));
    if (!contextTV)
    {
        Rml::Shutdown();
        throw std::runtime_error("Rml::CreateContext failed");
    }

    auto clockConstructor = contextTV->CreateDataModel("ClockModel");
    clockTV = new ViewModel::Clock();
    clockTV->bind(clockConstructor);
    clockModelTV = clockConstructor.GetModelHandle();

    std::string docPath = "Main.rml";
    fileInterface.addVirtual(docPath, Main_rml, Main_rml_size);

    fileInterface.addVirtual("Main.rcss", Main_rcss, Main_rcss_size);

    Rml::ElementDocument* document = contextTV->LoadDocument(docPath);
    if (document)
    {
        document->Show();
    }
    else
    {
        std::string msg = std::format("Rml::Context::LoadDocument failed: {}", docPath);
        throw std::runtime_error(msg);
    }
    auto* button0 = document->GetElementById("0003");
    button0->Focus(true); // 起動時フォーカス

    auto* focus = contextTV->GetFocusElement();
    Rml::Log::Message(Rml::Log::LT_INFO, "focus: %s", focus ? focus->GetId().c_str() : "null");
}

void RmlSystem::drcInit()
{
    Rml::SetRenderInterface(&renderInterfaceDRC);
    contextDRC = Rml::CreateContext("DRC", Rml::Vector2i(854, 480));
    if (!contextDRC)
    {
        Rml::Shutdown();
        throw std::runtime_error("Rml::CreateContext failed");
    }

    auto clockConstructor = contextDRC->CreateDataModel("ClockModel");
    clockDRC = new ViewModel::Clock();
    clockDRC->bind(clockConstructor);
    clockModelDRC = clockConstructor.GetModelHandle();

    std::string docPath = "Main.rml";
    fileInterface.addVirtual(docPath, Main_rml, Main_rml_size);

    fileInterface.addVirtual("Main.rcss", Main_rcss, Main_rcss_size);

    Rml::ElementDocument* document = contextDRC->LoadDocument(docPath);
    if (document)
    {
        document->Show();
    }
    else
    {
        std::string msg = std::format("Rml::Context::LoadDocument failed: {}", docPath);
        throw std::runtime_error(msg);
    }
    auto* button0 = document->GetElementById("0002");
    button0->Focus(true); // 起動時フォーカス

    auto* focus = contextTV->GetFocusElement();
    Rml::Log::Message(Rml::Log::LT_INFO, "focus: %s", focus ? focus->GetId().c_str() : "null");
}