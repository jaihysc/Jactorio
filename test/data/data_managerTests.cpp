#include <gtest/gtest.h>

#include "data/data_manager.h"
#include "data/pybind/pybind_manager.h"
namespace data_manager = jactorio::data::data_manager;

TEST(data_manager, data_raw_get_invalid) {
	// Should return a nullptr if the item is non-existent
	const auto ptr = data_manager::data_raw_get(jactorio::data::data_category::tile,
	                                            "asdfjsadhfkjdsafhs");
	EXPECT_EQ(ptr, nullptr);
}

TEST(data_manager, data_raw_add) {
	jactorio::data::Prototype_base prototype{};
	prototype.name = "fishey";

	data_manager::data_raw_add(jactorio::data::data_category::tile, "raw-fish", prototype);
	const auto proto = *data_manager::data_raw_get(jactorio::data::data_category::tile, "raw-fish");
	// Internal name of prototype should have been renamed to match data_raw name
	EXPECT_EQ(proto.name, "raw-fish");
}

TEST(data_manager, data_raw_override) {
	sf::Image img1{};
	img1.create(10, 10);

	sf::Image img2{};
	img2.create(20, 20);

	jactorio::data::Prototype_base prototype{};
	prototype.name = "small-electric-pole";
	prototype.sprite = img1;

	data_manager::data_raw_add(jactorio::data::data_category::tile, "small-electric-pole",
	                           prototype);

	// Override
	jactorio::data::Prototype_base prototype2{};
	prototype2.name = "small-electric-pole";
	prototype2.sprite = img2;
	data_manager::data_raw_add(jactorio::data::data_category::tile, "small-electric-pole",
	                           prototype2);

	// Get
	auto proto = *data_manager::data_raw_get(jactorio::data::data_category::tile,
	                                         "small-electric-pole");
	EXPECT_NE(img1.getSize(), proto.sprite.getSize());
	EXPECT_EQ(img2.getSize(), proto.sprite.getSize());

}

TEST(data_manager, load_data) {
	jactorio::data::pybind_manager::py_interpreter_init();
	data_manager::load_data("data");

	const auto proto = data_manager::data_raw_get(jactorio::data::data_category::tile, "test_tile");

	if (proto == nullptr)
		FAIL();

	EXPECT_EQ(proto->name, "test_tile");

	const auto sprite_size = proto->sprite.getSize();
	EXPECT_EQ(sprite_size.x, 32);
	EXPECT_EQ(sprite_size.y, 32);

	jactorio::data::pybind_manager::py_interpreter_terminate();
}

TEST(data_manager, get_data_non_existant) {
	const std::string path = data_manager::get_path(
		jactorio::data::data_category::audio, "adsjfalkshvkjhjasdflkj");
	EXPECT_EQ(path, "!");
}


bool contains(const std::vector<jactorio::data::Prototype_base>& vector, const std::string& key) {
	for (const auto& i : vector) {
		if (i.name == key) {
			return true;
		}
	}

	return false;
}

TEST(data_manager, get_all_data_of_type) {
	jactorio::data::pybind_manager::py_interpreter_init();
	data_manager::load_data("data");

	const auto paths = data_manager::get_all_data(
		jactorio::data::data_category::tile);

	EXPECT_EQ(contains(paths, "test_tile"), true);
	EXPECT_EQ(contains(paths, "asdf"), false);
	jactorio::data::pybind_manager::py_interpreter_terminate();
}
