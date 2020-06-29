// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include <GL/glew.h>

#include "renderer/window/window_manager.h"

#include <array>
#include <SDL.h>

#include "jactorio.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "renderer/render_main.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/rendering/renderer.h"

std::array<int, 2> window_pos{0, 0};
std::array<int, 2> window_size{0, 0};
std::array<int, 2> viewport_size{0, 0};
bool update_viewport = true;

SDL_Window* sdl_window = nullptr;
SDL_GLContext sdl_gl_context;

int display_count  = 0;
int active_display = 0;

bool gl_context_active = false;


/// Fullscreen
bool jactorio::renderer::IsFullscreen() {
	constexpr Uint32 fullscreen_flag = SDL_WINDOW_FULLSCREEN;
	return SDL_GetWindowFlags(sdl_window) & fullscreen_flag;
}

void jactorio::renderer::SetFullscreen(const bool desired_fullscreen) {
	if (IsFullscreen() == desired_fullscreen)
		return;

	if (desired_fullscreen) {
		// backup window position and window size
		SDL_GetWindowPosition(sdl_window, &window_pos[0], &window_pos[1]);
		SDL_GetWindowSize(sdl_window, &window_size[0], &window_size[1]);

		// get resolution of monitor
		// switch to full screen
		SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN);
	}
	else {
		// Exit fullscreen
		SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN);

		// restore last window size and position
		SDL_SetWindowPosition(sdl_window, window_pos[0], window_pos[1]);
		SDL_SetWindowSize(sdl_window, window_size[0], window_size[1]);
	}

	update_viewport = true;
}

///

int jactorio::renderer::InitWindow(const int width, const int height) {
	LOG_MESSAGE(info, "Using SDL2 for window creation")

	// ======================================================================
	// Initialize SDL

	// InitGlfwErrorHandling();

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		LOG_MESSAGE(critical, "SDL initialization failed");
		goto sdl_error;
	}

	// Window initialization

	sdl_window = SDL_CreateWindow(
		"Jactorio " JACTORIO_VERSION,      // window title
		SDL_WINDOWPOS_UNDEFINED,           // initial x position
		SDL_WINDOWPOS_UNDEFINED,           // initial y position
		width,                             // width, in pixels
		height,                            // height, in pixels
		SDL_WINDOW_OPENGL                  // flags - see below
	);
	if (!sdl_window) {
		SDL_Quit();
		LOG_MESSAGE(critical, "Error initializing window");
		goto sdl_error;
	}

	SDL_SetWindowResizable(sdl_window, SDL_TRUE);

	// Multi monitor information
	display_count = SDL_GetNumVideoDisplays();

	SDL_GetWindowPosition(sdl_window, &window_pos[0], &window_pos[1]);
	SDL_GetWindowSize(sdl_window, &window_size[0], &window_size[1]);

	update_viewport = true;

	// Set the Jactorio icon for the window
	{
		const auto icon = data::Sprite("core/graphics/jactorio-64-64.png");  // <-- This will throw if it fails

		// Convert the loaded Image into a surface for setting the icon
		SDL_Surface* surface =
			SDL_CreateRGBSurfaceFrom(const_cast<unsigned char*>(icon.GetSpritePtr()),
			                         icon.GetWidth(), icon.GetHeight(),
			                         32,
			                         sizeof(unsigned char) * 4 * icon.GetWidth(),
			                         0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

		// See https://wiki.libsdl.org/SDL_CreateRGBSurfaceFrom#Remarks

		SDL_SetWindowIcon(sdl_window, surface);
		SDL_FreeSurface(surface);
	}

	// Enables transparency in textures
	sdl_gl_context = SDL_GL_CreateContext(sdl_window);
	if (sdl_gl_context == nullptr) {
		LOG_MESSAGE(critical, "Failed to create OpenGL Context");
		goto sdl_error;
	}

	// ======================================================================
	// Initialize GLEW

	if (glewInit() != GLEW_OK) {
		LOG_MESSAGE(critical, "GLEW initialization failed");
		goto glew_error;
	}


	// ======================================================================
	// Opengl Setup

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gl_context_active = true;
	LOG_MESSAGE_f(info, "OpenGL initialized - OpenGL Version: %s", glGetString(GL_VERSION))
	return 0;


	// Error handling

sdl_error:
	if (sdl_window)
		SDL_DestroyWindow(sdl_window);
	SDL_Quit();
	return 2;

glew_error:
	SDL_GL_DeleteContext(sdl_gl_context);
	SDL_DestroyWindow(sdl_window);
	SDL_Quit();
	return 3;
}


int jactorio::renderer::TerminateWindow() {
	SDL_GL_DeleteContext(sdl_gl_context);
	SDL_DestroyWindow(sdl_window);
	SDL_Quit();

	gl_context_active = false;

	LOG_MESSAGE(info, "SDL, OpenGL terminated");
	return 0;
}

/// Window

SDL_Window* jactorio::renderer::GetWindow() {
	return sdl_window;
}

SDL_GLContext jactorio::renderer::GetContext() {
	return sdl_gl_context;
}

bool jactorio::renderer::WindowContextActive() {
	return gl_context_active;
}

// ======================================================================
// Events

void HandleWindowEvent(const SDL_Event& sdl_event) {
	switch (sdl_event.window.event) {
	case SDL_WINDOWEVENT_RESIZED:
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		{
			// Ignore window minimize (resolution 0 x 0)
			const auto x = sdl_event.window.data1;
			const auto y = sdl_event.window.data2;
			if (x > 0 && y > 0) {
				// glViewport is critical, changes the size of the rendering area
				glViewport(0, 0, x, y);
				jactorio::renderer::SetRecalculateRenderer(x, y);

				LOG_MESSAGE_f(debug, "Resolution changed to %dx%d", x, y);
			}
		}
		break;

	case SDL_WINDOWEVENT_SHOWN:
	case SDL_WINDOWEVENT_HIDDEN:
	case SDL_WINDOWEVENT_EXPOSED:
	case SDL_WINDOWEVENT_MOVED:
	case SDL_WINDOWEVENT_MINIMIZED:
	case SDL_WINDOWEVENT_MAXIMIZED:
	case SDL_WINDOWEVENT_RESTORED:
	case SDL_WINDOWEVENT_ENTER:
	case SDL_WINDOWEVENT_LEAVE:
	case SDL_WINDOWEVENT_FOCUS_GAINED:
	case SDL_WINDOWEVENT_FOCUS_LOST:
	case SDL_WINDOWEVENT_CLOSE:
	case SDL_WINDOWEVENT_TAKE_FOCUS:
	case SDL_WINDOWEVENT_HIT_TEST:
		break;

	default:
		LOG_MESSAGE_f(warning, "Window %d got unknown sdl_event %d",
		              sdl_event.window.windowID, sdl_event.window.event);
		break;
	}
}

void jactorio::renderer::HandleSdlEvent(const SDL_Event& sdl_event) {
	switch (sdl_event.type) {
	case SDL_WINDOWEVENT:
		HandleWindowEvent(sdl_event);
		break;

	case SDL_QUIT:
		render_thread_should_exit = true;
		break;

		// Keyboard events
	case SDL_KEYUP:
	case SDL_KEYDOWN:
		{
			// Remove num and caps modifiers, they will not be considered modifiers
			int i_keymod = static_cast<int>(SDL_GetModState());
			i_keymod |= KMOD_NUM;
			i_keymod |= KMOD_CAPS;
			i_keymod -= static_cast<int>(KMOD_NUM);
			i_keymod -= static_cast<int>(KMOD_CAPS);

			const auto keymod = static_cast<SDL_Keymod>(i_keymod);
			
			game::KeyInput::SetInput(static_cast<SDL_KeyCode>(sdl_event.key.keysym.sym),
			                         game::KeyInput::ToInputAction(sdl_event.key.type, sdl_event.key.repeat),
			                         keymod);
		}
		break;

		// Mouse events
	case SDL_MOUSEMOTION:
		game::SetCursorPosition(sdl_event.motion.x, sdl_event.motion.y);
		break;
	case SDL_MOUSEWHEEL:
		if (!input_captured)
			GetBaseRenderer()->tileProjectionMatrixOffset += static_cast<float>(sdl_event.wheel.y * 10);
		break;
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
		{
			game::InputAction action = game::InputAction::key_down;

			// Convert to InputAction, only button down and up supported
			if (sdl_event.button.state == SDL_RELEASED)
				action = game::InputAction::key_up;

			game::MouseInput mouse_input;
			switch (sdl_event.button.button) {
			case SDL_BUTTON_LEFT:
				mouse_input = game::MouseInput::left;
				break;
			case SDL_BUTTON_MIDDLE:
				mouse_input = game::MouseInput::middle;
				break;
			case SDL_BUTTON_RIGHT:
				mouse_input = game::MouseInput::right;
				break;
			case SDL_BUTTON_X1:
				mouse_input = game::MouseInput::x1;
				break;
			case SDL_BUTTON_X2:
				mouse_input = game::MouseInput::x2;
				break;

			default:
				assert(false);
				break;
			}

			game::KeyInput::SetInput(mouse_input, action, KMOD_NONE);
		}
		break;

	default:
		break;
	}
}
