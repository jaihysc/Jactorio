// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include <sstream>

#include "jactorio.h"

#include "renderer/opengl/error.h"
#include "renderer/window/window_manager.h"

// Do not directly call this
// Wrap a function with DEBUG_BREAK_IF_FALSE to automatically call this when an error occurs and pause code execution
// Gets and prints all errors from opengl
bool jactorio::renderer::OpenglPrintErrors(const char* function_name, const char* file, const int line) {
	// Do not log error if there is no opengl context
	if (!WindowContextActive())
		return false;

	const unsigned int max_errors = 1000;
	unsigned int error_count      = 0;  // Break after writing 1000 errors, in case this is stuck in a loop

	bool found_error = false;

	GLenum error;
	while (error_count < max_errors && (error = glGetError()) != GL_NO_ERROR) {
		std::stringstream err_ss;
		err_ss << std::hex << error;

		LOG_MESSAGE_F(error, "OpenGL: 0x%s, function %s in file %s at line %d",
		              err_ss.str().c_str(), function_name, file, line);

		found_error = true;
		error_count++;
	}

	if (error_count >= max_errors) {
		LOG_MESSAGE_F(error, "OpenGL: Errors cut, exceeded maximum %d errors", max_errors);
	}

	return found_error;
}

// Do not directly call this
// Wrap a function with DEBUG_BREAK_IF_FALSE to automatically call this when an error occurs and pause code execution
// Clears all existing opengl errors
void jactorio::renderer::OpenglClearErrors() {	// Do not log error if there is no opengl context
	if (!WindowContextActive())
		return;

	const unsigned int max_errors = 1000;
	unsigned int error_count      = 0;  // Break after writing 1000 errors, in case this is stuck in a loop

	while (error_count < max_errors && glGetError() != GL_NO_ERROR)
		error_count++;

	if (error_count >= max_errors) {
		LOG_MESSAGE_F(error, "OpenGL: Errors cut, exceeded maximum %d clear errors", max_errors);
	}
}
