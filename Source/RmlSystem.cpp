#include "RmlSystem.hpp"
#include "Backend/RmlUi_Backend.h"
#include "Backend/RmlUi_File_WiiU.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include "UI/Document/Main.h"
#include "UI/Style/Main.h"
#include "ViewModel/Clock.hpp"
#include <stdexcept>
#include <format>
#include <whb/log.h>
#include <whb/log_udp.h>

RmlSystem::RmlSystem(int width, int height) : context(nullptr), initialized(false)
{
    if (!Backend::Initialize("RmlUi Example", width, height, true))
    {
        throw std::runtime_error("Backend::Initialize failed");
    }

    Rml::SetFileInterface(&fileInterface);
    Rml::SetSystemInterface(Backend::GetSystemInterface());
    Rml::SetRenderInterface(Backend::GetRenderInterface());

    Rml::Initialise();

    context = Rml::CreateContext("main", Rml::Vector2i(width, height));
    if (!context)
    {
        Rml::Shutdown();
        Backend::Shutdown();
        throw std::runtime_error("Rml::CreateContext failed");
    }

    std::string fontPath = "fs:/vol/external01/wiiu/fonts/NotoSansJP-Medium.ttf";
    if (!Rml::LoadFontFace(fontPath))
    {
        std::string msg = std::format("Rml::LoadFontFace failed: {}", fontPath);
        throw std::runtime_error(msg);
    }

    auto clockConstructor = context->CreateDataModel("ClockModel");
    clock = new ViewModel::Clock();
    clock->bind(clockConstructor);
    clockModel = clockConstructor.GetModelHandle();

    std::string docPath = "Main.rml";
    fileInterface.addVirtual(docPath, Main_rml, Main_rml_size);

    fileInterface.addVirtual("Main.rcss", Main_rcss, Main_rcss_size);

    Rml::ElementDocument* document = context->LoadDocument(docPath);
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

    auto* focus = context->GetFocusElement();
    Rml::Log::Message(Rml::Log::LT_INFO, "focus: %s", focus ? focus->GetId().c_str() : "null");

    initialized = true;
}

RmlSystem::~RmlSystem()
{   
    initialized = false;
    
    delete clock;

    Rml::Shutdown();
    Backend::Shutdown();
}

void RmlSystem::draw()
{
    Backend::BeginFrame();
    context->Update();
    context->Render();
    Backend::PresentFrame();

    clock->update(clockModel);
}

Rml::Context* RmlSystem::getContext()
{
    return context;
}

bool RmlSystem::isInitialized()
{
    return initialized;
}