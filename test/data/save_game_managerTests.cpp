// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/save_game_manager.h"

#include <fstream>

#include "core/loop_common.h"
#include "data/prototype/container_entity.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
    TEST(SaveGameManager, PrepareWorldDataClear) {
        game::GameDataGlobal global;
        game::GameDataLocal local;

        global.worlds.emplace_back();
        global.worlds[0].EmplaceChunk(0, 0);


        // 1 Should not attempt to remove cursor overlays
        const Sprite sprite;
        const ContainerEntity container;
        local.input.mouse.DrawOverlay(global.worlds[0], {0, 0}, Orientation::up, &container, sprite);

        // 2 Should not hold pointer to any tile layer (as they will be destroyed)
        game::ChunkTileLayer tile_layer;
        global.player.placement.SetActivatedLayer(&tile_layer);


        PrepareWorldDataClear(local, global);


        // 1
        local.input.mouse.DrawOverlay(global.worlds[0], {0, 1}, Orientation::up, &container, sprite);
        EXPECT_EQ(global.worlds[0].GetChunkC(0, 0)->GetOverlay(game::OverlayLayer::cursor).size(), 2);

        // 2
        EXPECT_EQ(global.player.placement.GetActivatedLayer(), nullptr);
    }

    TEST(SaveGameManager, IsValidSaveName) {
        EXPECT_TRUE(IsValidSaveName("."));
        EXPECT_TRUE(IsValidSaveName("im"));
        EXPECT_TRUE(IsValidSaveName("just-running"));
        EXPECT_TRUE(IsValidSaveName("inthe90s"));
        EXPECT_TRUE(IsValidSaveName("do what you want with me"));
        EXPECT_TRUE(IsValidSaveName("im!@#$just!@#$running(*&^in@#the!!90s~"));
        EXPECT_TRUE(IsValidSaveName("\321\321321\3\213\12\321\3\213\21"));
        EXPECT_TRUE(IsValidSaveName(".running in the 90s!"));

        EXPECT_FALSE(IsValidSaveName(""));
        EXPECT_FALSE(IsValidSaveName("what/a/great/time/it/is/to/be"));
        EXPECT_FALSE(IsValidSaveName("im.just.running.in.the.90s"));
        EXPECT_FALSE(IsValidSaveName("RUNNING\\IN\\The\\90s"));
        EXPECT_FALSE(IsValidSaveName("~whatcha/gonnado"));
        EXPECT_FALSE(IsValidSaveName("~with\\me"));
        EXPECT_FALSE(IsValidSaveName("..imjust"));
        EXPECT_FALSE(IsValidSaveName(".running in the 90s."));
    }

    TEST(SaveGameManager, GetSaveDirIt) {
        try {
            const auto dir_it = GetSaveDirIt(); // Will create save folder
            std::filesystem::remove("saves");
        }
        catch (std::filesystem::filesystem_error&) {
            GTEST_FAIL();
        }
    }

    TEST(SaveGameManager, ResolveSavePath) {
        const auto path = ResolveSavePath("lovely_save_name"); // Will create save folder
        EXPECT_EQ(path, "saves/lovely_save_name.dat");

        try {
            auto x = std::ofstream(path);
            EXPECT_TRUE(x.is_open());
            x << "asdfjk;asdfjk;l";
        }
        catch (std::ios_base::failure&) {
            GTEST_FAIL();
        }

        std::filesystem::remove_all("saves");
    }
} // namespace jactorio::data
