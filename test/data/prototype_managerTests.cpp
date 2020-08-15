// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include <filesystem>

#include "data/prototype_manager.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
	class DataManagerTest : public testing::Test
	{
	protected:
		PrototypeManager dataManager_{};

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

	TEST_F(DataManagerTest, AddProto) {
		auto& added_proto = dataManager_.AddProto<Sprite>("raw-fish");

		const auto* proto = dataManager_.DataRawGet<Sprite>("raw-fish");
		EXPECT_EQ(proto, &added_proto);

	}

	TEST_F(DataManagerTest, DataRawAdd) {
		dataManager_.SetDirectoryPrefix("test");

		dataManager_.AddProto<Sprite>("raw-fish");

		const auto* proto = dataManager_.DataRawGet<Sprite>("__test__/raw-fish");


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

	TEST_F(DataManagerTest, DataRawAddDirectoryPrefix) {
		dataManager_.SetDirectoryPrefix();

		auto& prototype = dataManager_.AddProto<Sprite>("raw-fish");

		{
			const auto* proto = dataManager_.DataRawGet<Sprite>("__has_prefix__/raw-fish");
			EXPECT_EQ(proto, nullptr);
		}
		{
			const auto* proto = dataManager_.DataRawGet<Sprite>("raw-fish");
			EXPECT_EQ(proto, &prototype);
		}
	}

	TEST_F(DataManagerTest, DataRawAddIncrementId) {
		dataManager_.AddProto<Sprite>("raw-fish0");
		dataManager_.AddProto<Sprite>("raw-fish1");
		dataManager_.AddProto<Sprite>("raw-fish2");
		dataManager_.AddProto<Sprite>("raw-fish3");

		const auto* proto = dataManager_.DataRawGet<Sprite>("raw-fish3");

		EXPECT_EQ(proto->name, "raw-fish3");
		EXPECT_EQ(proto->Category(), DataCategory::sprite);
		EXPECT_EQ(proto->internalId, 4);
	}

	TEST_F(DataManagerTest, DataRawOverride) {
		dataManager_.SetDirectoryPrefix("test");

		// Normal name
		{
			dataManager_.AddProto<Sprite>("small-electric-pole");

			// Override
			auto& prototype2 = dataManager_.AddProto<Sprite>("small-electric-pole");

			// Get
			const auto* proto = dataManager_.DataRawGet<Sprite>("__test__/small-electric-pole");

			EXPECT_EQ(proto, &prototype2);
		}

		dataManager_.ClearData();
		// Empty name - Overriding is disabled for empty names, this is for destructor data_raw add
		// Instead, it will assign an auto generated name
		{
			dataManager_.AddProto<Sprite>("");

			// No Override
			dataManager_.AddProto<Sprite>("");

			// Get
			const auto sprite_protos = dataManager_.DataRawGetAll<Sprite>(DataCategory::sprite);
			EXPECT_EQ(sprite_protos.size(), 2);


			const auto* proto = dataManager_.DataRawGet<Sprite>("");

			// The empty name will be automatically assigned to something else
			EXPECT_EQ(proto, nullptr);
		}

	}


	TEST_F(DataManagerTest, LoadData) {
		active_data_manager = &dataManager_;
		dataManager_.SetDirectoryPrefix("asdf");

		// Load_data should set the directory prefix based on the subfolder
		dataManager_.LoadData("data");

		const auto* proto = dataManager_.DataRawGet<Sprite>("__test__/test_tile");

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
			dataManager_.DataRawGet<FrameworkBase>(DataCategory::sprite, "asdfjsadhfkjdsafhs");

		EXPECT_EQ(ptr, nullptr);
	}


	TEST_F(DataManagerTest, GetAllDataOfType) {
		dataManager_.AddProto<Sprite>("test_tile1");
		dataManager_.AddProto<Sprite>("test_tile2");

		const std::vector<Sprite*> paths = dataManager_.DataRawGetAll<Sprite>(
			DataCategory::sprite);

		EXPECT_EQ(Contains(paths, "test_tile1"), true);
		EXPECT_EQ(Contains(paths, "test_tile2"), true);

		EXPECT_EQ(Contains(paths, "asdf"), false);
	}

	TEST_F(DataManagerTest, GetAllSorted) {
		// Retrieved vector should have prototypes sorted in order of addition, first one being added is first in vector
		dataManager_.AddProto<Sprite>("test_tile1");
		dataManager_.AddProto<Sprite>("test_tile2");
		dataManager_.AddProto<Sprite>("test_tile3");
		dataManager_.AddProto<Sprite>("test_tile4");

		// Get
		const std::vector<Sprite*> protos =
			dataManager_.DataRawGetAllSorted<Sprite>(DataCategory::sprite);

		EXPECT_EQ(protos[0]->name, "test_tile1");
		EXPECT_EQ(protos[1]->name, "test_tile2");
		EXPECT_EQ(protos[2]->name, "test_tile3");
		EXPECT_EQ(protos[3]->name, "test_tile4");
	}

	TEST_F(DataManagerTest, PrototypeExists) {
		EXPECT_FALSE(dataManager_.FindProto("bunny"));

		dataManager_.AddProto<Sprite>("aqua");
		EXPECT_FALSE(dataManager_.FindProto("bunny"));
		EXPECT_TRUE(dataManager_.FindProto("aqua"));
	}

	TEST_F(DataManagerTest, ClearData) {
		dataManager_.AddProto<Sprite>("small-electric-pole");

		dataManager_.ClearData();

		// Get
		auto* data = dataManager_.DataRawGet<Sprite>("small-electric-pole");

		EXPECT_EQ(data, nullptr);

		// Get all
		const std::vector<Sprite*> data_all = dataManager_.DataRawGetAll<Sprite>(DataCategory::sprite);

		EXPECT_EQ(data_all.size(), 0);
	}


	TEST_F(DataManagerTest, GenerateRelocationTable) {
		auto& sprite_1 = dataManager_.AddProto<Sprite>();
		auto& sprite_2 = dataManager_.AddProto<Sprite>();
		auto& sprite_3 = dataManager_.AddProto<Sprite>();

		dataManager_.GenerateRelocationTable();

		EXPECT_EQ(&dataManager_.RelocationTableGet<Sprite>(0), &sprite_1);
		EXPECT_EQ(&dataManager_.RelocationTableGet<Sprite>(1), &sprite_2);
		EXPECT_EQ(&dataManager_.RelocationTableGet<Sprite>(2), &sprite_3);
	}

	TEST_F(DataManagerTest, ClearRelocationTable) {
		dataManager_.AddProto<Sprite>();
		dataManager_.GenerateRelocationTable();

		dataManager_.ClearData();

		auto& sprite = dataManager_.AddProto<Sprite>();
		dataManager_.GenerateRelocationTable();  // Does not save sprite from earlier

		EXPECT_EQ(&dataManager_.RelocationTableGet<Sprite>(0), &sprite);
		EXPECT_EQ(dataManager_.GetDebugInfo().relocationTable.size(), 1);
	}
}
