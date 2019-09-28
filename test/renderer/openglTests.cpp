#include <gtest/gtest.h>

#include "renderer/opengl/error.h"

TEST(opengl, errorHandling) {
	jactorio_renderer_gl::opengl_print_errors("RETURN-TYPE TEST-FUNCTION(PARAM1, PARAM2, PARAM3)", "FILE/PATH/GOING/SOMEWHERE", 123);
	jactorio_renderer_gl::opengl_clear_errors();

	jactorio_renderer_gl::init_glfw_error_handling();
	SUCCEED();
}