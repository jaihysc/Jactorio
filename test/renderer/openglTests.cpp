#include <gtest/gtest.h>

#include "renderer/opengl/error.h"

namespace renderer
{
	TEST(opengl, errorHandling) {
		jactorio::renderer::opengl_print_errors("RETURN-TYPE TEST-FUNCTION(PARAM1, PARAM2, PARAM3)", "FILE/PATH/GOING/SOMEWHERE", 123);
		jactorio::renderer::opengl_clear_errors();

		jactorio::renderer::init_glfw_error_handling();
		SUCCEED();
	}
}
