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
} // namespace jactorio::proto
