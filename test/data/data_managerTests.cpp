// 
// data_managerTests.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/22/2019
// Last modified: 04/03/2020
// 

#include <gtest/gtest.h>

#include <filesystem>

#include "data/data_manager.h"
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
			jactorio::data::dataCategory::sprite, "raw-fish", prototype, true);
		const auto proto =
			*data_manager::data_raw_get<jactorio::data::Sprite>(
				jactorio::data::dataCategory::sprite, "__test__/raw-fish");


		// data_manager should populate certain fields, see Prototype_base.h
		// Internal name of prototype should have been renamed to match data_raw name
		// Data category should also have been set to the one provided on add
		EXPECT_EQ(proto.name, "__test__/raw-fish");
		EXPECT_EQ(proto.category, jactorio::data::dataCategory::sprite);
		EXPECT_EQ(proto.internal_id, 1);
		EXPECT_EQ(proto.order, 1);
		// Since no localized name was specified, it uses the internal name
		EXPECT_EQ(proto.get_localized_name(), "__test__/raw-fish");

	}

	TEST(data_manager, data_raw_add_no_directory_prefix) {
		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);

		data_manager::set_directory_prefix("this_should_not_exist");

		jactorio::data::Sprite* const prototype = new jactorio::data::Sprite{};

		data_manager::data_raw_add(
			jactorio::data::dataCategory::sprite, "raw-fish", prototype, false);

		// Prefix __this_should_not_exist/ should not be added
		{
			const auto* proto =
				data_manager::data_raw_get<jactorio::data::Prototype_base>(
					jactorio::data::dataCategory::sprite, "__this_should_not_exist__/raw-fish");
			EXPECT_EQ(proto, nullptr);
		}
		{
			const auto* proto =
				data_manager::data_raw_get<jactorio::data::Prototype_base>(
					jactorio::data::dataCategory::sprite, "raw-fish");
			EXPECT_EQ(proto, prototype);
		}
	}

	TEST(data_manager, data_raw_add_increment_id) {
		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);

		data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "raw-fish0", new jactorio::data::Sprite{});
		data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "raw-fish1", new jactorio::data::Sprite{});
		data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "raw-fish2", new jactorio::data::Sprite{});
		data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "raw-fish3", new jactorio::data::Sprite{});

		const auto proto =
			*data_manager::data_raw_get<jactorio::data::Sprite>(
				jactorio::data::dataCategory::sprite, "raw-fish3");

		EXPECT_EQ(proto.name, "raw-fish3");
		EXPECT_EQ(proto.category, jactorio::data::dataCategory::sprite);
		EXPECT_EQ(proto.internal_id, 4);
	}

	TEST(data_manager, data_raw_override) {
		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);

		data_manager::set_directory_prefix("test");

		// Normal name
		{
			const auto prototype = new jactorio::data::Sprite{};
			prototype->name = "small-electric-pole";

			data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "small-electric-pole",
			                           prototype, true);

			// Override
			const auto prototype2 = new jactorio::data::Sprite{};
			prototype2->name = "small-electric-pole";
			data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "small-electric-pole",
			                           prototype2, true);

			// Get
			const auto proto = data_manager::data_raw_get<jactorio::data::Sprite>(
				jactorio::data::dataCategory::sprite,
				"__test__/small-electric-pole");
			EXPECT_EQ(proto, prototype2);
		}
		data_manager::clear_data();
		// Empty name - Overriding is disabled for empty names, this is for destructor data_raw add
		{
			const auto prototype = new jactorio::data::Sprite{};
			data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "",
			                           prototype, true);

			// No Override
			const auto prototype2 = new jactorio::data::Sprite{};
			data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "",
			                           prototype2, true);

			// Get
			const auto v = data_manager::data_raw_get_all<jactorio::data::Sprite
			>(jactorio::data::dataCategory::sprite);
			EXPECT_EQ(v.size(), 2);


			const auto proto = data_manager::data_raw_get<jactorio::data::Sprite>(
				jactorio::data::dataCategory::sprite, "");

			// The empty name will be automatically assigned to something else
			EXPECT_EQ(proto, nullptr);
		}

	}


	TEST(data_manager, load_data) {
		data_manager::set_directory_prefix("asdf");

		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);

		// Load_data should set the directory prefix based on the subfolder
		data_manager::load_data("data");

		const auto proto =
			data_manager::data_raw_get<jactorio::data::Sprite>(
				jactorio::data::dataCategory::sprite, "__test__/test_tile");

		if (proto == nullptr) {
			FAIL();
		}

		EXPECT_EQ(proto->name, "__test__/test_tile");

		EXPECT_EQ(proto->get_width(), 32);
		EXPECT_EQ(proto->get_height(), 32);
	}

	TEST(data_manager, load_data_invalid_path) {
		// Loading an invalid path will throw filesystem exception
		data_manager::set_directory_prefix("asdf");

		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);

		// Load_data should set the directory prefix based on the subfolder
		try {
			data_manager::load_data("yeet");
			FAIL();
		}
		catch (std::filesystem::filesystem_error&) {
			SUCCEED();
		}
	}

	TEST(data_manager, data_raw_get_invalid) {
		// Should return a nullptr if the item is non-existent
		const auto ptr =
			data_manager::data_raw_get<jactorio::data::Prototype_base>(
				jactorio::data::dataCategory::sprite,
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
		data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "test_tile", prototype);

		const auto prototype2 = new jactorio::data::Sprite{};
		data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "test_tile2", prototype2);

		const std::vector<jactorio::data::Sprite*> paths = data_manager::data_raw_get_all<jactorio::
			data::Sprite>(
			jactorio::data::dataCategory::sprite);

		EXPECT_EQ(contains(paths, "test_tile"), true);
		EXPECT_EQ(contains(paths, "test_tile2"), true);

		EXPECT_EQ(contains(paths, "asdf"), false);
	}

	TEST(data_manager, get_all_sorted) {
		// Retrieved vector should have prototypes sorted in order of addition, first one being added is first in vector
		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);

		const auto prototype = new jactorio::data::Sprite{};
		data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "test_tile", prototype);

		const auto prototype2 = new jactorio::data::Sprite{};
		data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "test_tile2", prototype2);

		const auto prototype3 = new jactorio::data::Sprite{};
		data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "test_tile3", prototype3);

		const auto prototype4 = new jactorio::data::Sprite{};
		data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "test_tile4", prototype4);

		// Get
		const std::vector<jactorio::data::Sprite*> protos = data_manager::data_raw_get_all_sorted<jactorio::
			data::Sprite>(jactorio::data::dataCategory::sprite);


		EXPECT_EQ(protos[0]->name, "test_tile");
		EXPECT_EQ(protos[1]->name, "test_tile2");
		EXPECT_EQ(protos[2]->name, "test_tile3");
		EXPECT_EQ(protos[3]->name, "test_tile4");
	}

	TEST(data_manager, clear_data) {
		const auto prototype = new jactorio::data::Sprite{};

		data_manager::data_raw_add(jactorio::data::dataCategory::sprite, "small-electric-pole",
		                           prototype);

		data_manager::clear_data();


		// Get
		auto* data = data_manager::data_raw_get<jactorio::data::Sprite>(
			jactorio::data::dataCategory::sprite,
			"small-electric-pole");
		EXPECT_EQ(data, nullptr);

		// Get all
		const std::vector<jactorio::data::Sprite*> data_all = data_manager::data_raw_get_all<
			jactorio::data::Sprite>(
			jactorio::data::dataCategory::sprite);

		EXPECT_EQ(data_all.size(), 0);
	}
}
