#ifndef RMLUI_BACKENDS_FONT_ENGINE_DUMMY_H
#define RMLUI_BACKENDS_FONT_ENGINE_DUMMY_H

#include <RmlUi/Core/FontEngineInterface.h>

class FontEngineInterface_Dummy : public Rml::FontEngineInterface {
public:
    FontEngineInterface_Dummy() {}
    virtual ~FontEngineInterface_Dummy() {}

    bool LoadFontFace(const Rml::String& file_name, int face_index, bool fallback_face, Rml::Style::FontWeight weight) override { return true; }
    
    bool LoadFontFace(Rml::Span<const Rml::byte> data, int face_index, const Rml::String& family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, bool fallback_face) override { return true; }

    Rml::FontFaceHandle GetFontFaceHandle(const Rml::String& family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, int size) override { return 1; }

    Rml::FontEffectsHandle PrepareFontEffects(Rml::FontFaceHandle handle, const Rml::FontEffectList& font_effects) override { return 1; }

    int GetStringWidth(Rml::FontFaceHandle handle, Rml::StringView string, const Rml::TextShapingContext& text_shaping_context, Rml::Character prior_character) override { return string.size() * 8; }

    int GetVersion(Rml::FontFaceHandle handle) override { return 0; }
};

#endif
