#include <gtest/gtest.h>

#include "renderer/opengl/error.h"

TEST(opengl, errorHandling) {
	opengl_print_errors("RETURN-TYPE TEST-FUNCTION(PARAM1, PARAM2, PARAM3)", "FILE/PATH/GOING/SOMEWHERE", 123);
	opengl_clear_errors();

	init_glfw_error_handling();
	SUCCEED();
}