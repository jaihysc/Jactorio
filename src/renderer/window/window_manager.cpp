// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "renderer/window/window_manager.h"

#include <array>

#include "jactorio.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "renderer/render_main.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/opengl/error.h"
#include "renderer/rendering/renderer.h"

std::array<int, 2> window_pos{0, 0};
std::array<int, 2> window_size{0, 0};
std::array<int, 2> viewport_size{0, 0};
bool update_viewport = true;

GLFWwindow* glfw_window = nullptr;
GLFWmonitor* monitor    = nullptr;

bool gl_context_active = false;


/// Fullscreen
bool jactorio::renderer::IsFullscreen() {
	return glfwGetWindowMonitor(glfw_window) != nullptr;
}

void jactorio::renderer::SetFullscreen(const bool fullscreen) {
	if (IsFullscreen() == fullscreen)
		return;

	if (fullscreen) {
		// backup window position and window size
		glfwGetWindowPos(glfw_window, &window_pos[0], &window_pos[1]);
		glfwGetWindowSize(glfw_window, &window_size[0], &window_size[1]);

		// get resolution of monitor
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		// switch to full screen
		glfwSetWindowMonitor(glfw_window, monitor, 0, 0, mode->width, mode->height, 60);
	}
	else {
		// restore last window size and position
		glfwSetWindowMonitor(glfw_window, nullptr,
		                     window_pos[0], window_pos[1],
		                     window_size[0], window_size[1], 0);
	}

	update_viewport = true;
}

///

int jactorio::renderer::InitWindow(const int width, const int height) {
	// GLFW initialization
	InitGlfwErrorHandling();

	if (!glfwInit()) {
		LOG_MESSAGE(critical, "glfw initialization failed");
		return 1;
	}

	// Window initialization
	glfw_window = glfwCreateWindow(width, height, "Jactorio " JACTORIO_VERSION, nullptr, nullptr);
	if (glfw_window == nullptr) {
		glfwTerminate();
		LOG_MESSAGE(critical, "Error initializing window");

		return 1;
	}

	glfwMakeContextCurrent(glfw_window);
	if (glewInit() != GLEW_OK) {
		LOG_MESSAGE(critical, "GLEW initialization failed");
		return 1;
	}

	// Set the Jactorio icon for the window
	{
		const auto icon = data::Sprite("core/graphics/jactorio.png");
		// Convert the loaded Image into a GLFWImage
		GLFWimage icons[1];
		icons[0].pixels = const_cast<unsigned char*>(icon.GetSpritePtr());

		icons[0].width  = static_cast<int>(icon.GetWidth());
		icons[0].height = static_cast<int>(icon.GetHeight());

		glfwSetWindowIcon(glfw_window, 1, icons);
	}

	monitor = glfwGetPrimaryMonitor();
	glfwGetWindowSize(glfw_window, &window_size[0], &window_size[1]);
	glfwGetWindowPos(glfw_window, &window_pos[0], &window_pos[1]);

	update_viewport = true;

	glfwSetWindowSizeCallback(glfw_window, [](GLFWwindow* /*window*/, const int cx, const int cy) {
		// Ignore window minimize (resolution 0 x 0)
		if (cx > 0 && cy > 0) {
			// glViewport is critical, changes the size of the rendering area
			glViewport(0, 0, cx, cy);
			SetRecalculateRenderer(cx, cy);

			LOG_MESSAGE_f(debug, "Resolution changed to %dx%d", cx, cy);
		}
	});

	// Mouse and keyboard callbacks
	glfwSetKeyCallback(
		glfw_window, [](GLFWwindow* /*window*/, const int key, int /*scancode*/, const int action, const int mod) {
			game::KeyInput::SetInput(
				game::KeyInput::ToInputKey(key),
				game::KeyInput::ToInputAction(action),
				game::KeyInput::ToInputMod(mod));
		});

	glfwSetMouseButtonCallback(glfw_window, [](GLFWwindow* /*window*/, const int key, const int action, const int mod) {
		game::KeyInput::SetInput(
			game::KeyInput::ToInputKey(key),
			game::KeyInput::ToInputAction(action),
			game::KeyInput::ToInputMod(mod));
	});

	glfwSetScrollCallback(glfw_window, [](GLFWwindow* /*window*/, double /*xoffset*/, const double yoffset) {
		if (!input_captured)
			GetBaseRenderer()->tileProjectionMatrixOffset += static_cast<float>(yoffset * 10);
	});
	glfwSetCursorPosCallback(glfw_window, [](GLFWwindow* /*window*/, const double xpos, const double ypos) {
		game::SetCursorPosition(xpos, ypos);
	});

	// Enables transparency in textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gl_context_active = true;
	LOG_MESSAGE_f(info, "OpenGL initialized - OpenGL Version: %s", glGetString(GL_VERSION))
	return 0;
}


int jactorio::renderer::TerminateWindow() {
	glfwDestroyWindow(glfw_window);
	glfwTerminate();

	gl_context_active = false;

	LOG_MESSAGE(info, "OpenGL terminated");
	return 0;
}

/// Window

GLFWwindow* jactorio::renderer::GetWindow() {
	return glfw_window;
}

bool jactorio::renderer::WindowContextActive() {
	return gl_context_active;
}
