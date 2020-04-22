// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <gtest/gtest.h>

#include "renderer/opengl/error.h"

namespace renderer
{
	TEST(Opengl, ErrorHandling) {
		jactorio::renderer::opengl_print_errors("RETURN-TYPE TEST-FUNCTION(PARAM1, PARAM2, PARAM3)",
		                                        "FILE/PATH/GOING/SOMEWHERE", 123);
		jactorio::renderer::opengl_clear_errors();

		jactorio::renderer::init_glfw_error_handling();
		SUCCEED();
	}
}
