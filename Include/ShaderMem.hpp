#pragma once

// wut
#include <whb/gfx.h>

bool WHBGfxLoadGFDShaderGroupMappedMem(WHBGfxShaderGroup *group, uint32_t index, const void *file);

bool WHBGfxInitFetchShaderMappedMem(WHBGfxShaderGroup *group);

bool WHBGfxFreeShaderGroupMappedMem(WHBGfxShaderGroup *group);