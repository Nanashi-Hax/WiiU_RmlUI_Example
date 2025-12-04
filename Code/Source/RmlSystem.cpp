#include "RmlSystem.hpp"
#include "Backend/RmlUi_Backend.h"
#include "Backend/RmlUi_File_WiiU.h"
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

    std::string fontPath = "fs:/vol/external01/wiiu/plugins/RmlUI/fonts/Lato-Regular.ttf";
    if (!Rml::LoadFontFace(fontPath))
    {
        std::string msg = std::format("Rml::LoadFontFace failed: {}", fontPath);
        throw std::runtime_error(msg);
    }

    std::string docPath = "fs:/vol/external01/wiiu/plugins/RmlUI/demo.rml";
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

    initialized = true;
}

RmlSystem::~RmlSystem()
{   
    initialized = false;
    Rml::Shutdown();
    Backend::Shutdown();
}

void RmlSystem::draw()
{
    Backend::BeginFrame();
    context->Update();
    context->Render();
    Backend::PresentFrame();
}

Rml::Context* RmlSystem::getContext()
{
    return context;
}

bool RmlSystem::isInitialized()
{
    return initialized;
}