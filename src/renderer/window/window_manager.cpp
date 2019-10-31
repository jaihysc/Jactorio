#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "renderer/window/window_manager.h"

#include <sstream>
#include <array>
#include <sstream>

#include "core/logger.h"
#include "renderer/render_main.h"
#include "renderer/opengl/error.h"

namespace logger = jactorio::core::logger;

std::array<int, 2> window_pos{0, 0};
std::array<int, 2> window_size{0, 0};
std::array<int, 2> viewport_size{0, 0};
bool update_viewport = true;

GLFWwindow* glfw_window = nullptr;
GLFWmonitor* monitor = nullptr;

bool gl_context_active = false;


/// Fullscreen
bool jactorio::renderer::window_manager::is_fullscreen() {
	return glfwGetWindowMonitor(glfw_window) != nullptr;
}

void jactorio::renderer::window_manager::set_fullscreen(const bool fullscreen) {
	if (is_fullscreen() == fullscreen)
		return;

	if (fullscreen) {
		// backup window position and window size
		glfwGetWindowPos(glfw_window, &window_pos[0], &window_pos[1]);
		glfwGetWindowSize(glfw_window, &window_size[0], &window_size[1]);

		// get resolution of monitor
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		// switch to full screen
		glfwSetWindowMonitor(glfw_window, monitor, 0, 0, mode->width, mode->height,
		                     get_render_refresh_rate());
	}
	else {
		// restore last window size and position
		glfwSetWindowMonitor(glfw_window, nullptr,
		                     window_pos[0], window_pos[1],
		                     window_size[0], window_size[1], 0);
	}

	update_viewport = true;
}

/// Callbacks

void callback_resize(GLFWwindow* window, int cx, int cy) {
	// Ignore window minimize (resolution 0 x 0)
	if (cx > 0 && cy > 0) {
		// glViewport is critical, changes the size of the rendering area
		glViewport(0, 0, cx, cy);
		jactorio::renderer::set_recalculate_renderer(cx, cy);

		std::stringstream ss;
		ss << "Resolution changed to: " << cx << " by " << cy;
		log_message(logger::debug, "Window_manager", ss.str());
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_RELEASE)
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_SPACE)
			jactorio::renderer::window_manager::set_fullscreen(
				!jactorio::renderer::window_manager::is_fullscreen());
}


///

int jactorio::renderer::window_manager::init(const int width, const int height) {
	// GLFW initialization
	init_glfw_error_handling();

	if (!glfwInit()) {
		log_message(logger::error, "Window manager", "glfw initialization failed");
		return 1;
	}

	// Window initialization
	glfw_window = glfwCreateWindow(width, height, "Jactorio", nullptr, nullptr);
	if (glfw_window == nullptr) {
		glfwTerminate();
		log_message(logger::error, "Window manager", "Error initializing window");

		return 1;
	}

	glfwMakeContextCurrent(glfw_window);
	if (glewInit() != GLEW_OK) {
		log_message(logger::error, "Window manager", "GLEW initialization failed");
		return 1;
	}


	monitor = glfwGetPrimaryMonitor();
	glfwGetWindowSize(glfw_window, &window_size[0], &window_size[1]);
	glfwGetWindowPos(glfw_window, &window_pos[0], &window_pos[1]);

	update_viewport = true;

	// Set callbacks
	glfwSetWindowSizeCallback(glfw_window, callback_resize);
	glfwSetKeyCallback(glfw_window, key_callback);
	gl_context_active = true;

	log_message(logger::info, "Window manager", "OpenGL Initialization successful");
	std::ostringstream oss;
	oss << "OpenGL version: " << glGetString(GL_VERSION);
	log_message(logger::info, "Window manager", oss.str());

	return 0;
}


int jactorio::renderer::window_manager::terminate() {
	glfwDestroyWindow(glfw_window);
	glfwTerminate();

	gl_context_active = false;

	log_message(logger::info, "Window manager", "OpenGL terminated");
	return 0;
}

/// Window

GLFWwindow* jactorio::renderer::window_manager::get_window() {
	return glfw_window;
}

bool jactorio::renderer::window_manager::context_active() {
	return gl_context_active;
}
