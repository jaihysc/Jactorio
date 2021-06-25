// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "proto/sprite.h"

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

    TEST(Sprite, TrySetDefaultSpriteGroup) {
        {
            // Item's sprite group should be set to terrain and gui if blank
            Sprite sprite;

            auto& group = sprite.group;

            sprite.DefaultSpriteGroup({Sprite::SpriteGroup::terrain, Sprite::SpriteGroup::gui});

            EXPECT_NE(std::find(group.begin(), group.end(), Sprite::SpriteGroup::terrain), group.end());
            EXPECT_NE(std::find(group.begin(), group.end(), Sprite::SpriteGroup::gui), group.end());
        }
        {
            // If not blank, use initialization provided sprite groups
            Sprite sprite;

            auto& group = sprite.group;
            group.push_back(Sprite::SpriteGroup::gui);

            EXPECT_EQ(group.size(), 1);
        }
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
} // namespace jactorio::proto
