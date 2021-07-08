// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "proto/sprite.h"

#include "data/prototype_manager.h"

namespace jactorio::proto
{
    TEST(Sprite, SpriteCopy) {
        Sprite first;
        first.Load("test/graphics/test/test_tile.png");

        const auto second = first;

        EXPECT_NE(first.GetImage().buffer, second.GetImage().buffer);
    }

    TEST(Sprite, SpriteMove) {
        Sprite first;
        first.Load("test/graphics/test/test_tile.png");

        const auto second = std::move(first);

        EXPECT_EQ(first.GetImage().buffer, nullptr);
        EXPECT_NE(second.GetImage().buffer, nullptr);
    }

    TEST(Sprite, LoadSprite) {
        {
            Sprite sprite;

            EXPECT_EQ(sprite.GetImage().width, 0);
            EXPECT_EQ(sprite.GetImage().height, 0);

            sprite.Load("test/graphics/test/test_tile.png");

            EXPECT_EQ(sprite.GetImage().width, 32);
            EXPECT_EQ(sprite.GetImage().height, 32);
        }
        {
            const Sprite sprite("test/graphics/test/20x59.png");

            EXPECT_EQ(sprite.GetImage().width, 20);
            EXPECT_EQ(sprite.GetImage().height, 59);
        }
    }

    TEST(Sprite, ErrorNoSpriteGroup) {
        data::PrototypeManager proto;

        Sprite sprite1;
        sprite1.group = Sprite::SpriteGroup::terrain;
        try {
            sprite1.PostLoadValidate(proto);
            SUCCEED();
        }
        catch (std::exception&) {
            FAIL();
        }

        const Sprite sprite2;
        try {
            sprite2.PostLoadValidate(proto);
            FAIL();
        }
        catch (std::exception&) {
            SUCCEED();
        }
    }

    static void TestPixel(Sprite& sprite, const std::pair<int, int> xy, std::tuple<int, int, int, int> rgba) {
        const auto offset = (sprite.GetImage().width * xy.second + xy.first) * sprite.GetImage().bytesPerPixel;
        EXPECT_EQ(sprite.GetImage().buffer[offset + 0], std::get<0>(rgba));
        EXPECT_EQ(sprite.GetImage().buffer[offset + 1], std::get<1>(rgba));
        EXPECT_EQ(sprite.GetImage().buffer[offset + 2], std::get<2>(rgba));
        EXPECT_EQ(sprite.GetImage().buffer[offset + 3], std::get<3>(rgba));
    };

    TEST(Sprite, Trim) {
        Sprite sprite;
        sprite.Load("test/graphics/test/test_tile4.png");

        sprite.Trim(4);
        EXPECT_EQ(sprite.GetImage().width, 9);
        EXPECT_EQ(sprite.GetImage().height, 24);

        TestPixel(sprite, {0, 0}, {0, 105, 162, 255});
        TestPixel(sprite, {8, 0}, {46, 109, 62, 255});
        TestPixel(sprite, {7, 9}, {119, 85, 255, 255});
        TestPixel(sprite, {8, 23}, {153, 223, 168, 255});
    }

    TEST(Sprite, TrimMultipleSetFrame) {
        Sprite sprite;
        sprite.Load("test/graphics/test/test_tile4.png");

        sprite.sets   = 4;
        sprite.frames = 2;

        sprite.Trim(1);
        EXPECT_EQ(sprite.GetImage().width, 12);
        EXPECT_EQ(sprite.GetImage().height, 24);

        TestPixel(sprite, {0, 0}, {251, 101, 20, 255});
        TestPixel(sprite, {5, 11}, {153, 223, 219, 255});
        TestPixel(sprite, {9, 10}, {73, 28, 253, 255});
        TestPixel(sprite, {9, 15}, {0, 251, 63, 255});
        TestPixel(sprite, {3, 18}, {21, 101, 38, 255});
        TestPixel(sprite, {11, 23}, {255, 0, 123, 255});
    }

    TEST(Sprite, TrimTooLarge) {
        Sprite sprite;
        sprite.Load("test/graphics/test/test_tile.png");

        sprite.Trim(16); // Ok, No pixels left after trimming
        EXPECT_EQ(sprite.GetImage().width, 0);
        EXPECT_EQ(sprite.GetImage().height, 0);

        sprite.Trim(0);

        try {
            sprite.Trim(1);
            FAIL();
        }
        catch (std::exception&) {
            SUCCEED();
        }
    }
} // namespace jactorio::proto
