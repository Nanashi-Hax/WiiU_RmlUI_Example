#pragma once
#include <gx2/context.h>

class DrawSystem
{
public:
    DrawSystem();
    ~DrawSystem();

private:
    GX2ContextState* original;
    GX2ContextState* plugin;

public:
    GX2ContextState* getOriginal();
    GX2ContextState* getPlugin();
};