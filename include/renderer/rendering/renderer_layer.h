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
	/// \remark Can only be created and destructed in an Opengl Context
	/// \remark Methods with Gl prefix must be called from an OpenGl context
	class RendererLayer
	{
		/// How many elements to reserve upon construction
		static constexpr uint32_t kInitialSize = 500;

		/// When resizing, the amount requested is multiplied by this, reducing future allocations
		static constexpr double kResizeECapacityMultiplier = 1.25;

		static constexpr int kIndicesPerElement = 8;
		static constexpr int kBytesPerElement   = kIndicesPerElement * sizeof(float);

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

		explicit RendererLayer();
		~RendererLayer();

		// Copying is disallowed because this needs to interact with openGL

		RendererLayer(const RendererLayer& other)                = delete;
		RendererLayer(RendererLayer&& other) noexcept            = default;
		RendererLayer& operator=(const RendererLayer& other)     = delete;
		RendererLayer& operator=(RendererLayer&& other) noexcept = default;

		// ======================================================================

		///
		/// \brief Appends element to layer, after the highest element index where values were assigned<br>
		/// Resizes if static_layer_ is false
		/// \remark Ensure GlWriteBegin() has been called first before attempting to write into buffers
		void PushBack(const Element& element);

		///
		/// \brief Returns current element capacity of buffers
		J_NODISCARD uint32_t GetCapacity() const noexcept {
			return eCapacity_;
		}

		///
		/// \brief Returns count of elements in buffers
		J_NODISCARD uint32_t GetElementCount() const {
			return nextElementIndex_;
		}

		///
		/// \brief Queues allocation to hold element count
		void Reserve(uint32_t count);

		///
		/// \brief Signal to begin overriding old existing data in buffers 
		/// \remark Does not guarantee that underlying buffers will be zeroed, merely that
		/// the data will not by uploaded with glBufferSubData
		void Clear();

		// ======================================================================

		///
		/// \brief Begins writing data to buffers
		void GlWriteBegin();
		///
		/// \brief Ends writing data to buffers
		void GlWriteEnd();


		///
		/// \brief If a buffer resize was requested, resizes the buffers
		void GlHandleBufferResize();

		///
		/// \brief Deletes vertex and index buffers
		void GlDeleteBuffers();

		///
		/// \brief Binds the vertex buffers, call this prior to drawing
		void GlBindBuffers() const;

	private:
		// Sets the positions within the buffer, but will not increment next_element_index_;
		void SetBufferVertex(uint32_t buffer_index, const core::QuadPosition& element) const;
		void SetBufferUv(uint32_t buffer_index, const core::QuadPosition& element) const;

		///
		/// \brief Generates indices to draw tiles using the grid from gen_render_grid
		/// \returns Indices to be feed into Index_buffer
		static unsigned int* GenRenderGridIndices(uint32_t tile_count);


		///
		/// \brief Initializes vertex buffers for rendering vertex and uv buffers + index buffer
		/// \remark Should only be called once
		void GlInitBuffers();

		///
		/// \brief Only deletes heap vertex buffers, does not set pointers to nullptr
		void GlFreeBuffers() const;

		// ======================================================================


		/// Buffer index to insert the next element on push_back
		uint32_t nextElementIndex_ = 0;

		/// Element capacity which will be requested on the next GUpdateData
		uint32_t queuedECapacity_ = 0;

		/// Current *element* capacity of VERTEX buffers
		/// for index buffer size, multiply by 6,
		uint32_t eCapacity_ = 0;

		bool writeEnabled_         = false;
		bool gResizeVertexBuffers_ = false;

		float* vertexBuffer_ = nullptr;
		float* uvBuffer_     = nullptr;

		VertexArray* vertexArray_ = nullptr;
		VertexBuffer* vertexVb_   = nullptr;
		VertexBuffer* uvVb_       = nullptr;
		IndexBuffer* indexIb_     = nullptr;
	};
}

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_LAYER_H
