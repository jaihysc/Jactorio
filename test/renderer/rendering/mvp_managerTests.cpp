// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include <glm/gtc/matrix_transform.hpp>

#include "renderer/opengl/mvp_manager.h"

namespace jactorio::renderer
{
	TEST(MvpManager, View) {
		MvpManager mvp_manager;
		glm::vec3* view_transform = mvp_manager.GetViewTransform();

		EXPECT_EQ(view_transform->x, 0);
		EXPECT_EQ(view_transform->y, 0);
		EXPECT_EQ(view_transform->z, 0);

		view_transform->x = 20;

		glm::vec3* view_transform2 = mvp_manager.GetViewTransform();
		EXPECT_EQ(view_transform2->x, 20);
	}

	TEST(MvpManager, ProjectionZoom) {
		// X axis is longer
		{
			const int tiles_x = 20;
			const int tiles_y = 10;

			// Zoom guarantees a minimum of offset
			auto proj_matrix = MvpManager::ToProjMatrix(tiles_x, tiles_y, 0);

			EXPECT_EQ(proj_matrix, glm::ortho(
				          0.f, 20.f,
				          10.f, 0.f,
				          -1.f, 1.f)
			);

			// Zoom guarantees a minimum of offset
			proj_matrix = MvpManager::ToProjMatrix(tiles_x, tiles_y, 2.5);

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
			const int tiles_x = 10;
			const int tiles_y = 20;

			// Zoom guarantees a minimum of offset
			auto proj_matrix = MvpManager::ToProjMatrix(tiles_x, tiles_y, 0);

			EXPECT_EQ(proj_matrix, glm::ortho(
				          0.f, 10.f,
				          20.f, 0.f,
				          -1.f, 1.f)
			);

			// Zoom guarantees a minimum of offset
			proj_matrix = MvpManager::ToProjMatrix(tiles_x, tiles_y, 2.5);

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
