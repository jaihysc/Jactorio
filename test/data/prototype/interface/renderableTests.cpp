// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 07/06/2020

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
		SpritemapFrame OnRGetSprite(const UniqueDataBase*, GameTickT) const override {
			return {nullptr, 0};
		}

		bool OnRShowGui(game::PlayerData&, const PrototypeManager&, game::ChunkTileLayer*) const override {
			return true;
		}
	};

	TEST_F(IRenderableTest, AllOfSprite) {
		EXPECT_EQ(AllOfSprite(sprite_, 0).second, 0);
		EXPECT_EQ(AllOfSprite(sprite_, 59).second, 59);
		EXPECT_EQ(AllOfSprite(sprite_, 60).second, 0);

		
		// Half speed
		EXPECT_EQ(AllOfSprite(sprite_, 0,  1.f/2).second, 0);
		EXPECT_EQ(AllOfSprite(sprite_, 1,  1.f/2).second, 0);
		EXPECT_EQ(AllOfSprite(sprite_, 60, 1.f/2).second, 30);
		EXPECT_EQ(AllOfSprite(sprite_, 61, 1.f/2).second, 30);
	}

	TEST_F(IRenderableTest, AllOfSpriteReversible) {
		EXPECT_EQ(AllOfSpriteReversing(sprite_, 0).second, 0);

		EXPECT_EQ(AllOfSpriteReversing(sprite_, 58).second, 58);
		EXPECT_EQ(AllOfSpriteReversing(sprite_, 59).second, 59);
		EXPECT_EQ(AllOfSpriteReversing(sprite_, 60).second, 58);

		EXPECT_EQ(AllOfSpriteReversing(sprite_, 118).second, 0);
		EXPECT_EQ(AllOfSpriteReversing(sprite_, 119).second, 1);

		
		// Half speed
		// Frames are not always distributed equally at speed < 1 < because of floating point truncation

		EXPECT_EQ(AllOfSpriteReversing(sprite_, 0,  1.f/2).second, 0);
		EXPECT_EQ(AllOfSpriteReversing(sprite_, 1,  1.f/2).second, 1);
		EXPECT_EQ(AllOfSpriteReversing(sprite_, 60, 1.f/2).second, 30);
		EXPECT_EQ(AllOfSpriteReversing(sprite_, 61, 1.f/2).second, 31);
	}

	TEST_F(IRenderableTest, AllOfSet) {
		EXPECT_EQ(AllOfSet(sprite_, 0).second, 0);
		EXPECT_EQ(AllOfSet(sprite_, 11).second, 11);
		EXPECT_EQ(AllOfSet(sprite_, 12).second, 0);
		EXPECT_EQ(AllOfSet(sprite_, 13).second, 1);
		
		// Half speed
		EXPECT_EQ(AllOfSet(sprite_, 0,  1.f/2).second, 0);
		EXPECT_EQ(AllOfSet(sprite_, 10, 1.f/2).second, 5);
		EXPECT_EQ(AllOfSet(sprite_, 11, 1.f/2).second, 5);
		EXPECT_EQ(AllOfSet(sprite_, 22, 1.f/2).second, 11);
		EXPECT_EQ(AllOfSet(sprite_, 23, 1.f/2).second, 11);
		EXPECT_EQ(AllOfSet(sprite_, 24, 1.f/2).second, 0);
	}
}
