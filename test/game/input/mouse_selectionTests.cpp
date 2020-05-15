// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/05/2020

#include <gtest/gtest.h>

#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/mining_drill.h"
#include "game/input/mouse_selection.h"

namespace game
{
	class MouseSelectionOverlayTest : public testing::Test
	{
		bool setupCursor_ = false;

	protected:
		jactorio::game::WorldData worldData_{};
		jactorio::game::PlayerData playerData_{};
		jactorio::game::MouseSelection mouseSelection_{};

		// Call setup_mouse_cursor before using
		jactorio::data::Sprite* cursorSprite_ = nullptr;


		void SetUp() override {
			playerData_.SetPlayerWorld(&worldData_);
			worldData_.AddChunk(jactorio::game::Chunk(0, 0));
		}

		void TearDown() override {
			if (setupCursor_) {
				jactorio::data::ClearData();
			}
		}

		///
		/// \brief If the test involves the selection cursor, which is loaded in the data_manager
		void SetupMouseCursor() {
			setupCursor_  = true;
			cursorSprite_ = new jactorio::data::Sprite{};
			DataRawAdd("__core__/cursor-select", cursorSprite_);
		}
	};

	TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedPlaceable) {
		// An item is selected
		// Clear last and draw new
		jactorio::data::Sprite entity_sprite{};
		jactorio::data::ContainerEntity entity{};
		entity.sprite    = &entity_sprite;
		entity.placeable = true;

		// Should set item's sprite at overlay layer at world position 0, 0
		mouseSelection_.DrawOverlay(playerData_, &entity, 0, 0, jactorio::data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay).
			prototypeData,
			&entity_sprite
		);

		// Should clear last overlay at 0,0 Draw new at 1, 0
		mouseSelection_.DrawOverlay(playerData_, &entity, 1, 0, jactorio::data::Orientation::up);
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

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::ContainerEntity entity{};
		entity.sprite    = &entity_sprite;
		entity.placeable = false;

		// Should NOT set item's sprite at overlay layer at world position 0, 0 since the entity selected is not placeable
		mouseSelection_.DrawOverlay(playerData_, &entity, 0, 0, jactorio::data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay)
			.prototypeData,
			nullptr);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayItemSelectedNonPlaceableHoverEntity) {
		// A non placeable item is selected over an entity
		// Clear last and draw cursor

		SetupMouseCursor();

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::ContainerEntity entity{};
		entity.sprite    = &entity_sprite;
		entity.placeable = false;

		worldData_.GetTile(0, 0)
		          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity).prototypeData = &entity;

		// Should NOT set item's sprite at overlay layer at world position 0, 0 since the entity selected is not placeable
		mouseSelection_.DrawOverlay(playerData_, &entity, 0, 0, jactorio::data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay)
			.prototypeData,
			cursorSprite_);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelected) {
		// NO item is selected
		// CLear last and draw nothing new

		// Should set item's sprite at overlay layer at world position 0, 0
		mouseSelection_.DrawOverlay(playerData_, nullptr, 0, 0, jactorio::data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay).
			prototypeData,
			nullptr
		);

		// Should clear last overlay at 0,0 Draw new at 1, 0
		mouseSelection_.DrawOverlay(playerData_, nullptr, 1, 0, jactorio::data::Orientation::up);
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

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::ContainerEntity entity{};
		entity.sprite = &entity_sprite;


		worldData_.GetTile(0, 0)
		          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity).prototypeData = &entity;


		mouseSelection_.DrawOverlay(playerData_, nullptr, 0, 0, jactorio::data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay)
			.prototypeData,
			cursorSprite_);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNotSelectedResourceEntityHover) {
		// NO item is selected, but cursor is hovered over a resource entity
		// Draw selection cursor

		SetupMouseCursor();

		jactorio::data::Sprite entity_sprite{};
		jactorio::data::ContainerEntity entity{};
		entity.sprite    = &entity_sprite;
		entity.placeable = false;


		worldData_.GetTile(0, 0)
		          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::resource).prototypeData = &entity;


		mouseSelection_.DrawOverlay(playerData_, nullptr, 0, 0, jactorio::data::Orientation::up);
		EXPECT_EQ(
			worldData_.GetTile(0, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay)
			.prototypeData,
			cursorSprite_);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayNoWorld) {
		// When selecting no world / chunk not generated
		// Attempt to clear last tile and do nothing

		jactorio::game::WorldData world_data{};
		jactorio::game::PlayerData player_data{};
		player_data.SetPlayerWorld(&world_data);

		jactorio::game::MouseSelection mouse_selection{};

		mouse_selection.DrawOverlay(player_data, nullptr, 0, 0, jactorio::data::Orientation::up);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlay3x3) {

		SetupMouseCursor();

		jactorio::data::MiningDrill drill{};
		drill.tileWidth  = 3;
		drill.tileHeight = 3;

		jactorio::data::Sprite drill_sprite{};
		drill.sprite = &drill_sprite;

		mouseSelection_.DrawOverlay(playerData_, &drill, 0, 0, jactorio::data::Orientation::up);

		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				const jactorio::data::PrototypeBase* data =
					worldData_.GetTile(x, y)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay).
					           prototypeData;
				EXPECT_EQ(data, &drill_sprite);
			}
		}


		// Removing the drawn overlay, no resource or entity data so no cursor is drawn
		mouseSelection_.DrawOverlay(playerData_, nullptr, 0, 0, jactorio::data::Orientation::up);

		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				const jactorio::data::PrototypeBase* data =
					worldData_.GetTile(x, y)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay).
					           prototypeData;
				EXPECT_EQ(data, nullptr);
			}
		}


		// Drawing a cursor should be 1 x 1
		worldData_.GetTile(0, 0) // Create an entity
		          ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity).prototypeData = &drill;

		// Should not touch this
		jactorio::game::ChunkTileLayer& untouched =
			worldData_.GetTile(1, 0)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::overlay);
		untouched.prototypeData = &drill;


		mouseSelection_.DrawOverlay(playerData_,
		                            nullptr,
		                            0,
		                            0,
		                            jactorio::data::Orientation::up);

		EXPECT_EQ(untouched.prototypeData, &drill);
	}

	TEST_F(MouseSelectionOverlayTest, DrawOverlayCallGetSprite) {
		class MockEntity final : public jactorio::data::Entity
		{
		public:
			PROTOTYPE_CATEGORY(test);

			mutable bool rGetSpriteCalled = false;

			J_NODISCARD std::pair<uint16_t, uint16_t> MapPlacementOrientation(jactorio::data::Orientation,
			                                                                  jactorio::game::WorldData&,
			                                                                  const jactorio::game::WorldData::WorldPair&)
			const override {
				return {16, 17};
			}

			void OnRShowGui(jactorio::game::PlayerData&,
			                jactorio::game::ChunkTileLayer*) const override {
			}


			void OnBuild(jactorio::game::WorldData&,
			             const jactorio::game::WorldData::WorldPair&,
			             jactorio::game::ChunkTileLayer&,
			             jactorio::data::Orientation) const override {
			}

			void OnRemove(jactorio::game::WorldData&,
			              const jactorio::game::WorldData::WorldPair&,
			              jactorio::game::ChunkTileLayer&) const override {
			}

			// Overriden to give nullptr
			std::pair<jactorio::data::Sprite*, jactorio::data::RenderableData::frame_t>
			OnRGetSprite(jactorio::data::UniqueDataBase* /*unique_data*/, jactorio::GameTickT) const override {
				rGetSpriteCalled = true;
				return {nullptr, 0};
			}
		};

		MockEntity entity{};
		entity.rotatable = true;
		entity.placeable = true;

		mouseSelection_.DrawOverlay(playerData_, &entity, 0, 0, jactorio::data::Orientation::up);

		// On_r_get_sprite should have been called 
		EXPECT_TRUE(entity.rGetSpriteCalled);
	}
}
