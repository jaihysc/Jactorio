// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/12/2020

#include <gtest/gtest.h>

#include "renderer/rendering/renderer_layer.h"

namespace renderer
{
	class RendererLayerTest : public testing::Test
	{
	protected:
		jactorio::renderer::Renderer_layer r_layer_;
		
	};

	TEST_F(RendererLayerTest, PushBack) {
		// Expect buffer to be expanded to accommodate
		// It will push back at the next place after the highest set
		r_layer_.reserve(10);

		r_layer_.set(9,
		            jactorio::renderer::Renderer_layer::Element(
			            jactorio::core::Quad_position(
							{0.0, 0.0},
							{0.0, 0.0}
			            ),
		            jactorio::core::Quad_position(
						{0.0, 0.0},
						{0.0, 0.0}
			            )
		            )
		);
		// Resize
		r_layer_.push_back(
			jactorio::renderer::Renderer_layer::Element(
				jactorio::core::Quad_position(
					{0.11, 0.22},
					{0.33, 0.44}
				),
				jactorio::core::Quad_position(
					{0.1, 0.2},
					{0.3, 0.4}
				)
			)
		);

		const auto vertex_ptr = r_layer_.get_buf_vertex().ptr;

		EXPECT_EQ(vertex_ptr[80], 0.11f);
		EXPECT_EQ(vertex_ptr[81], 0.22f);

		EXPECT_EQ(vertex_ptr[82], 0.33f);
		EXPECT_EQ(vertex_ptr[83], 0.22f);

		EXPECT_EQ(vertex_ptr[84], 0.33f);
		EXPECT_EQ(vertex_ptr[85], 0.44f);

		EXPECT_EQ(vertex_ptr[86], 0.11f);
		EXPECT_EQ(vertex_ptr[87], 0.44f);

		const auto uv_ptr = r_layer_.get_buf_uv().ptr;
		EXPECT_EQ(uv_ptr[80], 0.1f);
		EXPECT_EQ(uv_ptr[81], 0.4f);

		EXPECT_EQ(uv_ptr[82], 0.3f);
		EXPECT_EQ(uv_ptr[83], 0.4f);

		EXPECT_EQ(uv_ptr[84], 0.3f);
		EXPECT_EQ(uv_ptr[85], 0.2f);

		EXPECT_EQ(uv_ptr[86], 0.1f);
		EXPECT_EQ(uv_ptr[87], 0.2f);

		// Size not tested since the new increase in capacity to acquire is arbitrary
		// EXPECT_EQ(r_layer.e_count(), 11);
		// EXPECT_EQ(r_layer.get_buf_vertex().count, 11);
		// EXPECT_EQ(r_layer.get_buf_uv().count, 11);
	}

	TEST_F(RendererLayerTest, SetElement) {
		// Set element at element position 1
		// Reading from the buffer, this is index 8 - 15
		r_layer_.reserve(10);
		r_layer_.set(1,
		            jactorio::renderer::Renderer_layer::Element(
			            jactorio::core::Quad_position(
				            {0.11, 0.22},
				            {0.33, 0.44}
			            ),
			            jactorio::core::Quad_position(
				            {0.1, 0.2},
				            {0.3, 0.4}
			            )
		            )
		);

		const auto vertex_ptr = r_layer_.get_buf_vertex().ptr;

		EXPECT_EQ(vertex_ptr[8], 0.11f);
		EXPECT_EQ(vertex_ptr[9], 0.22f);

		EXPECT_EQ(vertex_ptr[10], 0.33f);
		EXPECT_EQ(vertex_ptr[11], 0.22f);

		EXPECT_EQ(vertex_ptr[12], 0.33f);
		EXPECT_EQ(vertex_ptr[13], 0.44f);

		EXPECT_EQ(vertex_ptr[14], 0.11f);
		EXPECT_EQ(vertex_ptr[15], 0.44f);

		const auto uv_ptr = r_layer_.get_buf_uv().ptr;
		EXPECT_EQ(uv_ptr[8 ], 0.1f);
		EXPECT_EQ(uv_ptr[9 ], 0.4f);

		EXPECT_EQ(uv_ptr[10], 0.3f);
		EXPECT_EQ(uv_ptr[11], 0.4f);

		EXPECT_EQ(uv_ptr[12], 0.3f);
		EXPECT_EQ(uv_ptr[13], 0.2f);

		EXPECT_EQ(uv_ptr[14], 0.1f);
		EXPECT_EQ(uv_ptr[15], 0.2f);
	}

	TEST_F(RendererLayerTest, Reserve) {
		// When reserving, the count should be updated appropriately for the buffers
		r_layer_.reserve(10);

		EXPECT_EQ(r_layer_.get_capacity(), 10);

		EXPECT_EQ(r_layer_.get_buf_vertex().count, 10);
		EXPECT_EQ(r_layer_.get_buf_uv().count, 10);
	}

	TEST_F(RendererLayerTest, Resize) {
		// Ensure values still remain in correct places after resizing
		r_layer_.reserve(10);
		r_layer_.set(1,
		            jactorio::renderer::Renderer_layer::Element(
			            jactorio::core::Quad_position(
				            {0.11, 0.22},
				            {0.33, 0.44}
			            ),
			            jactorio::core::Quad_position(
				            {0.1, 0.2},
				            {0.3, 0.4}
			            )
		            )
		);

		r_layer_.resize(5);

		// Ensure values still remain after resizing
		const auto vertex_ptr = r_layer_.get_buf_vertex().ptr;
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

		const auto uv_ptr = r_layer_.get_buf_uv().ptr;
		EXPECT_EQ(uv_ptr[8 ], 0.1f);
		EXPECT_EQ(uv_ptr[9 ], 0.4f);

		EXPECT_EQ(uv_ptr[10], 0.3f);
		EXPECT_EQ(uv_ptr[11], 0.4f);

		EXPECT_EQ(uv_ptr[12], 0.3f);
		EXPECT_EQ(uv_ptr[13], 0.2f);

		EXPECT_EQ(uv_ptr[14], 0.1f);
		EXPECT_EQ(uv_ptr[15], 0.2f);

		EXPECT_EQ(r_layer_.get_capacity(), 5);
		EXPECT_EQ(r_layer_.get_element_count(), 2);  // It assumes an element exists at position 0 since one was set at 1

		EXPECT_EQ(r_layer_.get_buf_vertex().count, 5);
		EXPECT_EQ(r_layer_.get_buf_uv().count, 5);
	}

	TEST_F(RendererLayerTest, DeleteBuffer) {
		// When reserving, the count should be updated appropriately for the buffers
		r_layer_.reserve(10);
		r_layer_.delete_buffer();

		EXPECT_EQ(r_layer_.get_capacity(), 0);

		EXPECT_EQ(r_layer_.get_buf_vertex().count, 0);
		EXPECT_EQ(r_layer_.get_buf_uv().count, 0);

		EXPECT_EQ(r_layer_.get_buf_vertex().ptr, nullptr);
		EXPECT_EQ(r_layer_.get_buf_uv().ptr, nullptr);
	}

	TEST(RendererLayer, GenRenderGridIndices) {
		unsigned int* grid = nullptr;
		jactorio::core::Capturing_guard<void()> guard([&] { delete[] grid; });

		grid = jactorio::renderer::Renderer_layer::gen_render_grid_indices(121);
		// Indices generation pattern:
		// top left
		// top right
		// bottom right
		// bottom left

		// 0, 0
		EXPECT_EQ(grid[0], 0);
		EXPECT_EQ(grid[1], 1);
		EXPECT_EQ(grid[2], 2);

		EXPECT_EQ(grid[3], 2);
		EXPECT_EQ(grid[4], 3);
		EXPECT_EQ(grid[5], 0);

		// 10, 10
		EXPECT_EQ(grid[720], 480);
		EXPECT_EQ(grid[721], 481);
		EXPECT_EQ(grid[722], 482);

		EXPECT_EQ(grid[723], 482);
		EXPECT_EQ(grid[724], 483);
		EXPECT_EQ(grid[725], 480);
	}
}
