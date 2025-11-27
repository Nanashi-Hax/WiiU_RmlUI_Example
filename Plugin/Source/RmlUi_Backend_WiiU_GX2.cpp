/*
 * RmlUi Wii U Backend - Main Integration
 * This combines the GX2 renderer and WiiU platform into a complete backend.
 */

#include "RmlUi_Backend.h"
#include "RmlUi_Platform_WiiU.h"
#include "RmlUi_Renderer_GX2.h"
#include <RmlUi/Core/Context.h>

// Wii U input headers
#include <vpad/input.h>
#include <padscore/wpad.h>

static SystemInterface_WiiU* system_interface = nullptr;
static RenderInterface_GX2* render_interface = nullptr;
static bool initialized = false;
static bool request_exit = false;

// Input state
static bool was_touched = false;

namespace Backend {

bool Initialize(const char* window_name, int width, int height, bool allow_resize) {
	(void)window_name;  // Wii U doesn't use window names
	(void)allow_resize; // Wii U has fixed display size

	// Create system and render interfaces
	system_interface = new SystemInterface_WiiU();
	render_interface = new RenderInterface_GX2();
	
	// Set viewport to Wii U screen size
	render_interface->SetViewport(width, height);
	
	initialized = true;
	request_exit = false;
	
	return true;
}

void Shutdown() {
	if (!initialized)
		return;

	delete render_interface;
	delete system_interface;
	
	render_interface = nullptr;
	system_interface = nullptr;
	
	initialized = false;
}

Rml::SystemInterface* GetSystemInterface() {
	return system_interface;
}

Rml::RenderInterface* GetRenderInterface() {
	return render_interface;
}

bool ProcessEvents(Rml::Context* context, KeyDownCallback key_down_callback, bool power_save) {
	(void)power_save;
	
	if (!context)
		return !request_exit;
	
	// Read VPAD (GamePad) input
	VPADStatus vpad_status;
	VPADReadError vpad_error;
	VPADRead(VPAD_CHAN_0, &vpad_status, 1, &vpad_error);
	
	if (vpad_error == VPAD_READ_SUCCESS) {
		// Process touch input
		VPADTouchData touch;
		VPADGetTPCalibratedPoint(VPAD_CHAN_0, &touch, &vpad_status.tpNormal);
		
		if (touch.touched) {
			// Convert touch coordinates to screen coordinates
			// GamePad touchscreen is 854x480, but we scale to context size
			int ctx_width = context->GetDimensions().x;
			int ctx_height = context->GetDimensions().y;
			
			float scale_x = (float)ctx_width / 1280.0f;
			float scale_y = (float)ctx_height / 720.0f;
			
			int mouse_x = (int)(touch.x * scale_x);
			int mouse_y = (int)(touch.y * scale_y);
			
			context->ProcessMouseMove(mouse_x, mouse_y, 0);
		}
		
		// Handle touch press/release
		if (touch.touched != was_touched) {
			if (touch.touched) {
				context->ProcessMouseButtonDown(0, 0);
			} else {
				context->ProcessMouseButtonUp(0, 0);
			}
			was_touched = touch.touched;
		}
		
		// Check for HOME button to exit
		if (vpad_status.trigger & VPAD_BUTTON_HOME) {
			request_exit = true;
		}
		
		// Process buttons as keyboard input (example)
		if (vpad_status.trigger & VPAD_BUTTON_A) {
			context->ProcessKeyDown(Rml::Input::KI_RETURN, 0);
			if (key_down_callback) {
				key_down_callback(context, Rml::Input::KI_RETURN, 0, 1.0f, false);
			}
		}
		if (vpad_status.release & VPAD_BUTTON_A) {
			context->ProcessKeyUp(Rml::Input::KI_RETURN, 0);
		}
		
		// D-Pad navigation
		if (vpad_status.trigger & VPAD_BUTTON_UP) {
			context->ProcessKeyDown(Rml::Input::KI_UP, 0);
		}
		if (vpad_status.release & VPAD_BUTTON_UP) {
			context->ProcessKeyUp(Rml::Input::KI_UP, 0);
		}
		
		if (vpad_status.trigger & VPAD_BUTTON_DOWN) {
			context->ProcessKeyDown(Rml::Input::KI_DOWN, 0);
		}
		if (vpad_status.release & VPAD_BUTTON_DOWN) {
			context->ProcessKeyUp(Rml::Input::KI_DOWN, 0);
		}
		
		if (vpad_status.trigger & VPAD_BUTTON_LEFT) {
			context->ProcessKeyDown(Rml::Input::KI_LEFT, 0);
		}
		if (vpad_status.release & VPAD_BUTTON_LEFT) {
			context->ProcessKeyUp(Rml::Input::KI_LEFT, 0);
		}
		
		if (vpad_status.trigger & VPAD_BUTTON_RIGHT) {
			context->ProcessKeyDown(Rml::Input::KI_RIGHT, 0);
		}
		if (vpad_status.release & VPAD_BUTTON_RIGHT) {
			context->ProcessKeyUp(Rml::Input::KI_RIGHT, 0);
		}
		
		// B button as ESC/Back
		if (vpad_status.trigger & VPAD_BUTTON_B) {
			context->ProcessKeyDown(Rml::Input::KI_ESCAPE, 0);
		}
		if (vpad_status.release & VPAD_BUTTON_B) {
			context->ProcessKeyUp(Rml::Input::KI_ESCAPE, 0);
		}
	}
	
	// Return false if user wants to exit
	return !request_exit;
}

void RequestExit() {
	request_exit = true;
}

void BeginFrame() {
	if (render_interface) {
		render_interface->BeginFrame();
	}
}

void PresentFrame() {
	if (render_interface) {
		render_interface->EndFrame();
	}
	
	// Note: Frame presentation is typically handled by your main graphics loop
	// using GX2CopyColorBufferToScanBuffer, GX2SwapScanBuffers, etc.
	// This depends on your rendering setup (WHBGfx or manual GX2)
}

} // namespace Backend
