// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_VERTEX_BUFFER_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_VERTEX_BUFFER_H
#pragma once

#include <cstdint>

namespace jactorio::renderer
{
	class VertexBuffer
	{
	public:
		VertexBuffer(const void* data, uint32_t byte_size, bool static_buffer);
		~VertexBuffer();

		VertexBuffer(const VertexBuffer& other)                = delete;
		VertexBuffer(VertexBuffer&& other) noexcept            = delete;
		VertexBuffer& operator=(const VertexBuffer& other)     = delete;
		VertexBuffer& operator=(VertexBuffer&& other) noexcept = delete;


		void UpdateData(const void* data, uint32_t offset, uint32_t size) const;

		///
		/// \brief Creates a new buffer of provided specifications
		void Reserve(const void* data, uint32_t byte_size, bool static_buffer) const;

		void Bind() const;
		static void Unbind();

	private:
		unsigned int id_ = 0;
	};
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_VERTEX_BUFFER_H
