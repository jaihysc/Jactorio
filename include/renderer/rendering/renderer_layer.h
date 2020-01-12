#ifndef RENDERER_RENDERING_RENDERER_LAYER_H
#define RENDERER_RENDERING_RENDERER_LAYER_H

#include "jactorio.h"

#include "core/data_type.h"
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
		// TODO if buffer type is marked as static, resize one large buffer which is used by multiple static renderer_layers...
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
		explicit Renderer_layer(bool static_layer);

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
		 * If static the layer will not automatically resize on push_back if exceeding maximum capacity, instead
		 * reserve or resize must be called manually
		 */
		bool static_layer_ = false;

		/**
		 * Buffer index to insert the next element on push_back
		 */
		uint32_t next_element_index_ = 0;

		/**
		 * Current size of buffers
		 */
		uint32_t e_count_ = 0;

	public:
		// Set
		
		/**
		 * Appends element to layer, resizes if static_layer_ is false
		 */
		void push_back(Element element);

		void set(uint32_t element_index, Element element) const;  // Both vertex and uv
		void set_vertex(uint32_t element_index, core::Quad_position element) const;
		void set_uv(uint32_t element_index, core::Quad_position element) const;

		// Get buffers

		J_NODISCARD Buffer<float> get_buf_vertex();
		J_NODISCARD Buffer<float> get_buf_uv();
		
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
		 * Returns current element count of buffers
		 */
		J_NODISCARD uint32_t e_count() const;
		
		/**
		 * Deallocates vertex and uv buffers, clearing any stored data
		 */
		void delete_buffer() noexcept;


	private:
		// OpenGL methods | The methods below MUST be called from an openGL context
		
		Vertex_buffer* vertex_vb_ = nullptr;
		Vertex_buffer* uv_vb_ = nullptr;
		
		bool vertex_buffers_generated_ = false;

		/**
		 * Initializes vertex buffers for rendering the 2 buffers <br>
		 * Should only be called once
		 */
		void g_init_buffer();
		
	public:
		/**
		 * Updates vertex buffers based on the current data in the 2 buffers
		 */
		void g_update_data();

		/**
		 * Binds the vertex buffers, call this prior to drawing
		 */
		void g_buffer_bind() const;
	};
}

#endif // RENDERER_RENDERING_RENDERER_LAYER_H
