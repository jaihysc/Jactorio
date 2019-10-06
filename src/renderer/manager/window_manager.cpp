#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "renderer/manager/window_manager.h"

#include <sstream>

#include "core/logger.h"
#include "renderer/opengl/error.h"

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}


GLFWwindow* window;
bool gl_context_active = false;

int jactorio::renderer::opengl_init() {
	init_glfw_error_handling();
	
	if (!glfwInit())
		return 1;

	// glfwWindowHint(GLEW_VERSION_MAJOR, 3);
	// glfwWindowHint(GLEW_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	
	// window = glfwCreateWindow(640, 480, "Jactorio", NULL, NULL);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	window = glfwCreateWindow(mode->width, mode->height, "Jactorio", monitor, NULL);
	
	if (!window) {
		glfwTerminate();
		return 2;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		return 3;

	glfwSetKeyCallback(window, key_callback);

	gl_context_active = true;


	std::ostringstream oss;
	oss << "OpenGL version: " << glGetString(GL_VERSION);
	logger::log_message(logger::info, "OpenGL", "OpenGL Initialization successful");
	logger::log_message(logger::info, "OpenGL", oss.str());
	
	return 0;
}

int jactorio::renderer::opengl_terminate() {
	glfwDestroyWindow(window);
	glfwTerminate();

	gl_context_active = false;

	logger::log_message(logger::info, "OpenGL", "OpenGL terminated");
	return 0;
}

GLFWwindow* jactorio::renderer::opengl_get_window() {
	return window;
}
bool jactorio::renderer::opengl_get_context_active() {
	return gl_context_active;
}