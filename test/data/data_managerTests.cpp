#include <gtest/gtest.h>

#include "data/data_manager.h"

// These test fail if the working directory is not directly in the executable directory
// Bad test
TEST(data_manager, load_data) {
	jactorio::data::data_manager::load_data("data");

	// Get path
	const std::string path = jactorio::data::data_manager::get_path(
		jactorio::data::data_type::graphics, "grass-1");

	EXPECT_EQ(
		path,
		"~/data/test/graphics/grass/grassTexture_RegisteredToGrass-1.png");

	
	// Get iname
	const std::string iname = jactorio::data::data_manager::get_iname(
		jactorio::data::data_type::graphics,
		"~/data/test/graphics/grass/grassTexture_RegisteredToGrass-1.png");
	
	EXPECT_EQ(
		iname,
		"grass-1");
}

TEST(data_manager, get_data_non_existant) {
	const std::string path = jactorio::data::data_manager::get_path(
		jactorio::data::data_type::audio, "adsjfalkshvkjhjasdflkj");
	EXPECT_EQ(path, "!");
}

bool contains(const std::vector<std::string>& vector, const std::string& key) {
	for (const auto& i : vector) {
		if (i == key) {
			return true;
		}
	}

	return false;
}

TEST(data_manager, get_all_data_of_type) {
	jactorio::data::data_manager::load_data("data");

	const auto paths = jactorio::data::data_manager::get_all_data(
		jactorio::data::data_type::graphics);

	EXPECT_EQ(contains(paths, "~/data/test/graphics/grass/grassTexture_RegisteredToGrass-1.png"), true);
	EXPECT_EQ(contains(paths, "dasdfskjdfhlkjahs.png"), false);
}
