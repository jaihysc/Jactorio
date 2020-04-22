// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include <gtest/gtest.h>

#include <glm/gtc/matrix_transform.hpp>

#include "renderer/rendering/mvp_manager.h"

namespace renderer
{
	TEST(MvpManager, View) {
		glm::vec3* view_transform = jactorio::renderer::get_view_transform();

		EXPECT_EQ(view_transform->x, 0);
		EXPECT_EQ(view_transform->y, 0);
		EXPECT_EQ(view_transform->z, 0);

		view_transform->x = 20;

		glm::vec3* view_transform2 = jactorio::renderer::get_view_transform();
		EXPECT_EQ(view_transform2->x, 20);
	}

	TEST(MvpManager, ProjectionZoom) {
		// X axis is longer
		{
			jactorio::renderer::Projection_tile_data tile_prop{};

			tile_prop.tiles_x = 20;
			tile_prop.tiles_y = 10;

			// Zoom guarantees a minimum of offset
			auto proj_matrix = jactorio::renderer::to_proj_matrix(
				tile_prop.tiles_x, tile_prop.tiles_y, 0);

			EXPECT_EQ(proj_matrix, glm::ortho(
				          0.f, 20.f,
				          10.f, 0.f,
				          -1.f, 1.f)
			);

			// Zoom guarantees a minimum of offset
			proj_matrix = jactorio::renderer::to_proj_matrix(
				tile_prop.tiles_x, tile_prop.tiles_y, 2.5);

			// Y is smaller axis

			// Scale factor of 2 for X axis
			// L R B T
			EXPECT_EQ(proj_matrix, glm::ortho(
				          5.f, 15.f,
				          7.5f, 2.5f,
				          -1.f, 1.f)
			);
		}

		// Y axis is longer
		{
			jactorio::renderer::Projection_tile_data tile_prop{};

			tile_prop.tiles_x = 10;
			tile_prop.tiles_y = 20;

			// Zoom guarantees a minimum of offset
			auto proj_matrix = jactorio::renderer::to_proj_matrix(
				tile_prop.tiles_x, tile_prop.tiles_y, 0);

			EXPECT_EQ(proj_matrix, glm::ortho(
				          0.f, 10.f,
				          20.f, 0.f,
				          -1.f, 1.f)
			);

			// Zoom guarantees a minimum of offset
			proj_matrix = jactorio::renderer::to_proj_matrix(
				tile_prop.tiles_x, tile_prop.tiles_y, 2.5);

			// Scale factor of 2 for Y axis
			// L R B T
			EXPECT_EQ(proj_matrix, glm::ortho(
				          2.5f, 7.5f,
				          15.f, 5.f,
				          -1.f, 1.f)
			);
		}
	}
}
