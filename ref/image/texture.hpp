#pragma once

#include <cstdint>

#include <gx2/texture.h>

#include "../../imgui_backend/imgui_impl_gx2.h"

ImGui_ImplGX2_Texture* CreateGX2Texture(int width, int height);
ImGui_ImplGX2_Texture* CreateGX2Texture(uint8_t* data, int width, int height);
void DestroyGX2Texture(ImGui_ImplGX2_Texture* texture);

GX2ColorBuffer* CreateGX2ColorBuffer(int width, int height);
void DestroyGX2ColorBuffer(GX2ColorBuffer* colorBuffer);

extern ImGui_ImplGX2_Texture* gDummyTexture;