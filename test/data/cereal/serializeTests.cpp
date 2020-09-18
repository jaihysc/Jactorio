// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/cereal/serialize.h"

#include <fstream>

namespace jactorio::data
{
    TEST(Serialize, GetSaveDirIt) {
        try {
            const auto dir_it = GetSaveDirIt(); // Will create save folder
            std::filesystem::remove("saves");
        }
        catch (std::filesystem::filesystem_error&) {
            GTEST_FAIL();
        }
    }

    TEST(Serialize, ResolveSavePath) {
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
