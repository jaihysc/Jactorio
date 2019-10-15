#include <gtest/gtest.h>

#include "data/data_manager.h"

// This test may fail if the working directory is not directly in the executable directory
// Bad test
TEST(data_manager, load_data) {
	jactorio::data::data_manager::load_data("data");
	const std::string path = jactorio::data::data_manager::get_data(
		jactorio::data::data_type::graphics, "grass-1");

	EXPECT_EQ(
		path,
		"~/data/test/graphics/grass/grassTexture_RegisteredToGrass-1.png");
}

TEST(data_manager, get_data_non_existant) {
	const std::string path = jactorio::data::data_manager::get_data(
		jactorio::data::data_type::audio, "adsjfalkshvkjhjasdflkj");
	EXPECT_EQ(path, "!");
}
