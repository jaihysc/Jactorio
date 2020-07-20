// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

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
	///
	/// \brief Generates and maintains the buffers of a rendering layer
	/// Currently 2 buffers are used: Vertex and UV
	/// \remark Methods with G prefix must be called from an OpenGl context
	class RendererLayer
	{
		// This is essentially a std::vector with most methods removed for SPEED and additional openGL calls
		// Each records their begin and end point within the large buffer
	public:
		struct Element
		{
			Element() = default;

			Element(const core::QuadPosition vertex, const core::QuadPosition uv)
				: vertex(vertex), uv(uv) {
			}

			core::QuadPosition vertex;
			core::QuadPosition uv;
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
			uint32_t span  = 0;
			uint32_t count = 0;
		};

	public:
		explicit RendererLayer();

		~RendererLayer();

		// Copying is disallowed because this needs to interact with openGL and the large size of buffers

		RendererLayer(const RendererLayer& other) = delete;
		RendererLayer(RendererLayer&& other) noexcept = default;
		RendererLayer& operator=(const RendererLayer& other) = delete;
		RendererLayer& operator=(RendererLayer&& other) noexcept = default;

	private:
		// Buffers
		Buffer<float> vertexBuffer_;
		Buffer<float> uvBuffer_;

		/// Buffer index to insert the next element on push_back
		mutable uint32_t nextElementIndex_ = 0;

		/// Element capacity which will be requested on the next GUpdateData
		uint32_t queuedECapacity_ = 0;

		/// Current *element* capacity of VERTEX buffers <br>
		/// for index buffer size, multiply by 6, <br>
		/// for raw size of float array, multiply by 8 <br>
		uint32_t eCapacity_ = 0;

	public:
		// Set

		// Ensure GWriteBegin() has been called first before attempting to write into buffers

		///
		/// \brief Appends element to layer, after the highest element index where values were assigned<br>
		/// Resizes if static_layer_ is false
		void PushBack(const Element& element);

	private:
		// Sets the positions within the buffer, but will not increment next_element_index_;
		void SetBufferVertex(uint32_t buffer_index, const core::QuadPosition& element) const;
		void SetBufferUv(uint32_t buffer_index, const core::QuadPosition& element) const;

	public:
		// Get buffers

		J_NODISCARD Buffer<float> GetBufVertex();
		J_NODISCARD Buffer<float> GetBufUv();
		J_NODISCARD const IndexBuffer* GetBufIndex() const;

		// Resize

		///
		/// \brief Indicates to allocates memory to hold element count, deletes existing elements,
		/// performed when GUpdateData is called
		void Reserve(uint32_t count);

		///
		/// \brief Returns current element capacity of buffers
		J_NODISCARD uint32_t GetCapacity() const;

		///
		/// \brief Returns count of elements in buffers
		J_NODISCARD uint32_t GetElementCount() const;

		///
		/// \brief Deallocates vertex and uv buffers, clearing any stored data
		void DeleteBuffer() noexcept;


		///
		/// \brief Sets next_element_index_ to 0, does not guarantee that underlying buffers will be zeroed, merely that
		/// the data will not by uploaded with glBufferSubData
		void Clear() const;

		// ======================================================================
		// Rendering grid

		///
		/// \brief Generates indices to draw tiles using the grid from gen_render_grid
		/// \returns Indices to be feed into Index_buffer
		static unsigned int* GenRenderGridIndices(uint32_t tile_count);

		// #######################################################################
		// OpenGL methods | The methods below MUST be called from an openGL context
	private:

		bool writeEnabled_ = false;

		VertexArray* vertexArray_ = nullptr;

		VertexBuffer* vertexVb_ = nullptr;
		VertexBuffer* uvVb_     = nullptr;
		IndexBuffer* indexIb_   = nullptr;

		bool gResizeVertexBuffers_ = false;

		///
		/// \brief Only deletes heap vertex buffers, does not set pointers to nullptr or vertex_buffers_generated_ to false
		void GDeleteBufferS() const;

	public:
		///
		/// \brief Initializes vertex buffers for rendering vertex and uv buffers + index buffer
		/// \remark Should only be called once
		void GInitBuffer();


		// Ensure GWriteBegin() has been called first before attempting to write into buffers

		///
		/// \brief Begins writing data to buffers
		void GWriteBegin();
		///
		/// \brief Ends writing data to buffers
		void GWriteEnd();


		///
		/// \brief Updates vertex buffers based on the current data in the vertex and uv buffer <br>
		/// Will only upload elements from 0 to next_element_index_ - 1
		void GUpdateData();

		///
		/// \brief Deletes vertex and index buffers
		void GDeleteBuffer();

		///
		/// \brief Binds the vertex buffers, call this prior to drawing
		void GBufferBind() const;
	};
}

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_LAYER_H
