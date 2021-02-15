// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/input/mouse_selection.h"

#include "jactorioTests.h"

#include "game/player/player.h"

namespace jactorio::game
{
    class MouseSelectionOverlayTest : public testing::Test
    {
        GameWorlds gameWorlds_{1};

    protected:
        World& world_ = gameWorlds_[0];
        Logic logic_;

        Player player_;
        MouseSelection mouseSelection_;


        proto::Sprite entitySprite_;
        proto::ContainerEntity entity_;

        void SetUp() override {
            player_.world.SetId(0);

            entity_.sprite    = &entitySprite_;
            entity_.placeable = true;
        }
    };

    TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedPlaceable) {
        // An item is selected
        // Clear last and draw new

        world_.EmplaceChunk(0, 0);

        // Should set item's sprite at overlay layer at world position 0, 0
        const proto::Sprite cursor;
        mouseSelection_.DrawOverlay(world_, {0, 0}, Orientation::up, &entity_, cursor);


        auto& overlay_layer = world_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::cursor);
        ASSERT_EQ(overlay_layer.size(), 1);
        EXPECT_EQ(overlay_layer[0].sprite, &entitySprite_);
        EXPECT_EQ(overlay_layer[0].position.x, 0);
        EXPECT_EQ(overlay_layer[0].position.y, 0);


        // Should clear last overlay at 0,0 Draw new at 1, 0
        mouseSelection_.DrawOverlay(world_, {1, 0}, Orientation::up, &entity_, cursor);
        ASSERT_EQ(overlay_layer.size(), 1);
        EXPECT_EQ(overlay_layer[0].sprite, &entitySprite_);
        EXPECT_EQ(overlay_layer[0].position.x, 1);
        EXPECT_EQ(overlay_layer[0].position.y, 0);
    }

    TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedNonPlaceable) {
        // A non placeable item is selected
        // Clear last and draw nothing new

        world_.EmplaceChunk(0, 0);
        entity_.placeable = false;


        // Should NOT set item's sprite at overlay layer at world position 0, 0 since the entity selected is not
        // placeable
        const proto::Sprite cursor;
        mouseSelection_.DrawOverlay(world_, {0, 0}, Orientation::up, &entity_, cursor);


        auto& overlay_layer = world_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::cursor);
        EXPECT_TRUE(overlay_layer.empty());
    }

    TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedNonPlaceableHoverEntity) {
        // A non placeable item is selected over an entity
        // Clear last and draw cursor, draw cursor over entity, no preview ghost

        world_.EmplaceChunk(0, 0);
        entity_.placeable = false;


        // Should draw cursor since hovering over entity and selected entity is not placeable
        world_.GetTile(0, 0)->Entity().SetPrototype(Orientation::up, &entity_);

        proto::Sprite cursor;
        mouseSelection_.DrawOverlay(world_, {0, 0}, Orientation::up, &entity_, cursor);

        auto& overlay_layer = world_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::cursor);
        ASSERT_EQ(overlay_layer.size(), 1);
        EXPECT_EQ(overlay_layer[0].sprite, &cursor);


        // Remove overlay at 0, 0
        mouseSelection_.DrawOverlay(world_, {1, 0}, Orientation::up, nullptr, cursor);
        EXPECT_TRUE(overlay_layer.empty());


        // Nothing happens as already removed
        mouseSelection_.DrawOverlay(world_, {1, 0}, Orientation::up, nullptr, cursor);
        EXPECT_TRUE(overlay_layer.empty());
    }

    TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelected) {
        // NO item is selected
        // Clear last and draw nothing new

        world_.EmplaceChunk(0, 0);

        const proto::Sprite cursor;
        mouseSelection_.DrawOverlay(world_, {0, 0}, Orientation::up, nullptr, cursor);

        auto& overlay_layer = world_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::cursor);
        EXPECT_TRUE(overlay_layer.empty());

        mouseSelection_.DrawOverlay(world_, {1, 0}, Orientation::up, nullptr, cursor);
        EXPECT_TRUE(overlay_layer.empty());
    }

    TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelectedEntityHover) {
        // NO item is selected, but cursor is hovered over an entity
        // Draw selection cursor

        world_.EmplaceChunk(0, 0);

        world_.GetTile(0, 0)->Entity().SetPrototype(Orientation::up, &entity_);
        proto::Sprite cursor;
        mouseSelection_.DrawOverlay(world_, {0, 0}, Orientation::up, nullptr, cursor);

        auto& overlay_layer = world_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::cursor);
        ASSERT_EQ(overlay_layer.size(), 1);
        EXPECT_EQ(overlay_layer[0].sprite, &cursor);
    }

    TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelectedResourceEntityHover) {
        // NO item is selected, but cursor is hovered over a resource entity
        // Draw selection cursor

        world_.EmplaceChunk(0, 0);

        world_.GetTile(0, 0)->Resource().SetPrototype(Orientation::up, &entity_);
        proto::Sprite cursor;
        mouseSelection_.DrawOverlay(world_, {0, 0}, Orientation::up, nullptr, cursor);

        auto& overlay_layer = world_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::cursor);
        ASSERT_EQ(overlay_layer.size(), 1);
        EXPECT_EQ(overlay_layer[0].sprite, &cursor);
    }

    TEST_F(MouseSelectionOverlayTest, DrawOverlayNoWorld) {
        // When selecting no world / chunk not generated
        // Attempt to clear last tile and do nothing

        const proto::Sprite cursor;
        mouseSelection_.DrawOverlay(world_, {0, 0}, Orientation::up, nullptr, cursor);
    }


    TEST_F(MouseSelectionOverlayTest, DrawOverlayCallGetSprite) {

        class MockEntity final : public TestMockEntity
        {
        public:
            J_NODISCARD proto::Sprite* OnRGetSprite(SpriteSetT /*set*/) const override {
                getSpriteCalled = true;
                return nullptr;
            }

            J_NODISCARD SpriteSetT OnRGetSpriteSet(Orientation /*orientation*/,
                                                   World& /*world*/,
                                                   const WorldCoord& /*coord*/) const override {
                getSpriteSetCalled = true;
                return 16;
            }

            mutable bool getSpriteCalled    = false;
            mutable bool getSpriteSetCalled = false;
        };

        world_.EmplaceChunk(0, 0);

        MockEntity entity;
        entity.rotatable = true;
        entity.placeable = true;

        const proto::Sprite cursor;
        mouseSelection_.DrawOverlay(world_, {0, 0}, Orientation::up, &entity, cursor);

        EXPECT_TRUE(entity.getSpriteCalled);
        EXPECT_TRUE(entity.getSpriteSetCalled);
    }

    TEST_F(MouseSelectionOverlayTest, SkipErasingLastOverlay) {
        world_.EmplaceChunk(0, 0);

        world_.GetTile(0, 0)->Resource().SetPrototype(Orientation::up, &entity_);
        const proto::Sprite cursor;
        mouseSelection_.DrawOverlay(world_, {0, 0}, Orientation::up, nullptr, cursor);

        world_.GetTile(1, 0)->Resource().SetPrototype(Orientation::up, &entity_);
        mouseSelection_.SkipErasingLastOverlay();
        mouseSelection_.DrawOverlay(world_, {1, 0}, Orientation::up, nullptr, cursor);

        EXPECT_EQ(world_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::cursor).size(), 2);
    }
} // namespace jactorio::game
