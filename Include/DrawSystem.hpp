#pragma once
#include <gx2/context.h>
#include <gx2/display.h>

class DrawSystem
{
public:
    DrawSystem();
    ~DrawSystem();

private:
    GX2ContextState* originalContext;
    GX2ContextState* pluginContext;
    bool isInitialized;

public:
    GX2ContextState* getOriginalContext();
    GX2ContextState* getPluginContext();

    void setOriginalContext(GX2ContextState* context);
    void update(const GX2ColorBuffer *colorBuffer);
    void initialize();
};