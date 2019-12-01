#include <gtest/gtest.h>

#include <SFML/Graphics/Image.hpp>

#include "data/data_manager.h"
#include "data/pybind/pybind_manager.h"
#include "data/prototype/sprite.h"

namespace data
{
	namespace data_manager = jactorio::data::data_manager;

	TEST(data_manager, data_raw_get_invalid) {
		// Should return a nullptr if the item is non-existent
		const auto ptr =
			data_manager::data_raw_get<jactorio::data::Prototype_base>(
				jactorio::data::data_category::sprite,
				"asdfjsadhfkjdsafhs");

		EXPECT_EQ(ptr, nullptr);
	}

	TEST(data_manager, data_raw_add) {
		jactorio::data::Sprite prototype{};
		prototype.name = "fishey";

		data_manager::data_raw_add(jactorio::data::data_category::sprite, "raw-fish", &prototype);
		const auto proto =
			*data_manager::data_raw_get<jactorio::data::Prototype_base>(
				jactorio::data::data_category::sprite, "raw-fish");

		// Internal name of prototype should have been renamed to match data_raw name
		// Data category should also have been set to the one provided on add
		EXPECT_EQ(proto.name, "raw-fish");
		EXPECT_EQ(proto.category, jactorio::data::data_category::sprite);
	}

	TEST(data_manager, data_raw_override) {
		sf::Image img1{};
		img1.create(10, 10);

		sf::Image img2{};
		img2.create(20, 20);

		jactorio::data::Sprite prototype{};
		prototype.name = "small-electric-pole";
		prototype.sprite_image = img1;

		data_manager::data_raw_add(jactorio::data::data_category::sprite, "small-electric-pole",
		                           &prototype);

		// Override
		jactorio::data::Sprite prototype2{};
		prototype2.name = "small-electric-pole";
		prototype2.sprite_image = img2;
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "small-electric-pole",
		                           &prototype2);

		// Get
		const auto proto = *data_manager::data_raw_get<jactorio::data::Sprite>(
			jactorio::data::data_category::sprite,
			"small-electric-pole");

		EXPECT_NE(img1.getSize(), proto.sprite_image.getSize());
		EXPECT_EQ(img2.getSize(), proto.sprite_image.getSize());

	}

	TEST(data_manager, load_data) {
		jactorio::data::pybind_manager::py_interpreter_init();
		data_manager::load_data("data");

		const auto proto =
			data_manager::data_raw_get<jactorio::data::Sprite>(
				jactorio::data::data_category::sprite, "test_tile");

		if (proto == nullptr) {
			FAIL();
		}

		EXPECT_EQ(proto->name, "test_tile");

		const auto sprite_size = proto->sprite_image.getSize();
		EXPECT_EQ(sprite_size.x, 32);
		EXPECT_EQ(sprite_size.y, 32);

		jactorio::data::pybind_manager::py_interpreter_terminate();
	}

	bool contains(const std::vector<jactorio::data::Sprite*>& vector, const std::string& key) {
		for (const auto& i : vector) {
			if (i->name == key) {
				return true;
			}
		}

		return false;
	}

	TEST(data_manager, get_all_data_of_type) {
		jactorio::data::pybind_manager::py_interpreter_init();
		data_manager::load_data("data");

		const std::vector<jactorio::data::Sprite*> paths = data_manager::data_raw_get_all<jactorio::
			data::Sprite>(
			jactorio::data::data_category::sprite);

		EXPECT_EQ(contains(paths, "test_tile"), true);
		EXPECT_EQ(contains(paths, "asdf"), false);
		jactorio::data::pybind_manager::py_interpreter_terminate();
	}

	TEST(data_manager, clear_data) {
		sf::Image img1{};
		img1.create(10, 10);

		jactorio::data::Sprite prototype{};
		prototype.sprite_image = img1;

		data_manager::data_raw_add(jactorio::data::data_category::sprite, "small-electric-pole",
		                           &prototype);

		data_manager::clear_data();


		// Get
		auto* data = data_manager::data_raw_get<jactorio::data::Sprite>(
			jactorio::data::data_category::sprite,
			"small-electric-pole");
		EXPECT_EQ(data, nullptr);

		// Get all
		const std::vector<jactorio::data::Sprite*> data_all = data_manager::data_raw_get_all<
			jactorio::data::Sprite>(
			jactorio::data::data_category::sprite);

		EXPECT_EQ(data_all.size(), 0);
	}
}
