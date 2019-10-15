#include <gtest/gtest.h>

#include <glm/gtc/matrix_transform.hpp>

#include "renderer/rendering/mvp_manager.h"

TEST(mvp_manager, view) {
	glm::vec3* view_transform = jactorio::renderer::mvp_manager::
		get_view_transform();

	EXPECT_EQ(view_transform->x, 0);
	EXPECT_EQ(view_transform->y, 0);
	EXPECT_EQ(view_transform->z, 0);

	view_transform->x = 20;

	glm::vec3* view_transform2 = jactorio::renderer::mvp_manager::
		get_view_transform();
	EXPECT_EQ(view_transform2->x, 20);
}

TEST(mvp_manager, projection_tile_calculation_and_matrix) {
	const auto tile_prop = jactorio::renderer::mvp_manager::
		projection_calculate_tile_properties(
			10, 1920, 1080);

	EXPECT_EQ(tile_prop.tiles_x, 192);
	EXPECT_EQ(tile_prop.tiles_y, 108);

	// Matrix
	const auto proj_matrix = to_proj_matrix(tile_prop, 0);

	EXPECT_EQ(proj_matrix, glm::ortho(
		          0.f, 192.f,
		          108.f, 0.f,
		          -1.f, 1.f)
	);
}

TEST(mvp_manager, projection_zoom) {
	jactorio::renderer::mvp_manager::Projection_tile_data tile_prop{};

	tile_prop.tiles_x = 20;
	tile_prop.tiles_y = 30;

	const auto proj_matrix = to_proj_matrix(tile_prop, 10);


	EXPECT_EQ(proj_matrix, glm::ortho(
		          10.f, 10.f,
		          15.f, 15.f,
		          -1.f, 1.f)
	);

}
