// 
// vertex_array.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/15/2019
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_VERTEX_ARRAY_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_VERTEX_ARRAY_H
#pragma once

#include "renderer/opengl/vertex_buffer.h"

namespace jactorio::renderer
{
	class Vertex_array
	{
		unsigned int id_{};

	public:
		Vertex_array();
		~Vertex_array();

		Vertex_array(const Vertex_array& other) = delete;
		Vertex_array(Vertex_array&& other) noexcept = delete;
		Vertex_array& operator=(const Vertex_array& other) = delete;
		Vertex_array& operator=(Vertex_array&& other) noexcept = delete;

		/**
		 * Adds specified buffer to the vertex array <br>
		 * Vertex_buffer must be deleted manually, it is not managed by the vertex array
		 * @param vb Vertex buffer to add to vertex array
		 * @param span Number of floats for one set of coordinates (2 for X and Y)
		 * @param location Slot in vertex array in which vertex buffer is placed <br>
		 * This must be managed manually to avoid conflicts
		 */
		void add_buffer(const Vertex_buffer* vb, unsigned span,
		                unsigned location) const;

		void bind() const;
		static void unbind();
	};
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_VERTEX_ARRAY_H
