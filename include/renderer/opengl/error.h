#ifndef RENDERER_OPENGL_ERROR_H
#define RENDERER_OPENGL_ERROR_H

// Different break statements depending on the compiler
#ifdef _MSC_VER
#define DEBUG_BREAK __debugbreak();
#undef __GNUC__
#endif

#ifdef __GNUC__
#include <signal.h>
#define DEBUG_BREAK raise(SIGTRAP);
#undef _MSC_VER
#endif


// Wrap a function with DEBUG_OPENGL_CALL to automatically call this when an error occurs and pause code execution
#ifdef JACTORIO_DEBUG_BUILD
#define DEBUG_OPENGL_CALL(func)\
			jactorio::renderer::opengl_clear_errors();\
			func;\
			if ((jactorio::renderer::opengl_print_errors(#func, __FILE__, __LINE__))) DEBUG_BREAK
#else
#define DEBUG_OPENGL_CALL(func)\
			func;
#endif

namespace jactorio::renderer
{
	bool opengl_print_errors(const char* function_name, const char* file,
	                         int line);
	void opengl_clear_errors();

	void init_glfw_error_handling();
}

#endif // RENDERER_OPENGL_ERROR_H
