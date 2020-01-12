#include <gtest/gtest.h>

#include "renderer/rendering/renderer_layer.h"

namespace renderer
{
	TEST(renderer_layer, push_back) {
		// Expect buffer to be expanded to accommodate

		using namespace jactorio::renderer;
		using namespace jactorio::core;
	
		auto r_layer = Renderer_layer(false);

		// No resize
		r_layer.push_back(
			Renderer_layer::Element(
				Quad_position(
					Position2<float>(0.11, 0.22),
					Position2<float>(0.33, 0.44)
				),
				Quad_position(
					Position2<float>(0.1, 0.2),
					Position2<float>(0.3, 0.4)
				)
			)
		);

		// This causes resize
		r_layer.push_back(
			Renderer_layer::Element(
				Quad_position(
					Position2<float>(0.11, 0.22),
					Position2<float>(0.33, 0.44)
				),
				Quad_position(
					Position2<float>(0.1, 0.2),
					Position2<float>(0.3, 0.4)
				)
			)
		);
		
		EXPECT_EQ(r_layer.e_count(), 2);
		EXPECT_EQ(r_layer.get_buf_vertex().count, 2);
		EXPECT_EQ(r_layer.get_buf_uv().count, 2);
	}
	
	TEST(renderer_layer, set_element) {
		// Set element at element position 1
		// Reading from the buffer, this is index 8 - 15

		using namespace jactorio::renderer;
		using namespace jactorio::core;
	
		auto r_layer = Renderer_layer(false);

		r_layer.reserve(10);
		r_layer.set(1,
		            Renderer_layer::Element(
			            Quad_position(
				            Position2<float>(0.11, 0.22),
				            Position2<float>(0.33, 0.44)
			            ),
			            Quad_position(
				            Position2<float>(0.1, 0.2),
				            Position2<float>(0.3, 0.4)
			            )
		            )
		);

		const auto vertex_ptr = r_layer.get_buf_vertex().ptr;

		EXPECT_EQ(vertex_ptr[8], 0.11f);
		EXPECT_EQ(vertex_ptr[9], 0.22f);

		EXPECT_EQ(vertex_ptr[10], 0.33f);
		EXPECT_EQ(vertex_ptr[11], 0.22f);
		
		EXPECT_EQ(vertex_ptr[12], 0.33f);
		EXPECT_EQ(vertex_ptr[13], 0.44f);

		EXPECT_EQ(vertex_ptr[14], 0.11f);
		EXPECT_EQ(vertex_ptr[15], 0.44f);
		
		const auto uv_ptr = r_layer.get_buf_uv().ptr;
		EXPECT_EQ(uv_ptr[8 ], 0.1f);
		EXPECT_EQ(uv_ptr[9 ], 0.4f);
		
		EXPECT_EQ(uv_ptr[10], 0.3f);
		EXPECT_EQ(uv_ptr[11], 0.4f);
		
		EXPECT_EQ(uv_ptr[12], 0.3f);
		EXPECT_EQ(uv_ptr[13], 0.2f);
		
		EXPECT_EQ(uv_ptr[14], 0.1f);
		EXPECT_EQ(uv_ptr[15], 0.2f);
	}

	TEST(renderer_layer, reserve) {
		// When reserving, the count should be updated appropriately for the buffers

		using namespace jactorio::renderer;
		using namespace jactorio::core;
	
		auto r_layer = Renderer_layer(false);

		r_layer.reserve(10);

		EXPECT_EQ(r_layer.e_count(), 10);
		
		EXPECT_EQ(r_layer.get_buf_vertex().count, 10);
		EXPECT_EQ(r_layer.get_buf_uv().count, 10);
	}

	TEST(renderer_layer, resize) {
		// Ensure values still remain in correct places after resizing

		using namespace jactorio::renderer;
		using namespace jactorio::core;
	
		auto r_layer = Renderer_layer(false);

		r_layer.reserve(10);
		r_layer.set(1,
		            Renderer_layer::Element(
			            Quad_position(
				            Position2<float>(0.11, 0.22),
				            Position2<float>(0.33, 0.44)
			            ),
			            Quad_position(
				            Position2<float>(0.1, 0.2),
				            Position2<float>(0.3, 0.4)
			            )
		            )
		);

		r_layer.resize(5);

		// Ensure values still remain after resizing
		const auto vertex_ptr = r_layer.get_buf_vertex().ptr;
		EXPECT_EQ(vertex_ptr[8], 0.11f);
		EXPECT_EQ(vertex_ptr[9], 0.22f);

		EXPECT_EQ(vertex_ptr[10], 0.33f);
		EXPECT_EQ(vertex_ptr[11], 0.22f);
		
		EXPECT_EQ(vertex_ptr[12], 0.33f);
		EXPECT_EQ(vertex_ptr[13], 0.44f);

		EXPECT_EQ(vertex_ptr[14], 0.11f);
		EXPECT_EQ(vertex_ptr[15], 0.44f);
		/* Inverted
		EXPECT_EQ(vertex_ptr[8 ], 0.11f);
		EXPECT_EQ(vertex_ptr[9 ], 0.44f);
		
		EXPECT_EQ(vertex_ptr[10], 0.33f);
		EXPECT_EQ(vertex_ptr[11], 0.44f);
		
		EXPECT_EQ(vertex_ptr[12], 0.33f);
		EXPECT_EQ(vertex_ptr[13], 0.22f);
		
		EXPECT_EQ(vertex_ptr[14], 0.11f);
		EXPECT_EQ(vertex_ptr[15], 0.22f);
		 */

		const auto uv_ptr = r_layer.get_buf_uv().ptr;
		EXPECT_EQ(uv_ptr[8 ], 0.1f);
		EXPECT_EQ(uv_ptr[9 ], 0.4f);
		
		EXPECT_EQ(uv_ptr[10], 0.3f);
		EXPECT_EQ(uv_ptr[11], 0.4f);
		
		EXPECT_EQ(uv_ptr[12], 0.3f);
		EXPECT_EQ(uv_ptr[13], 0.2f);
		
		EXPECT_EQ(uv_ptr[14], 0.1f);
		EXPECT_EQ(uv_ptr[15], 0.2f);

		EXPECT_EQ(r_layer.e_count(), 5);
		EXPECT_EQ(r_layer.get_buf_vertex().count, 5);
		EXPECT_EQ(r_layer.get_buf_uv().count, 5);
	}

	TEST(renderer_layer, delete_buffer) {
		// When reserving, the count should be updated appropriately for the buffers

		using namespace jactorio::renderer;
		using namespace jactorio::core;
	
		auto r_layer = Renderer_layer(false);

		r_layer.reserve(10);
		r_layer.delete_buffer();

		EXPECT_EQ(r_layer.e_count(), 0);

		EXPECT_EQ(r_layer.get_buf_vertex().count, 0);
		EXPECT_EQ(r_layer.get_buf_uv().count, 0);

		EXPECT_EQ(r_layer.get_buf_vertex().ptr, nullptr);
		EXPECT_EQ(r_layer.get_buf_uv().ptr, nullptr);
	}
}
