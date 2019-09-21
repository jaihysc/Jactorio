#ifndef OPENGL_ERROR_H
#define OPENGL_ERROR_H

// Different break statements depending on the compiler
#ifdef _MSC_VER
#define DEBUG_BREAK __debugbreak();

#endif
#ifdef __GNUC__
#define DEBUG_BREAK raise(SIGTRAP);
#endif


// Wrap a function with DEBUG_OPENGL_CALL to automatically call this when an error occurs and pause code execution


#ifdef JACTORIO_DEBUG_BUILD
#define DEBUG_OPENGL_CALL(func)\
		opengl_clear_errors();\
		func;\
		if ((opengl_print_errors(#func, __FILE__, __LINE__))) DEBUG_BREAK
#else
#define DEBUG_OPENGL_CALL(func)\
		func;
#endif

bool opengl_print_errors(const char* function_name, const char* file, int line);
void opengl_clear_errors();

void init_glfw_error_handling();

#endif // OPENGL_ERROR_H
