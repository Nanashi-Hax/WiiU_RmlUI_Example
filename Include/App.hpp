#pragma once
#include "InputSystem.hpp"
#include "RmlSystem.hpp"
#include "DrawSystem.hpp"

class App
{
private:
    RmlSystem* rmlSystem;
    DrawSystem* drawSystem;
    InputSystem* inputSystem;

public:
    void initialize();
    void shutdown();

    RmlSystem* getRmlSystem();
    DrawSystem* getDrawSystem();
    InputSystem* getInputSystem();
};