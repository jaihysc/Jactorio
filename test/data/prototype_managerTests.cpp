// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include <filesystem>

#include "data/prototype_manager.h"

#include "data/globals.h"
#include "proto/label.h"
#include "proto/sprite.h"

namespace jactorio::data
{
    class PrototypeManagerTest : public testing::Test
    {
    protected:
        PrototypeManager proto_;

        /// Returns true if element exists in vector
        static bool Contains(const std::vector<proto::Sprite*>& vector, const std::string& key) {
            for (const auto& i : vector) {
                if (i->name == key) {
                    return true;
                }
            }

            return false;
        }
    };

    TEST_F(PrototypeManagerTest, Make) {
        auto& added_proto = proto_.Make<proto::Sprite>("raw-fish");

        const auto* proto = proto_.Get<proto::Sprite>("raw-fish");
        EXPECT_EQ(proto, &added_proto);
    }

    TEST_F(PrototypeManagerTest, Add) {
        proto_.SetDirectoryPrefix("test");

        proto_.Make<proto::Sprite>("raw-fish");

        const auto* sprite = proto_.Get<proto::Sprite>("__test__/raw-fish");


        // Internal name of prototype should have been renamed to match data_raw name
        // Data category should also have been set to the one provided on add
        EXPECT_EQ(sprite->name, "__test__/raw-fish");
        EXPECT_EQ(sprite->GetCategory(), proto::Category::sprite);
        EXPECT_EQ(sprite->internalId, 1);
        EXPECT_EQ(sprite->order, 1);
        // Since no localized name was specified, it uses the internal name
        EXPECT_EQ(sprite->GetLocalizedName(), "__test__/raw-fish");
    }

    TEST_F(PrototypeManagerTest, AddDirectoryPrefix) {
        proto_.SetDirectoryPrefix();

        auto& prototype = proto_.Make<proto::Sprite>("raw-fish");

        {
            const auto* proto = proto_.Get<proto::Sprite>("__has_prefix__/raw-fish");
            EXPECT_EQ(proto, nullptr);
        }
        {
            const auto* proto = proto_.Get<proto::Sprite>("raw-fish");
            EXPECT_EQ(proto, &prototype);
        }
    }

    TEST_F(PrototypeManagerTest, AddIncrementId) {
        proto_.Make<proto::Sprite>("raw-fish0");
        proto_.Make<proto::Sprite>("raw-fish1");
        proto_.Make<proto::Sprite>("raw-fish2");
        proto_.Make<proto::Sprite>("raw-fish3");

        const auto* proto = proto_.Get<proto::Sprite>("raw-fish3");

        EXPECT_EQ(proto->name, "raw-fish3");
        EXPECT_EQ(proto->GetCategory(), proto::Category::sprite);
        EXPECT_EQ(proto->internalId, 4);
    }

    TEST_F(PrototypeManagerTest, OverrideExisting) {
        proto_.SetDirectoryPrefix("test");

        // Normal name
        {
            proto_.Make<proto::Sprite>("small-electric-pole");

            // Override
            auto& prototype2 = proto_.Make<proto::Sprite>("small-electric-pole");

            // Get
            const auto* proto = proto_.Get<proto::Sprite>("__test__/small-electric-pole");

            EXPECT_EQ(proto, &prototype2);
        }

        proto_.Clear();
        // Empty name - Overriding is disabled for empty names, this is for destructor data_raw add
        // Instead, it will assign an auto generated name
        {
            proto_.Make<proto::Sprite>("");

            // No Override
            proto_.Make<proto::Sprite>("");

            // Get
            const auto sprite_protos = proto_.GetAll<proto::Sprite>();
            EXPECT_EQ(sprite_protos.size(), 2);


            const auto* proto = proto_.Get<proto::Sprite>("");

            // The empty name will be automatically assigned to something else
            EXPECT_EQ(proto, nullptr);
        }
    }


    /// This test excluded in Valgrind
    TEST_F(PrototypeManagerTest, LoadProto) {
        active_prototype_manager = &proto_;
        proto_.SetDirectoryPrefix("asdf");

        // Load_data should set the directory prefix based on the subfolder
        proto_.LoadProto(PrototypeManager::kDataFolder);

        const auto* proto = proto_.Get<proto::Sprite>("__test__/test_tile");

        if (proto == nullptr) {
            FAIL();
        }

        EXPECT_EQ(proto->name, "__test__/test_tile");

        EXPECT_EQ(proto->GetWidth(), 32);
        EXPECT_EQ(proto->GetHeight(), 32);
    }

    TEST_F(PrototypeManagerTest, LoadInvalidPath) {
        // Loading an invalid path will throw filesystem exception
        proto_.SetDirectoryPrefix("asdf");

        // Load_data should set the directory prefix based on the subfolder
        try {
            proto_.LoadProto("yeet");
            FAIL();
        }
        catch (std::filesystem::filesystem_error&) {
            SUCCEED();
        }
    }

    TEST_F(PrototypeManagerTest, LoadLocalization) {
        proto_.Make<proto::Sprite>("__test__/test_tile");

        proto_.LoadLocal(PrototypeManager::kDataFolder, "test");
        const auto* proto = proto_.Get<proto::Sprite>("__test__/test_tile");

        ASSERT_NE(proto, nullptr);
        EXPECT_EQ(proto->GetLocalizedName(), "Localized Test Tile");
    }

    TEST_F(PrototypeManagerTest, GetInvalid) {
        // Should return a nullptr if the item is non-existent
        const auto* ptr = proto_.Get<proto::FrameworkBase>(proto::Category::sprite, "asdfjsadhfkjdsafhs");

        EXPECT_EQ(ptr, nullptr);
    }


    TEST_F(PrototypeManagerTest, GetAllOfType) {
        proto_.Make<proto::Sprite>("test_tile1");
        proto_.Make<proto::Sprite>("test_tile2");

        const std::vector<proto::Sprite*> paths = proto_.GetAll<proto::Sprite>();

        EXPECT_EQ(Contains(paths, "test_tile1"), true);
        EXPECT_EQ(Contains(paths, "test_tile2"), true);

        EXPECT_EQ(Contains(paths, "asdf"), false);
    }

    TEST_F(PrototypeManagerTest, GetAllSorted) {
        // Retrieved vector should have prototypes sorted in order of addition, first one being added is first in vector
        proto_.Make<proto::Sprite>("test_tile1");
        proto_.Make<proto::Sprite>("test_tile2");
        proto_.Make<proto::Sprite>("test_tile3");
        proto_.Make<proto::Sprite>("test_tile4");

        // Get
        const std::vector<proto::Sprite*> protos = proto_.GetAllSorted<proto::Sprite>();

        EXPECT_EQ(protos[0]->name, "test_tile1");
        EXPECT_EQ(protos[1]->name, "test_tile2");
        EXPECT_EQ(protos[2]->name, "test_tile3");
        EXPECT_EQ(protos[3]->name, "test_tile4");
    }

    TEST_F(PrototypeManagerTest, PrototypeExists) {
        EXPECT_FALSE(proto_.Find("bunny"));

        proto_.Make<proto::Sprite>("aqua");
        EXPECT_FALSE(proto_.Find("bunny"));
        EXPECT_TRUE(proto_.Find("aqua"));
    }

    TEST_F(PrototypeManagerTest, Clear) {
        proto_.Make<proto::Sprite>("small-electric-pole");

        proto_.Clear();

        // Get
        auto* data = proto_.Get<proto::Sprite>("small-electric-pole");

        EXPECT_EQ(data, nullptr);

        // Get all
        const std::vector<proto::Sprite*> data_all = proto_.GetAll<proto::Sprite>();

        EXPECT_EQ(data_all.size(), 0);
    }

    TEST_F(PrototypeManagerTest, GetValidLabel) {
        auto& label = proto_.Make<proto::Label>("label-1");
        label.SetLocalizedName("April");

        EXPECT_EQ(proto_.GetLocalText("label-1"), "April");
    }
    TEST_F(PrototypeManagerTest, GetInvalidLabel) {
        EXPECT_EQ(proto_.GetLocalText("Creeper aw man"), "???");
    }


    TEST_F(PrototypeManagerTest, GenerateRelocationTable) {
        auto& sprite_1 = proto_.Make<proto::Sprite>();
        auto& sprite_2 = proto_.Make<proto::Sprite>();
        auto& sprite_3 = proto_.Make<proto::Sprite>();

        proto_.GenerateRelocationTable();

        EXPECT_EQ(&proto_.RelocationTableGet<proto::Sprite>(sprite_1.internalId), &sprite_1);
        EXPECT_EQ(&proto_.RelocationTableGet<proto::Sprite>(sprite_2.internalId), &sprite_2);
        EXPECT_EQ(&proto_.RelocationTableGet<proto::Sprite>(sprite_3.internalId), &sprite_3);
    }

    TEST_F(PrototypeManagerTest, ClearRelocationTable) {
        proto_.Make<proto::Sprite>();
        proto_.GenerateRelocationTable();

        proto_.Clear();

        auto& sprite = proto_.Make<proto::Sprite>();
        proto_.GenerateRelocationTable(); // Does not save sprite from earlier

        EXPECT_EQ(&proto_.RelocationTableGet<proto::Sprite>(1), &sprite);
        EXPECT_EQ(proto_.GetDebugInfo().relocationTable.size(), 1);
    }
} // namespace jactorio::data
