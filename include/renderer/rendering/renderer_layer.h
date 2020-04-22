// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/12/2020

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_LAYER_H
#define JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_LAYER_H
#pragma once

#include "jactorio.h"

#include "core/data_type.h"
#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/vertex_array.h"
#include "renderer/opengl/vertex_buffer.h"

namespace jactorio::renderer
{
	/**
	 * Generates and maintains the buffers of a rendering layer <br>
	 * Currently 2 buffers are used: Vertex and UV
	 * 
	 */
	class Renderer_layer
	{
		// This is essentially a std::vector with most methods removed for SPEED and additional openGL calls
		// Each records their begin and end point within the large buffer
	public:
		struct Element
		{
			Element() = default;

			Element(const core::Quad_position vertex, const core::Quad_position uv)
				: vertex(vertex), uv(uv) {
			}

			core::Quad_position vertex;
			core::Quad_position uv;
		};

		template <typename T>
		struct Buffer
		{
			Buffer() = default;

			Buffer(T* ptr, const uint32_t span, const uint32_t count)
				: ptr(ptr), span(span), count(count) {
			}

			T* ptr = nullptr;

			// These are only set when called with get_buf_#()
			uint32_t span = 0;
			uint32_t count = 0;
		};

	public:
		explicit Renderer_layer();

		~Renderer_layer();

		// Copying is disallowed because this needs to interact with openGL and the large size of buffers
		Renderer_layer(const Renderer_layer& other) = delete;
		Renderer_layer(Renderer_layer&& other) noexcept = delete;
		Renderer_layer& operator=(const Renderer_layer& other) = delete;
		Renderer_layer& operator=(Renderer_layer&& other) noexcept = delete;

	private:
		// Buffers
		Buffer<float> vertex_buffer_;
		Buffer<float> uv_buffer_;

		/**
		 * Simplified version of delete_buffers, will only delete - nothing else
		 */
		void delete_buffers_s() const;

		/**
		 * Buffer index to insert the next element on push_back
		 */
		mutable uint32_t next_element_index_ = 0;

		/**
		 * Current *element* capacity of VERTEX buffers <br>
		 * for index buffer size, multiply by 6, <br>
		 * for raw size of float array, multiply by 8 <br>
		 */
		uint32_t e_capacity_ = 0;

	public:
		// Set

		/**
		 * Appends element to layer, after the highest element index where values were assigned<br>
		 * Resizes if static_layer_ is false
		 */
		void push_back(const Element& element);

		void set(uint32_t element_index, Element element) const;  // Both vertex and uv
		void set_vertex(uint32_t element_index, core::Quad_position element) const;
		void set_uv(uint32_t element_index, core::Quad_position element) const;
	private:
		// Sets the positions within the buffer, but will not increment next_element_index_;
		void set_buffer_vertex(uint32_t buffer_index, const core::Quad_position& element) const;
		void set_buffer_uv(uint32_t buffer_index, const core::Quad_position& element) const;

	public:
		// Get buffers

		J_NODISCARD Buffer<float> get_buf_vertex();
		J_NODISCARD Buffer<float> get_buf_uv();
		J_NODISCARD const Index_buffer* get_buf_index() const;

		// Resize

		/**
		 * Allocates memory to hold element count, deletes existing elements
		 */
		void reserve(uint32_t count);

		/**
		 * Calls reserve, then copies over existing elements
		 */
		void resize(uint32_t count);

		/**
		 * Returns current element capacity of buffers
		 */
		J_NODISCARD uint32_t get_capacity() const;
		/**
		 * Returns count of elements in buffers
		 */
		J_NODISCARD uint32_t get_element_count() const;

		/**
		 * Deallocates vertex and uv buffers, clearing any stored data
		 */
		void delete_buffer() noexcept;


		/**
		 * Sets next_element_index_ to 0, does not guarantee that underlying buffers will be zeroed, merely that
		 * the data will not by uploaded with glBufferSubData
		 */
		void clear() const;

		// ======================================================================
		// Rendering grid

		///
		/// \brief Generates indices to draw tiles using the grid from gen_render_grid
		/// \returns Indices to be feed into Index_buffer
		static unsigned int* gen_render_grid_indices(uint32_t tile_count);

		// #######################################################################
		// OpenGL methods | The methods below MUST be called from an openGL context
	private:

		Vertex_array* vertex_array_ = nullptr;

		Vertex_buffer* vertex_vb_ = nullptr;
		Vertex_buffer* uv_vb_ = nullptr;
		Index_buffer* index_ib_ = nullptr;

		bool g_resize_vertex_buffers_ = false;

		/**
		 * Only deletes heap vertex buffers, does not set pointers to nullptr or vertex_buffers_generated_ to false
		 */
		void g_delete_buffer_s() const;

	public:
		/**
		 * Initializes vertex buffers for rendering vertex and uv buffers + index buffer<br>
		 * Should only be called once
		 */
		void g_init_buffer();

		/**
		 * Updates vertex buffers based on the current data in the vertex and uv buffer <br>
		 * Will only upload elements from 0 to next_element_index_ - 1
		 */
		void g_update_data();

		/**
		 * Deletes vertex and index buffers
		 */
		void g_delete_buffer();

		/**
		 * Binds the vertex buffers, call this prior to drawing
		 */
		void g_buffer_bind() const;
	};
}

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_LAYER_H
