#include <gtest/gtest.h>

#include "data/data_manager.h"
#include "data/pybind/pybind_manager.h"
#include "data/prototype/sprite.h"
#include "core/resource_guard.h"

namespace data
{
	namespace data_manager = jactorio::data::data_manager;

	TEST(data_manager, data_raw_add) {
		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);

		data_manager::set_directory_prefix("test");

		const auto prototype = new jactorio::data::Sprite{};
		prototype->name = "fishey";

		data_manager::data_raw_add(
			jactorio::data::data_category::sprite, "raw-fish", prototype, true);
		const auto proto =
			*data_manager::data_raw_get<jactorio::data::Prototype_base>(
				jactorio::data::data_category::sprite, "__test__/raw-fish");

		
		// data_manager should populate certain fields, see Prototype_base.h
		// Internal name of prototype should have been renamed to match data_raw name
		// Data category should also have been set to the one provided on add
		EXPECT_EQ(proto.name, "__test__/raw-fish");
		EXPECT_EQ(proto.category, jactorio::data::data_category::sprite);
		EXPECT_EQ(proto.internal_id, 1);
		EXPECT_EQ(proto.order, 1);
	}

	TEST(data_manager, data_raw_add_no_directory_prefix) {
		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);
		
		data_manager::set_directory_prefix("this_should_not_exist");

		jactorio::data::Sprite* const prototype = new jactorio::data::Sprite{};

		data_manager::data_raw_add(
			jactorio::data::data_category::sprite, "raw-fish", prototype, false);

		// Prefix __this_should_not_exist/ should not be added
		{
			const auto* proto =
				data_manager::data_raw_get<jactorio::data::Prototype_base>(
					jactorio::data::data_category::sprite, "__this_should_not_exist__/raw-fish");
			EXPECT_EQ(proto, nullptr);
		}
		{
			const auto* proto =
				data_manager::data_raw_get<jactorio::data::Prototype_base>(
					jactorio::data::data_category::sprite, "raw-fish");
			EXPECT_EQ(proto, prototype);
		}
	}

	TEST(data_manager, data_raw_add_increment_id) {
		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);

		const auto prototype = new jactorio::data::Sprite{};

		data_manager::data_raw_add(jactorio::data::data_category::sprite, "raw-fish", prototype);
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "raw-fish", prototype);
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "raw-fish", prototype);
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "raw-fish", prototype);

		const auto proto =
			*data_manager::data_raw_get<jactorio::data::Prototype_base>(
				jactorio::data::data_category::sprite, "raw-fish");

		EXPECT_EQ(proto.name, "raw-fish");
		EXPECT_EQ(proto.category, jactorio::data::data_category::sprite);
		EXPECT_EQ(proto.internal_id, 4);
	}

	TEST(data_manager, data_raw_override) {
		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);
		
		data_manager::set_directory_prefix("test");

		const auto prototype = new jactorio::data::Sprite{};
		prototype->name = "small-electric-pole";

		data_manager::data_raw_add(jactorio::data::data_category::sprite, "small-electric-pole",
		                           prototype, true);

		// Override
		const auto prototype2 = new jactorio::data::Sprite{};
		prototype2->name = "small-electric-pole";
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "small-electric-pole",
		                           prototype2, true);

		// Get
		const auto proto = data_manager::data_raw_get<jactorio::data::Sprite>(
			jactorio::data::data_category::sprite,
			"__test__/small-electric-pole");

		EXPECT_EQ(proto, prototype2);
	}

	
	TEST(data_manager, load_data) {
		data_manager::set_directory_prefix("asdf");

		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);
		auto py_guard = jactorio::core::Resource_guard(jactorio::data::pybind_manager::py_interpreter_terminate);
		jactorio::data::pybind_manager::py_interpreter_init();
		
		// Load_data should set the directory prefix based on the subfolder
		EXPECT_EQ(data_manager::load_data("data"), 0);

		const auto proto =
			data_manager::data_raw_get<jactorio::data::Sprite>(
				jactorio::data::data_category::sprite, "__test__/test_tile");

		if (proto == nullptr) {
			FAIL();
		}

		EXPECT_EQ(proto->name, "__test__/test_tile");

		EXPECT_EQ(proto->get_width(), 32);
		EXPECT_EQ(proto->get_height(), 32);
	}

	TEST(data_manager, data_raw_get_invalid) {
		// Should return a nullptr if the item is non-existent
		const auto ptr =
			data_manager::data_raw_get<jactorio::data::Prototype_base>(
				jactorio::data::data_category::sprite,
				"asdfjsadhfkjdsafhs");

		EXPECT_EQ(ptr, nullptr);
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
		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);
		
		const auto prototype = new jactorio::data::Sprite{};
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "test_tile", prototype);

		const auto prototype2 = new jactorio::data::Sprite{};
		data_manager::data_raw_add(jactorio::data::data_category::sprite, "test_tile2", prototype2);
		
		const std::vector<jactorio::data::Sprite*> paths = data_manager::data_raw_get_all<jactorio::
			data::Sprite>(
			jactorio::data::data_category::sprite);

		EXPECT_EQ(contains(paths, "test_tile"), true);
		EXPECT_EQ(contains(paths, "test_tile2"), true);
		
		EXPECT_EQ(contains(paths, "asdf"), false);
	}

	TEST(data_manager, clear_data) {
		data_manager::set_directory_prefix("test");

		const auto prototype = new jactorio::data::Sprite{};

		data_manager::data_raw_add(jactorio::data::data_category::sprite, "small-electric-pole",
		                           prototype);

		data_manager::clear_data();


		// Get
		auto* data = data_manager::data_raw_get<jactorio::data::Sprite>(
			jactorio::data::data_category::sprite,
			"__test__/small-electric-pole");
		EXPECT_EQ(data, nullptr);

		// Get all
		const std::vector<jactorio::data::Sprite*> data_all = data_manager::data_raw_get_all<
			jactorio::data::Sprite>(
			jactorio::data::data_category::sprite);

		EXPECT_EQ(data_all.size(), 0);
	}
}
