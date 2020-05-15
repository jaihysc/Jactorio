// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_ERROR_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_ERROR_H
#pragma once

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
			jactorio::renderer::OpenglClearErrors();\
			func;\
			if ((jactorio::renderer::OpenglPrintErrors(#func, __FILE__, __LINE__))) DEBUG_BREAK
#else
#define DEBUG_OPENGL_CALL(func)\
			func;
#endif

namespace jactorio::renderer
{
	bool OpenglPrintErrors(const char* function_name, const char* file,
	                       int line);
	void OpenglClearErrors();

	void InitGlfwErrorHandling();
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_ERROR_H
