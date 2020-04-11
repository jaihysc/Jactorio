// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_VERTEX_BUFFER_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_VERTEX_BUFFER_H
#pragma once

#include <cstdint>

namespace jactorio::renderer
{
	class Vertex_buffer
	{
	public:
		Vertex_buffer(const void* data, uint32_t byte_size, bool static_buffer);
		~Vertex_buffer();

		Vertex_buffer(const Vertex_buffer& other) = delete;
		Vertex_buffer(Vertex_buffer&& other) noexcept = delete;
		Vertex_buffer& operator=(const Vertex_buffer& other) = delete;
		Vertex_buffer& operator=(Vertex_buffer&& other) noexcept = delete;


		void update_data(const void* data, uint32_t offset, uint32_t size) const;

		/**
		 * Creates a new buffer of provided specifications
		 */
		void reserve(const void* data, uint32_t byte_size, bool static_buffer) const;

		void bind() const;
		static void unbind();

	private:
		unsigned int id_ = 0;
	};
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_VERTEX_BUFFER_H
