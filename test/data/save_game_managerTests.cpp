// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/save_game_manager.h"

#include <fstream>

#include "core/loop_common.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
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
