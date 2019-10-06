#include <gtest/gtest.h>

#include <glm/gtc/matrix_transform.hpp>

#include "renderer/manager/mvp_manager.h"

TEST(mvp_manager, view) {
	glm::vec3* view_transform = jactorio::renderer::Mvp_manager::get_view_transform();

	EXPECT_EQ(view_transform->x, 0);
	EXPECT_EQ(view_transform->y, 0);
	EXPECT_EQ(view_transform->z, 0);
	
	view_transform->x = 20;

	glm::vec3* view_transform2 = jactorio::renderer::Mvp_manager::get_view_transform();
	EXPECT_EQ(view_transform2->x, 20);
}

TEST(mvp_manager, projection) {
	const auto tile_prop = jactorio::renderer::Mvp_manager::projection_calculate_tile_properties(
		10, 1920, 1080);

	EXPECT_EQ(tile_prop.tiles_x, 192);
	EXPECT_EQ(tile_prop.tiles_y, 108);

	// Matrix
	const auto proj_matrix = jactorio::renderer::Mvp_manager::to_proj_matrix(tile_prop);

	EXPECT_EQ(proj_matrix, glm::ortho(
		0.f, 192.f,
		108.f, 0.f,
		-1.f, 1.f)
	);
}