// 
// index_buffer.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/15/2019
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_INDEX_BUFFER_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_INDEX_BUFFER_H
#pragma once

#include "jactorio.h"

namespace jactorio::renderer
{
	class Index_buffer
	{
	public:
		/**
		 * @param data
		 * @param count Number of indices
		 */
		Index_buffer(const unsigned int* data, unsigned int count);
		~Index_buffer();

		Index_buffer(const Index_buffer& other) = delete;
		Index_buffer(Index_buffer&& other) noexcept = delete;
		Index_buffer& operator=(const Index_buffer& other) = delete;
		Index_buffer& operator=(Index_buffer&& other) noexcept = delete;

		/**
		 * Creates a new buffer of provided specifications
		 */
		void reserve(const void* data, uint32_t index_count);

		void bind() const;
		static void unbind();

		J_NODISCARD unsigned int count() const;

	private:
		unsigned int id_ = 0;
		unsigned int count_ = 0;
	};
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_INDEX_BUFFER_H
