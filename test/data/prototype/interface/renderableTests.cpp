// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype/interface/renderable.h"

namespace jactorio::data
{
    class IRenderableTest : public testing::Test, public IRenderable
    {
    protected:
        Sprite sprite_{};

        void SetUp() override {
            sprite_.sets   = 5;
            sprite_.frames = 12;
        }


    public:
        J_NODISCARD Sprite* OnRGetSprite(Sprite::SetT) const override {
            return nullptr;
        }

        J_NODISCARD Sprite::SetT OnRGetSpriteSet(Orientation, game::WorldData&, const WorldCoord&) const override {
            return 0;
        }

        J_NODISCARD Sprite::FrameT OnRGetSpriteFrame(const UniqueDataBase&, GameTickT) const override {
            return 0;
        }

        bool OnRShowGui(const render::GuiRenderer& /*g_rendr*/, game::ChunkTileLayer* /*tile_layer*/) const override {
            return true;
        }
    };

    TEST_F(IRenderableTest, AllOfSprite) {
        EXPECT_EQ(AllOfSprite(sprite_, 0), 0);
        EXPECT_EQ(AllOfSprite(sprite_, 59), 59);
        EXPECT_EQ(AllOfSprite(sprite_, 60), 0);


        // Half speed
        EXPECT_EQ(AllOfSprite(sprite_, 0, 1.f / 2), 0);
        EXPECT_EQ(AllOfSprite(sprite_, 1, 1.f / 2), 0);
        EXPECT_EQ(AllOfSprite(sprite_, 60, 1.f / 2), 30);
        EXPECT_EQ(AllOfSprite(sprite_, 61, 1.f / 2), 30);
    }

    TEST_F(IRenderableTest, AllOfSpriteReversible) {
        EXPECT_EQ(AllOfSpriteReversing(sprite_, 0), 0);

        EXPECT_EQ(AllOfSpriteReversing(sprite_, 58), 58);
        EXPECT_EQ(AllOfSpriteReversing(sprite_, 59), 59);
        EXPECT_EQ(AllOfSpriteReversing(sprite_, 60), 58);

        EXPECT_EQ(AllOfSpriteReversing(sprite_, 118), 0);
        EXPECT_EQ(AllOfSpriteReversing(sprite_, 119), 1);


        // Half speed
        // Frames are not always distributed equally at speed < 1 < because of floating point truncation

        EXPECT_EQ(AllOfSpriteReversing(sprite_, 0, 1.f / 2), 0);
        EXPECT_EQ(AllOfSpriteReversing(sprite_, 1, 1.f / 2), 1);
        EXPECT_EQ(AllOfSpriteReversing(sprite_, 60, 1.f / 2), 30);
        EXPECT_EQ(AllOfSpriteReversing(sprite_, 61, 1.f / 2), 31);
    }

    TEST_F(IRenderableTest, AllOfSet) {
        EXPECT_EQ(AllOfSet(sprite_, 0), 0);
        EXPECT_EQ(AllOfSet(sprite_, 11), 11);
        EXPECT_EQ(AllOfSet(sprite_, 12), 0);
        EXPECT_EQ(AllOfSet(sprite_, 13), 1);

        // Half speed
        EXPECT_EQ(AllOfSet(sprite_, 0, 1.f / 2), 0);
        EXPECT_EQ(AllOfSet(sprite_, 10, 1.f / 2), 5);
        EXPECT_EQ(AllOfSet(sprite_, 11, 1.f / 2), 5);
        EXPECT_EQ(AllOfSet(sprite_, 22, 1.f / 2), 11);
        EXPECT_EQ(AllOfSet(sprite_, 23, 1.f / 2), 11);
        EXPECT_EQ(AllOfSet(sprite_, 24, 1.f / 2), 0);
    }
} // namespace jactorio::data
