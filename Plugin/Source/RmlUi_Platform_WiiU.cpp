/*
 * RmlUi Wii U System Interface Implementation
 * Based on WiiU time and debug functions
 */

#include "RmlUi_Platform_WiiU.h"
#include <RmlUi/Core.h>

// Wii U system headers
#include <coreinit/time.h>
#include <coreinit/systeminfo.h>
#include <whb/log.h>

SystemInterface_WiiU::SystemInterface_WiiU() {
	// Get bus clock speed for time conversion
	OSSystemInfo* system_info = OSGetSystemInfo();
	uint64_t bus_clock_speed = system_info->busClockSpeed;
	
	// Initialize start time
	uint64_t current_time = OSGetTime();
	start_time = (double)current_time / (double)bus_clock_speed;
}

SystemInterface_WiiU::~SystemInterface_WiiU() {
}

double SystemInterface_WiiU::GetElapsedTime() {
	OSSystemInfo* system_info = OSGetSystemInfo();
	uint64_t bus_clock_speed = system_info->busClockSpeed;
	
	uint64_t current_time = OSGetTime();
	double current_seconds = (double)current_time / (double)bus_clock_speed;
	
	return current_seconds - start_time;
}

bool SystemInterface_WiiU::LogMessage(Rml::Log::Type type, const Rml::String& message) {
	const char* type_str = "";
	switch (type) {
		case Rml::Log::LT_ALWAYS:
			type_str = "[ALWAYS] ";
			break;
		case Rml::Log::LT_ERROR:
			type_str = "[ERROR] ";
			break;
		case Rml::Log::LT_ASSERT:
			type_str = "[ASSERT] ";
			break;
		case Rml::Log::LT_WARNING:
			type_str = "[WARNING] ";
			break;
		case Rml::Log::LT_INFO:
			type_str = "[INFO] ";
			break;
		case Rml::Log::LT_DEBUG:
			type_str = "[DEBUG] ";
			break;
		default:
			break;
	}

	// Use WHB logging (similar to OSReport)
	WHBLogPrintf("%s%s", type_str, message.c_str());

	return true; // Return true to continue execution
}

void SystemInterface_WiiU::SetMouseCursor(const Rml::String& cursor_name) {
	// Wii U GamePad doesn't have traditional mouse cursor
	// Could be used to show different cursor textures in UI layer
	(void)cursor_name;
}

void SystemInterface_WiiU::SetClipboardText(const Rml::String& text) {
	// Wii U doesn't have a traditional clipboard
	// Could store in memory for copy/paste within the application
	(void)text;
}

void SystemInterface_WiiU::GetClipboardText(Rml::String& text) {
	// Wii U doesn't have a traditional clipboard
	text.clear();
}
