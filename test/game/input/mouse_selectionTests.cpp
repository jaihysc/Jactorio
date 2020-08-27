// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype_manager.h"
#include "data/prototype/container_entity.h"
#include "data/prototype/mining_drill.h"
#include "game/input/mouse_selection.h"
#include "game/player/player_data.h"

namespace jactorio::game
{
	class MouseSelectionOverlayTest : public testing::Test
	{
		bool setupCursor_ = false;

	protected:
		WorldData worldData_{};
		LogicData logicData_{};

		PlayerData playerData_{};
		MouseSelection mouseSelection_{};

		data::PrototypeManager dataManager_{};

		// Call setup_mouse_cursor before using
		data::Sprite* cursorSprite_ = nullptr;


		data::Sprite entitySprite_{};
		data::ContainerEntity entity_{};

		void SetUp() override {
			playerData_.SetPlayerWorldData(worldData_);
			playerData_.SetPlayerLogicData(logicData_);
			worldData_.EmplaceChunk(0, 0);

			entity_.sprite    = &entitySprite_;
			entity_.placeable = true;
		}

		///
		/// \brief If the test involves the selection cursor, which is loaded in the data_manager
		void SetupMouseCursor() {
			setupCursor_  = true;
			cursorSprite_ = &dataManager_.AddProto<data::Sprite>("__core__/cursor-select");
		}
	};

	TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedPlaceable) {
		// An item is selected
		// Clear last and draw new

		auto& overlay_layer = worldData_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::general);

		// Should set item's sprite at overlay layer at world position 0, 0
		mouseSelection_.DrawOverlay(playerData_, dataManager_, &entity_, 0, 0, data::Orientation::up);

		ASSERT_EQ(overlay_layer.size(), 1);
		EXPECT_EQ(overlay_layer[0].sprite, &entitySprite_);
		EXPECT_EQ(overlay_layer[0].position.x, 0);
		EXPECT_EQ(overlay_layer[0].position.y, 0);


		// Should clear last overlay at 0,0 Draw new at 1, 0
		mouseSelection_.DrawOverlay(playerData_, dataManager_, &entity_, 1, 0, data::Orientation::up);
		ASSERT_EQ(overlay_layer.size(), 1);
		EXPECT_EQ(overlay_layer[0].sprite, &entitySprite_);
		EXPECT_EQ(overlay_layer[0].position.x, 1);
		EXPECT_EQ(overlay_layer[0].position.y, 0);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedNonPlaceable) {
		// A non placeable item is selected
		// Clear last and draw nothing new

		entity_.placeable = false;

		auto& overlay_layer = worldData_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::general);

		// Should NOT set item's sprite at overlay layer at world position 0, 0 since the entity selected is not placeable
		mouseSelection_.DrawOverlay(playerData_, dataManager_, &entity_, 0, 0, data::Orientation::up);
		EXPECT_EQ(overlay_layer.size(), 0);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedNonPlaceableHoverEntity) {
		// A non placeable item is selected over an entity
		// Clear last and draw cursor, draw cursor over entity, no preview ghost

		entity_.placeable = false;

		SetupMouseCursor();

		worldData_.GetTile(0, 0)->GetLayer(TileLayer::entity).prototypeData = &entity_;


		auto& overlay_layer = worldData_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::general);

		// Should NOT set item's sprite at overlay layer at world position 0, 0 since the entity selected is not placeable
		mouseSelection_.DrawOverlay(playerData_, dataManager_, &entity_, 0, 0, data::Orientation::up);

		ASSERT_EQ(overlay_layer.size(), 1);
		EXPECT_EQ(overlay_layer[0].sprite, cursorSprite_);

		// Remove overlay at 0, 0
		mouseSelection_.DrawOverlay(playerData_, dataManager_, nullptr, 1, 0, data::Orientation::up);
		EXPECT_EQ(overlay_layer.size(), 0);

		// Nothing happens as already removed
		mouseSelection_.DrawOverlay(playerData_, dataManager_, nullptr, 1, 0, data::Orientation::up);
		EXPECT_EQ(overlay_layer.size(), 0);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelected) {
		// NO item is selected
		// Clear last and draw nothing new

		auto& overlay_layer = worldData_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::general);

		mouseSelection_.DrawOverlay(playerData_, dataManager_, nullptr, 0, 0, data::Orientation::up);
		EXPECT_EQ(overlay_layer.size(), 0);

		mouseSelection_.DrawOverlay(playerData_, dataManager_, nullptr, 1, 0, data::Orientation::up);
		EXPECT_EQ(overlay_layer.size(), 0);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelectedEntityHover) {
		// NO item is selected, but cursor is hovered over an entity
		// Draw selection cursor

		SetupMouseCursor();

		worldData_.GetTile(0, 0)
		          ->GetLayer(TileLayer::entity).prototypeData = &entity_;

		auto& overlay_layer = worldData_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::general);

		mouseSelection_.DrawOverlay(playerData_, dataManager_, nullptr, 0, 0, data::Orientation::up);
		ASSERT_EQ(overlay_layer.size(), 1);
		EXPECT_EQ(overlay_layer[0].sprite, cursorSprite_);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelectedResourceEntityHover) {
		// NO item is selected, but cursor is hovered over a resource entity
		// Draw selection cursor

		SetupMouseCursor();

		worldData_.GetTile(0, 0)
		          ->GetLayer(TileLayer::resource).prototypeData = &entity_;

		auto& overlay_layer = worldData_.GetChunkC(0, 0)->GetOverlay(OverlayLayer::general);

		mouseSelection_.DrawOverlay(playerData_, dataManager_, nullptr, 0, 0, data::Orientation::up);
		ASSERT_EQ(overlay_layer.size(), 1);
		EXPECT_EQ(overlay_layer[0].sprite, cursorSprite_);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNoWorld) {
		// When selecting no world / chunk not generated
		// Attempt to clear last tile and do nothing

		WorldData world_data{};
		PlayerData player_data{};
		player_data.SetPlayerWorldData(world_data);

		MouseSelection mouse_selection{};

		mouse_selection.DrawOverlay(player_data, dataManager_, nullptr, 0, 0, data::Orientation::up);
	}

	class MockEntity final : public data::Entity
	{
	public:
		PROTOTYPE_CATEGORY(test);

		mutable bool getSpriteCalled    = false;
		mutable bool getSpriteSetCalled = false;


		J_NODISCARD data::Sprite* OnRGetSprite(data::Sprite::SetT) const override {
			getSpriteCalled = true;
			return nullptr;
		}

		J_NODISCARD data::Sprite::SetT OnRGetSpriteSet(data::Orientation,
		                                               WorldData&,
		                                               const WorldCoord&) const override {
			getSpriteSetCalled = true;
			return 16;
		}

		void OnBuild(WorldData&,
		             LogicData&,
		             const WorldCoord&,
		             ChunkTileLayer&, data::Orientation) const override {
		}

		void OnRemove(WorldData&,
		              LogicData&,
		              const WorldCoord&, ChunkTileLayer&) const override {
		}
	};

	TEST_F(MouseSelectionOverlayTest, DrawOverlayCallGetSprite) {

		MockEntity entity{};
		entity.rotatable = true;
		entity.placeable = true;

		mouseSelection_.DrawOverlay(playerData_, dataManager_, &entity, 0, 0, data::Orientation::up);

		EXPECT_TRUE(entity.getSpriteCalled);
		EXPECT_TRUE(entity.getSpriteSetCalled);
	}
}
