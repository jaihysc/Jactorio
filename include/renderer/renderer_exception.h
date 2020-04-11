// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERER_EXCEPTION_H
#define JACTORIO_INCLUDE_RENDERER_RENDERER_EXCEPTION_H
#pragma once

#include <stdexcept>

namespace jactorio::renderer
{
	// These are raised by prototype classes if an error occurred
	class Renderer_exception : public std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};
}


#endif // JACTORIO_INCLUDE_RENDERER_RENDERER_EXCEPTION_H
