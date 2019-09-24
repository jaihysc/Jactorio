#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "renderer/opengl/error.h"
#include "renderer/manager/window_manager.h"

// Do not directly call this
// Wrap a function with DEBUG_BREAK_IF_FALSE to automatically call this when an error occurs and pause code execution
// Gets and prints all errors from opengl
bool opengl_print_errors(const char* function_name, const char* file, int line) {
	// Do not log error if there is no opengl context
	if (!opengl_get_context_active())
		return false;
	
	const unsigned int max_errors = 1000;
	unsigned int error_count = 0;  // Break after writing 1000 errors, in case this is stuck in a loop

	bool found_error = false;
	
	GLenum error;
	while (error_count < max_errors && (error = glGetError()) != GL_NO_ERROR) {
		std::cerr << "[OpenGL ERROR] 0x" << std::hex << error
		<< std::dec << " - " << function_name << " | LINE: " << line << " | FILE: " << file << "\n";

		found_error = true;
		error_count++;
	}

	if (error_count >= max_errors)
		std::cerr << "[OpenGL ERROR] Errors cut, exceeded maximum " << max_errors << " errors";
	
	return found_error;
}

// Do not directly call this
// Wrap a function with DEBUG_BREAK_IF_FALSE to automatically call this when an error occurs and pause code execution
// Clears all existing opengl errors
void opengl_clear_errors() {	// Do not log error if there is no opengl context
	if (!opengl_get_context_active())
		return;
	
	const unsigned int max_errors = 1000;
	unsigned int error_count = 0;  // Break after writing 1000 errors, in case this is stuck in a loop
	
	while (error_count < max_errors && glGetError() != GL_NO_ERROR)
		error_count++;

	if (error_count >= max_errors)
		std::cerr << "[OpenGL] Clear errors cut, exceeded maximum " << max_errors << " clear errors";
}


// GLFW errors
static void error_callback(int error, const char* description) {
	std::cerr << "[GLFW OpenGL] Error: " << error << " - " << description << "\n";
}

// Initializes error handling for GLFW errors
void init_glfw_error_handling() {
	glfwSetErrorCallback(error_callback);
}