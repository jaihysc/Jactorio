// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include <gtest/gtest.h>

#include <filesystem>

#include "data/data_manager.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
	class DataManagerTest : public testing::Test
	{
	protected:
		DataManager dataManager_{};

		/// \brief Returns true if element exists in vector
		static bool Contains(const std::vector<Sprite*>& vector, const std::string& key) {
			for (const auto& i : vector) {
				if (i->name == key) {
					return true;
				}
			}

			return false;
		}
	};

	TEST_F(DataManagerTest, DataRawAdd) {
		dataManager_.SetDirectoryPrefix("test");

		dataManager_.DataRawAdd("raw-fish", new Sprite{}, true);

		const auto* proto =
			dataManager_.DataRawGet<Sprite>(
				DataCategory::sprite,
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
		dataManager_.SetDirectoryPrefix("this_should_not_exist");

		auto* prototype = new Sprite{};
		dataManager_.DataRawAdd("raw-fish", prototype, false);

		// Prefix __this_should_not_exist/ should not be added
		{
			const auto* proto =
				dataManager_.DataRawGet<PrototypeBase>(
					DataCategory::sprite,
					"__this_should_not_exist__/raw-fish");
			EXPECT_EQ(proto, nullptr);
		}
		{
			const auto* proto =
				dataManager_.DataRawGet<PrototypeBase>(
					DataCategory::sprite,
					"raw-fish");
			EXPECT_EQ(proto, prototype);
		}
	}

	TEST_F(DataManagerTest, DataRawAddIncrementId) {
		dataManager_.DataRawAdd("raw-fish0", new Sprite{});
		dataManager_.DataRawAdd("raw-fish1", new Sprite{});
		dataManager_.DataRawAdd("raw-fish2", new Sprite{});
		dataManager_.DataRawAdd("raw-fish3", new Sprite{});

		const auto* proto =
			dataManager_.DataRawGet<Sprite>(
				DataCategory::sprite,
				"raw-fish3");

		EXPECT_EQ(proto->name, "raw-fish3");
		EXPECT_EQ(proto->Category(), DataCategory::sprite);
		EXPECT_EQ(proto->internalId, 4);
	}

	TEST_F(DataManagerTest, DataRawOverride) {
		dataManager_.SetDirectoryPrefix("test");

		// Normal name
		{
			auto* prototype = new Sprite{};
			dataManager_.DataRawAdd("small-electric-pole", prototype, true);

			// Override
			auto* prototype2 = new Sprite{};
			dataManager_.DataRawAdd("small-electric-pole", prototype2, true);

			// Get
			const auto* proto = dataManager_.DataRawGet<Sprite>(DataCategory::sprite, "__test__/small-electric-pole");

			EXPECT_EQ(proto, prototype2);
		}

		dataManager_.ClearData();
		// Empty name - Overriding is disabled for empty names, this is for destructor data_raw add
		// Instead, it will assign an auto generated name
		{
			auto* prototype = new Sprite{};
			dataManager_.DataRawAdd("", prototype, true);

			// No Override
			auto* prototype2 = new Sprite{};
			dataManager_.DataRawAdd("", prototype2, true);

			// Get
			const auto sprite_protos = dataManager_.DataRawGetAll<Sprite>(DataCategory::sprite);
			EXPECT_EQ(sprite_protos.size(), 2);


			const auto* proto = dataManager_.DataRawGet<Sprite>(DataCategory::sprite, "");

			// The empty name will be automatically assigned to something else
			EXPECT_EQ(proto, nullptr);
		}

	}


	TEST_F(DataManagerTest, LoadData) {
		active_data_manager = &dataManager_;
		dataManager_.SetDirectoryPrefix("asdf");

		// Load_data should set the directory prefix based on the subfolder
		dataManager_.LoadData("data");

		const auto* proto = dataManager_.DataRawGet<Sprite>(DataCategory::sprite, "__test__/test_tile");

		if (proto == nullptr) {
			FAIL();
		}

		EXPECT_EQ(proto->name, "__test__/test_tile");

		EXPECT_EQ(proto->GetWidth(), 32);
		EXPECT_EQ(proto->GetHeight(), 32);
	}

	TEST_F(DataManagerTest, LoadDataInvalidPath) {
		// Loading an invalid path will throw filesystem exception
		dataManager_.SetDirectoryPrefix("asdf");

		// Load_data should set the directory prefix based on the subfolder
		try {
			dataManager_.LoadData("yeet");
			FAIL();
		}
		catch (std::filesystem::filesystem_error&) {
			SUCCEED();
		}
	}

	TEST_F(DataManagerTest, DataRawGetInvalid) {
		// Should return a nullptr if the item is non-existent
		const auto* ptr =
			dataManager_.DataRawGet<PrototypeBase>(DataCategory::sprite, "asdfjsadhfkjdsafhs");

		EXPECT_EQ(ptr, nullptr);
	}


	TEST_F(DataManagerTest, GetAllDataOfType) {
		dataManager_.DataRawAdd("test_tile1", new Sprite{});
		dataManager_.DataRawAdd("test_tile2", new Sprite{});

		const std::vector<Sprite*> paths = dataManager_.DataRawGetAll<Sprite>(
			DataCategory::sprite);

		EXPECT_EQ(Contains(paths, "test_tile1"), true);
		EXPECT_EQ(Contains(paths, "test_tile2"), true);

		EXPECT_EQ(Contains(paths, "asdf"), false);
	}

	TEST_F(DataManagerTest, GetAllSorted) {
		// Retrieved vector should have prototypes sorted in order of addition, first one being added is first in vector
		dataManager_.DataRawAdd("test_tile1", new Sprite{});
		dataManager_.DataRawAdd("test_tile2", new Sprite{});
		dataManager_.DataRawAdd("test_tile3", new Sprite{});
		dataManager_.DataRawAdd("test_tile4", new Sprite{});

		// Get
		const std::vector<Sprite*> protos =
			dataManager_.DataRawGetAllSorted<Sprite>(DataCategory::sprite);

		EXPECT_EQ(protos[0]->name, "test_tile1");
		EXPECT_EQ(protos[1]->name, "test_tile2");
		EXPECT_EQ(protos[2]->name, "test_tile3");
		EXPECT_EQ(protos[3]->name, "test_tile4");
	}

	TEST_F(DataManagerTest, PrototypeExists) {
		EXPECT_FALSE(dataManager_.PrototypeExists("bunny"));
		
		dataManager_.DataRawAdd("aqua", new Sprite{});
		EXPECT_FALSE(dataManager_.PrototypeExists("bunny"));
		EXPECT_TRUE(dataManager_.PrototypeExists("aqua"));
	}

	TEST_F(DataManagerTest, ClearData) {
		dataManager_.DataRawAdd("small-electric-pole", new Sprite{});

		dataManager_.ClearData();

		// Get
		auto* data = dataManager_.DataRawGet<Sprite>(DataCategory::sprite, "small-electric-pole");

		EXPECT_EQ(data, nullptr);

		// Get all
		const std::vector<Sprite*> data_all = dataManager_.DataRawGetAll<Sprite>(DataCategory::sprite);

		EXPECT_EQ(data_all.size(), 0);
	}
}
