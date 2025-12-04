#pragma once
#include "RmlSystem.hpp"
#include "DrawSystem.hpp"

class App
{
private:
    RmlSystem* rmlSystem;
    DrawSystem* drawSystem;

public:
    void initialize();
    void shutdown();

    RmlSystem* getRmlSystem();
    DrawSystem* getDrawSystem();
};