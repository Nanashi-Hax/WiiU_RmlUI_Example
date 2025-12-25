#include "App.hpp"
#include "DrawSystem.hpp"
#include "InputSystem.hpp"
#include "RmlSystem.hpp"
#include <whb/log.h>

void App::initialize()
{
    try
    {
        rmlSystem = new RmlSystem(1280, 720);
        drawSystem = new DrawSystem();
        inputSystem = new InputSystem();
    }
    catch(std::runtime_error& e)
    {
        WHBLogPrintf("App initialization failed: %s", e.what());
    }
}

void App::shutdown()
{
    if(rmlSystem) delete rmlSystem;
    if(drawSystem) delete drawSystem;
    if(inputSystem) delete inputSystem;
}

RmlSystem* App::getRmlSystem()
{
    return rmlSystem;
}

DrawSystem* App::getDrawSystem()
{
    return drawSystem;
}

InputSystem* App::getInputSystem()
{
    return inputSystem;
}