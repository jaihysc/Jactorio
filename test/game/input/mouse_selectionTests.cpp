// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/mining_drill.h"
#include "game/input/mouse_selection.h"

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


		void SetUp() override {
			playerData_.SetPlayerWorldData(worldData_);
			playerData_.SetPlayerLogicData(logicData_);
			worldData_.AddChunk(Chunk(0, 0));
		}

		///
		/// \brief If the test involves the selection cursor, which is loaded in the data_manager
		void SetupMouseCursor() {
			setupCursor_  = true;
			cursorSprite_ = new data::Sprite{};
			dataManager_.DataRawAdd("__core__/cursor-select", cursorSprite_);
		}
	};

	TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedPlaceable) {
		// An item is selected
		// Clear last and draw new
		data::Sprite entity_sprite{};
		data::ContainerEntity entity{};
		entity.sprite    = &entity_sprite;
		entity.placeable = true;

		// Should set item's sprite at overlay layer at world position 0, 0
		mouseSelection_.DrawOverlay(playerData_, dataManager_, &entity, 0, 0, data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay).
			prototypeData,
			&entity_sprite
		);

		// Should clear last overlay at 0,0 Draw new at 1, 0
		mouseSelection_.DrawOverlay(playerData_, dataManager_, &entity, 1, 0, data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay).
			prototypeData,
			nullptr
		);
		EXPECT_EQ(
			worldData_.GetTile(1, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay).
			prototypeData,
			&entity_sprite
		);

	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedNonPlaceable) {
		// A non placeable item is selected
		// Clear last and draw nothing new

		data::Sprite entity_sprite{};
		data::ContainerEntity entity{};
		entity.sprite    = &entity_sprite;
		entity.placeable = false;

		// Should NOT set item's sprite at overlay layer at world position 0, 0 since the entity selected is not placeable
		mouseSelection_.DrawOverlay(playerData_, dataManager_, &entity, 0, 0, data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay)
			.prototypeData,
			nullptr);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedNonPlaceableHoverEntity) {
		// A non placeable item is selected over an entity
		// Clear last and draw cursor

		SetupMouseCursor();

		data::Sprite entity_sprite{};
		data::ContainerEntity entity{};
		entity.sprite    = &entity_sprite;
		entity.placeable = false;

		worldData_.GetTile(0, 0)
		          ->GetLayer(ChunkTile::ChunkLayer::entity).prototypeData = &entity;

		// Should NOT set item's sprite at overlay layer at world position 0, 0 since the entity selected is not placeable
		mouseSelection_.DrawOverlay(playerData_, dataManager_, &entity, 0, 0, data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay)
			.prototypeData,
			cursorSprite_);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelected) {
		// NO item is selected
		// CLear last and draw nothing new

		// Should set item's sprite at overlay layer at world position 0, 0
		mouseSelection_.DrawOverlay(playerData_, dataManager_, nullptr, 0, 0, data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay).
			prototypeData,
			nullptr
		);

		// Should clear last overlay at 0,0 Draw new at 1, 0
		mouseSelection_.DrawOverlay(playerData_, dataManager_, nullptr, 1, 0, data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay).
			prototypeData,
			nullptr
		);
		EXPECT_EQ(
			worldData_.GetTile(1, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay).
			prototypeData,
			nullptr
		);

	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelectedEntityHover) {
		// NO item is selected, but cursor is hovered over an entity
		// Draw selection cursor

		SetupMouseCursor();

		data::Sprite entity_sprite{};
		data::ContainerEntity entity{};
		entity.sprite = &entity_sprite;


		worldData_.GetTile(0, 0)
		          ->GetLayer(ChunkTile::ChunkLayer::entity).prototypeData = &entity;


		mouseSelection_.DrawOverlay(playerData_, dataManager_, nullptr, 0, 0, data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay)
			.prototypeData,
			cursorSprite_);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelectedResourceEntityHover) {
		// NO item is selected, but cursor is hovered over a resource entity
		// Draw selection cursor

		SetupMouseCursor();

		data::Sprite entity_sprite{};
		data::ContainerEntity entity{};
		entity.sprite    = &entity_sprite;
		entity.placeable = false;


		worldData_.GetTile(0, 0)
		          ->GetLayer(ChunkTile::ChunkLayer::resource).prototypeData = &entity;


		mouseSelection_.DrawOverlay(playerData_, dataManager_, nullptr, 0, 0, data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay)
			.prototypeData,
			cursorSprite_);
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

	TEST_F(MouseSelectionOverlayTest, DrawOverlay3x3) {

		SetupMouseCursor();

		data::MiningDrill drill{};
		drill.tileWidth  = 3;
		drill.tileHeight = 3;

		data::Sprite drill_sprite{};
		drill.sprite = &drill_sprite;

		mouseSelection_.DrawOverlay(playerData_, dataManager_, &drill, 0, 0, data::Orientation::up);

		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				const data::PrototypeBase* data =
					worldData_.GetTile(x, y)->GetLayer(ChunkTile::ChunkLayer::overlay).
					           prototypeData;
				EXPECT_EQ(data, &drill_sprite);
			}
		}


		// Removing the drawn overlay, no resource or entity data so no cursor is drawn
		mouseSelection_.DrawOverlay(playerData_, dataManager_, nullptr, 0, 0, data::Orientation::up);

		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				const data::PrototypeBase* data =
					worldData_.GetTile(x, y)->GetLayer(ChunkTile::ChunkLayer::overlay).
					           prototypeData;
				EXPECT_EQ(data, nullptr);
			}
		}


		// Drawing a cursor should be 1 x 1
		worldData_.GetTile(0, 0) // Create an entity
		          ->GetLayer(ChunkTile::ChunkLayer::entity).prototypeData = &drill;

		// Should not touch this
		ChunkTileLayer& untouched =
			worldData_.GetTile(1, 0)->GetLayer(ChunkTile::ChunkLayer::overlay);
		untouched.prototypeData = &drill;


		mouseSelection_.DrawOverlay(playerData_, dataManager_, nullptr, 0, 0, data::Orientation::up);

		EXPECT_EQ(untouched.prototypeData, &drill);
	}


	class MockEntity final : public data::Entity
	{
	public:
		PROTOTYPE_CATEGORY(test);

		mutable bool rGetSpriteCalled = false;

		J_NODISCARD data::Sprite::SetT OnRGetSet(data::Orientation,
		                                         WorldData&,
		                                         const WorldData::WorldPair&) const override {
			return 16;
		}

		void OnBuild(WorldData&,
		             LogicData&,
		             const WorldData::WorldPair&,
		             ChunkTileLayer&, data::Orientation) const override {
		}

		void OnRemove(WorldData&,
		              LogicData&,
		              const WorldData::WorldPair&, ChunkTileLayer&) const override {
		}

		std::pair<data::Sprite*, data::Sprite::FrameT>
		OnRGetSprite(const data::UniqueDataBase* /*unique_data*/, GameTickT) const override {
			rGetSpriteCalled = true;
			return {nullptr, 0};
		}
	};

	TEST_F(MouseSelectionOverlayTest, DrawOverlayCallGetSprite) {

		MockEntity entity{};
		entity.rotatable = true;
		entity.placeable = true;

		mouseSelection_.DrawOverlay(playerData_, dataManager_, &entity, 0, 0, data::Orientation::up);

		// On_r_get_sprite should have been called 
		EXPECT_TRUE(entity.rGetSpriteCalled);
	}
}
