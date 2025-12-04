/*
 * RmlUi Wii U System Interface
 * 
 * This provides system utilities for RmlUi on Wii U.
 */

#ifndef RMLUI_BACKENDS_PLATFORM_WIIU_H
#define RMLUI_BACKENDS_PLATFORM_WIIU_H

#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/Types.h>

class SystemInterface_WiiU : public Rml::SystemInterface {
public:
	SystemInterface_WiiU();
	~SystemInterface_WiiU();

	// -- Inherited from Rml::SystemInterface --

	double GetElapsedTime() override;
	bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;

	// Optional implementations
	void SetMouseCursor(const Rml::String& cursor_name) override;
	void SetClipboardText(const Rml::String& text) override;
	void GetClipboardText(Rml::String& text) override;

private:
	double start_time;
};

#endif
