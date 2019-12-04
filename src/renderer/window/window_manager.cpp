#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "renderer/window/window_manager.h"

#include <array>

#include "core/logger.h"
#include "core/loop_manager.h"
#include "game/input/input_manager.h"
#include "renderer/render_main.h"
#include "renderer/opengl/error.h"
#include "renderer/rendering/renderer.h"

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

int jactorio::renderer::window_manager::init(const int width, const int height) {
	// GLFW initialization
	init_glfw_error_handling();

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
		auto icon = sf::Image();
		icon.loadFromFile(core::filesystem::resolve_path("~/data/core/graphics/jactorio.png"));

		// Convert the loaded Image into a GLFWImage
		GLFWimage icons[1];
		icons[0].pixels = const_cast<unsigned char*>(icon.getPixelsPtr());

		const auto& dimensions = icon.getSize();
		icons[0].width = dimensions.x;
		icons[0].height = dimensions.y;
		
		glfwSetWindowIcon(glfw_window, 1, icons);
	}

	monitor = glfwGetPrimaryMonitor();
	glfwGetWindowSize(glfw_window, &window_size[0], &window_size[1]);
	glfwGetWindowPos(glfw_window, &window_pos[0], &window_pos[1]);

	update_viewport = true;

	glfwSetWindowSizeCallback(glfw_window, [](GLFWwindow* window, int cx, int cy) {
		// Ignore window minimize (resolution 0 x 0)
		if (cx > 0 && cy > 0) {
			// glViewport is critical, changes the size of the rendering area
			glViewport(0, 0, cx, cy);
			set_recalculate_renderer(cx, cy);

			LOG_MESSAGE_f(debug, "Resolution changed to %dx%d", cx, cy);
		}
	});

	// Mouse and keyboard callbacks
	glfwSetKeyCallback(
		glfw_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			game::input_manager::set_input(key, action, mods);
		});
	
	glfwSetMouseButtonCallback(glfw_window, [](GLFWwindow* window, int key, int action, int mods) {
		game::input_manager::set_input(key, action, mods);
	});
	glfwSetScrollCallback(glfw_window, [](GLFWwindow* window, double xoffset, double yoffset) {
		get_base_renderer()->tile_projection_matrix_offset += yoffset * 10;
	});

	
	gl_context_active = true;

	LOG_MESSAGE_f(info, "OpenGL initialized - OpenGL Version: %s", glGetString(GL_VERSION))
	return 0;
}


int jactorio::renderer::window_manager::terminate() {
	glfwDestroyWindow(glfw_window);
	glfwTerminate();

	gl_context_active = false;

	LOG_MESSAGE(info, "OpenGL terminated");
	return 0;
}

/// Window

GLFWwindow* jactorio::renderer::window_manager::get_window() {
	return glfw_window;
}

bool jactorio::renderer::window_manager::context_active() {
	return gl_context_active;
}
