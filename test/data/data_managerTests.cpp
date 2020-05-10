// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include <gtest/gtest.h>

#include <filesystem>

#include "data/data_manager.h"
#include "data/prototype/sprite.h"

namespace data
{
	class DataManagerTest : public testing::Test
	{
	protected:
		void TearDown() override {
			jactorio::data::ClearData();
		}

		/// \brief Returns true if element exists in vector
		static bool Contains(const std::vector<jactorio::data::Sprite*>& vector, const std::string& key) {
			for (const auto& i : vector) {
				if (i->name == key) {
					return true;
				}
			}

			return false;
		}
	};

	TEST_F(DataManagerTest, DataRawAdd) {
		jactorio::data::SetDirectoryPrefix("test");

		DataRawAdd("raw-fish", new jactorio::data::Sprite{}, true);

		const auto* proto =
			jactorio::data::DataRawGet<jactorio::data::Sprite>(
				jactorio::data::DataCategory::sprite,
				"__test__/raw-fish");


		// data_manager should populate certain fields, see Prototype_base.h
		// Internal name of prototype should have been renamed to match data_raw name
		// Data category should also have been set to the one provided on add
		EXPECT_EQ(proto->name, "__test__/raw-fish");
		EXPECT_EQ(proto->Category(), jactorio::data::DataCategory::sprite);
		EXPECT_EQ(proto->internalId, 1);
		EXPECT_EQ(proto->order, 1);
		// Since no localized name was specified, it uses the internal name
		EXPECT_EQ(proto->GetLocalizedName(), "__test__/raw-fish");

	}

	TEST_F(DataManagerTest, DataRawAddNoDirectoryPrefix) {
		jactorio::data::SetDirectoryPrefix("this_should_not_exist");

		auto* prototype = new jactorio::data::Sprite{};
		DataRawAdd("raw-fish", prototype, false);

		// Prefix __this_should_not_exist/ should not be added
		{
			const auto* proto =
				jactorio::data::DataRawGet<jactorio::data::PrototypeBase>(
					jactorio::data::DataCategory::sprite,
					"__this_should_not_exist__/raw-fish");
			EXPECT_EQ(proto, nullptr);
		}
		{
			const auto* proto =
				jactorio::data::DataRawGet<jactorio::data::PrototypeBase>(
					jactorio::data::DataCategory::sprite,
					"raw-fish");
			EXPECT_EQ(proto, prototype);
		}
	}

	TEST_F(DataManagerTest, DataRawAddIncrementId) {
		DataRawAdd("raw-fish0", new jactorio::data::Sprite{});
		DataRawAdd("raw-fish1", new jactorio::data::Sprite{});
		DataRawAdd("raw-fish2", new jactorio::data::Sprite{});
		DataRawAdd("raw-fish3", new jactorio::data::Sprite{});

		const auto* proto =
			jactorio::data::DataRawGet<jactorio::data::Sprite>(
				jactorio::data::DataCategory::sprite,
				"raw-fish3");

		EXPECT_EQ(proto->name, "raw-fish3");
		EXPECT_EQ(proto->Category(), jactorio::data::DataCategory::sprite);
		EXPECT_EQ(proto->internalId, 4);
	}

	TEST_F(DataManagerTest, DataRawOverride) {
		jactorio::data::SetDirectoryPrefix("test");

		// Normal name
		{
			auto* prototype = new jactorio::data::Sprite{};
			DataRawAdd("small-electric-pole", prototype, true);

			// Override
			auto* prototype2 = new jactorio::data::Sprite{};
			DataRawAdd("small-electric-pole", prototype2, true);

			// Get
			const auto proto = jactorio::data::DataRawGet<jactorio::data::Sprite>(
				jactorio::data::DataCategory::sprite,
				"__test__/small-electric-pole");

			EXPECT_EQ(proto, prototype2);
		}

		jactorio::data::ClearData();
		// Empty name - Overriding is disabled for empty names, this is for destructor data_raw add
		// Instead, it will assign an auto generated name
		{
			auto* prototype = new jactorio::data::Sprite{};
			DataRawAdd("", prototype, true);

			// No Override
			auto* prototype2 = new jactorio::data::Sprite{};
			DataRawAdd("", prototype2, true);

			// Get
			const auto sprite_protos =
				jactorio::data::DataRawGetAll<jactorio::data::Sprite>(jactorio::data::DataCategory::sprite);
			EXPECT_EQ(sprite_protos.size(), 2);


			const auto proto = jactorio::data::DataRawGet<jactorio::data::Sprite>(
				jactorio::data::DataCategory::sprite,
				"");

			// The empty name will be automatically assigned to something else
			EXPECT_EQ(proto, nullptr);
		}

	}


	TEST_F(DataManagerTest, LoadData) {
		jactorio::data::SetDirectoryPrefix("asdf");

		// Load_data should set the directory prefix based on the subfolder
		jactorio::data::LoadData("data");

		const auto proto =
			jactorio::data::DataRawGet<jactorio::data::Sprite>(
				jactorio::data::DataCategory::sprite,
				"__test__/test_tile");

		if (proto == nullptr) {
			FAIL();
		}

		EXPECT_EQ(proto->name, "__test__/test_tile");

		EXPECT_EQ(proto->GetWidth(), 32);
		EXPECT_EQ(proto->GetHeight(), 32);
	}

	TEST_F(DataManagerTest, LoadDataInvalidPath) {
		// Loading an invalid path will throw filesystem exception
		jactorio::data::SetDirectoryPrefix("asdf");

		// Load_data should set the directory prefix based on the subfolder
		try {
			jactorio::data::LoadData("yeet");
			FAIL();
		}
		catch (std::filesystem::filesystem_error&) {
			SUCCEED();
		}
	}

	TEST_F(DataManagerTest, DataRawGetInvalid) {
		// Should return a nullptr if the item is non-existent
		const auto ptr =
			jactorio::data::DataRawGet<jactorio::data::PrototypeBase>(jactorio::data::DataCategory::sprite,
			                                                          "asdfjsadhfkjdsafhs");

		EXPECT_EQ(ptr, nullptr);
	}


	TEST_F(DataManagerTest, GetAllDataOfType) {
		DataRawAdd("test_tile1", new jactorio::data::Sprite{});
		DataRawAdd("test_tile2", new jactorio::data::Sprite{});

		const std::vector<jactorio::data::Sprite*> paths = jactorio::data::DataRawGetAll<jactorio::
			data::Sprite>(
			jactorio::data::DataCategory::sprite);

		EXPECT_EQ(Contains(paths, "test_tile1"), true);
		EXPECT_EQ(Contains(paths, "test_tile2"), true);

		EXPECT_EQ(Contains(paths, "asdf"), false);
	}

	TEST_F(DataManagerTest, GetAllSorted) {
		// Retrieved vector should have prototypes sorted in order of addition, first one being added is first in vector
		DataRawAdd("test_tile1", new jactorio::data::Sprite{});
		DataRawAdd("test_tile2", new jactorio::data::Sprite{});
		DataRawAdd("test_tile3", new jactorio::data::Sprite{});
		DataRawAdd("test_tile4", new jactorio::data::Sprite{});

		// Get
		const std::vector<jactorio::data::Sprite*> protos =
			jactorio::data::DataRawGetAllSorted<jactorio::data::Sprite>(jactorio::data::DataCategory::sprite);

		EXPECT_EQ(protos[0]->name, "test_tile1");
		EXPECT_EQ(protos[1]->name, "test_tile2");
		EXPECT_EQ(protos[2]->name, "test_tile3");
		EXPECT_EQ(protos[3]->name, "test_tile4");
	}

	TEST_F(DataManagerTest, ClearData) {
		DataRawAdd("small-electric-pole", new jactorio::data::Sprite{});

		jactorio::data::ClearData();

		// Get
		auto* data =
			jactorio::data::DataRawGet<jactorio::data::Sprite>(
				jactorio::data::DataCategory::sprite,
				"small-electric-pole");

		EXPECT_EQ(data, nullptr);

		// Get all
		const std::vector<jactorio::data::Sprite*> data_all =
			jactorio::data::DataRawGetAll<jactorio::data::Sprite>(
				jactorio::data::DataCategory::sprite);

		EXPECT_EQ(data_all.size(), 0);
	}
}
