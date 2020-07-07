// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/24/2020

#include <gtest/gtest.h>

#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/interface/update_listener.h"
#include "data/prototype/tile/tile.h"
#include "game/player/player_data.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	class PlayerDataPlacementTest : public testing::Test
	{
	protected:
		PlayerData playerData_{};
		WorldData worldData_{};

		data::DataManager dataManager_{};

		// Creates the base tile and entity at world coords
		void SetEntityCoords(const int world_x,
		                     const int world_y,
		                     data::Tile* tile_proto,
		                     data::Entity* entity_proto) const {
			worldData_.GetTile(world_x, world_y)
			          ->GetLayer(ChunkTile::ChunkLayer::base).prototypeData = tile_proto;

			worldData_.GetTile(world_x, world_y)
			          ->GetLayer(ChunkTile::ChunkLayer::entity).prototypeData = entity_proto;
		}
	};

	TEST_F(PlayerDataPlacementTest, RotatePlacementOrientation) {
		EXPECT_EQ(playerData_.placementOrientation, jactorio::data::Orientation::up);

		playerData_.RotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, jactorio::data::Orientation::right);

		playerData_.RotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, jactorio::data::Orientation::down);

		playerData_.RotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, jactorio::data::Orientation::left);

		playerData_.RotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, jactorio::data::Orientation::up);
	}

	TEST_F(PlayerDataPlacementTest, CounterRotatePlacementOrientation) {
		EXPECT_EQ(playerData_.placementOrientation, jactorio::data::Orientation::up);

		playerData_.CounterRotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, jactorio::data::Orientation::left);

		playerData_.CounterRotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, jactorio::data::Orientation::down);

		playerData_.CounterRotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, jactorio::data::Orientation::right);

		playerData_.CounterRotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, jactorio::data::Orientation::up);
	}

	TEST_F(PlayerDataPlacementTest, TryPlaceEntity) {
		// Create entity
		auto item           = data::Item();
		auto item_no_entity = data::Item();  // Does not hold an entity reference

		auto entity = std::make_unique<data::ContainerEntity>();
		entity->SetItem(&item);


		auto entity2 = std::make_unique<data::ContainerEntity>();


		auto tile_proto    = data::Tile();
		tile_proto.isWater = false;

		// Create world with entity at 0, 0
		auto* tiles = new ChunkTile[1024];
		tiles[0].SetTilePrototype(ChunkTile::ChunkLayer::base, &tile_proto);

		tiles[1].SetTilePrototype(ChunkTile::ChunkLayer::base, &tile_proto);

		tiles[1].SetEntityPrototype(ChunkTile::ChunkLayer::entity, entity2.get());

		worldData_.EmplaceChunk(0, 0, tiles);

		// Edge cases
		playerData_.TryPlaceEntity(worldData_, 0, 0);  // Placing with no items selected

		data::Item::Stack selected_item = {&item_no_entity, 2};
		playerData_.SetSelectedItem(selected_item);

		tiles[0].SetEntityPrototype(ChunkTile::ChunkLayer::entity, entity.get());

		playerData_.TryPlaceEntity(worldData_, 0, 0);  // Item holds no reference to an entity
		EXPECT_EQ(
			tiles[0].GetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity),
			entity.get());  // Should not delete item at this location


		// Placement tests

		// Place at 0, 0
		selected_item = {&item, 2};
		playerData_.SetSelectedItem(selected_item);

		tiles[0].SetEntityPrototype(ChunkTile::ChunkLayer::entity, nullptr);

		playerData_.TryPlaceEntity(worldData_, 0, 0);  // Place on empty tile 0, 0

		EXPECT_EQ(
			tiles[0].GetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity),
			entity.get());
		EXPECT_EQ(playerData_.GetSelectedItem()->second, 1);  // 1 less item 

		// The on_build() method should get called, creating unique data on the tile which holds the inventory
		EXPECT_NE(tiles[0].GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity).GetUniqueData(), nullptr);


		// Do not place at 1, 0 
		playerData_.TryPlaceEntity(worldData_, 1, 0);  // A tile already exists on 1, 0 - Should not override it
		EXPECT_EQ(
			tiles[1].GetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity),
			entity2.get());
	}

	TEST_F(PlayerDataPlacementTest, TryPlaceEntityActivateLayer) {
		// Create entity
		auto item           = data::Item();
		auto item_no_entity = data::Item();  // Does not hold an entity reference

		auto entity       = std::make_unique<data::ContainerEntity>();
		entity->placeable = true;
		entity->SetItem(&item);


		auto tile_proto    = data::Tile();
		tile_proto.isWater = false;

		// Create world with entity at 0, 0
		auto* tiles = new ChunkTile[1024];
		tiles[0].SetTilePrototype(ChunkTile::ChunkLayer::base, &tile_proto);

		tiles[0].SetEntityPrototype(ChunkTile::ChunkLayer::entity, entity.get());


		worldData_.EmplaceChunk(0, 0, tiles);

		// If selected item's entity is placeable, do not set activated_layer
		data::Item::Stack selected_item = {&item, 2};
		playerData_.SetSelectedItem(selected_item);

		playerData_.TryPlaceEntity(worldData_, 0, 0, true);
		EXPECT_EQ(playerData_.GetActivatedLayer(), nullptr);

		// Clicking on an entity with no placeable items selected will set activated_layer
		selected_item = {&item_no_entity, 2};
		playerData_.SetSelectedItem(selected_item);

		// However! If mouse_release is not true, do not set activated_layer
		playerData_.TryPlaceEntity(worldData_, 0, 0);
		EXPECT_EQ(playerData_.GetActivatedLayer(), nullptr);

		playerData_.TryPlaceEntity(worldData_, 0, 0, true);
		EXPECT_EQ(playerData_.GetActivatedLayer(),
		          &tiles[0].GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity));

		// Clicking again will NOT unset
		playerData_.TryPlaceEntity(worldData_, 0, 0, true);
		EXPECT_EQ(playerData_.GetActivatedLayer(),
		          &tiles[0].GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity));


		// Activated layer can be set to nullptr to unactivate layers
		playerData_.SetActivatedLayer(nullptr);
		EXPECT_EQ(playerData_.GetActivatedLayer(), nullptr);

	}

	TEST_F(PlayerDataPlacementTest, TryPickupEntityDeactivateLayer) {
		// Picking up an entity wil unset activated layer if activated layer was the entity

		// Create entity
		auto item = data::Item();

		auto entity       = std::make_unique<data::ContainerEntity>();
		entity->placeable = false;
		entity->SetItem(&item);


		auto tile_proto    = data::Tile();
		tile_proto.isWater = false;

		// Create world with entity at 0, 0
		auto* tiles = new ChunkTile[1024];
		tiles[0].SetTilePrototype(ChunkTile::ChunkLayer::base, &tile_proto);

		tiles[0].SetEntityPrototype(ChunkTile::ChunkLayer::entity, entity.get());


		worldData_.EmplaceChunk(0, 0, tiles);

		// Entity is non-placeable, therefore when clicking on an entity, it will get activated_layer
		data::Item::Stack selected_item = {&item, 2};
		playerData_.SetSelectedItem(selected_item);

		// Set
		playerData_.TryPlaceEntity(worldData_, 0, 0, true);
		EXPECT_EQ(playerData_.GetActivatedLayer(),
		          &tiles[0].GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity));

		// Picking up entity will unset
		playerData_.TryPickup(worldData_, 0, 0, 1000);
		EXPECT_EQ(playerData_.GetActivatedLayer(), nullptr);

	}

	TEST_F(PlayerDataPlacementTest, try_pickup_entity) {
		// Create entity
		auto item = data::Item();

		auto* entity       = new data::ContainerEntity();
		entity->pickupTime = 1.f;
		entity->SetItem(&item);
		dataManager_.DataRawAdd("chester", entity);

		// Create world with entity at 0, 0
		auto* tiles = new ChunkTile[1024];
		tiles[0].SetEntityPrototype(ChunkTile::ChunkLayer::entity, entity);

		tiles[1].SetEntityPrototype(ChunkTile::ChunkLayer::entity, entity);

		// Create unique data by calling build event for prototype with layer
		{
			WorldData world_data{};
			entity->OnBuild(world_data,
			                {},
			                tiles[0].GetLayer(ChunkTile::ChunkLayer::entity),
			                data::Orientation::up);
		}

		worldData_.EmplaceChunk(0, 0, tiles);


		// 
		EXPECT_EQ(playerData_.GetPickupPercentage(), 0.f);  // Defaults to 0
		playerData_.TryPickup(worldData_, 0, 2, 990);  // Will not attempt to pickup non entity tiles


		// Test pickup
		playerData_.TryPickup(worldData_, 0, 0, 30);
		EXPECT_EQ(playerData_.GetPickupPercentage(), 0.5f);  // 50% picked up 30 ticks out of 60
		EXPECT_EQ(
			tiles[0].GetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity),
			entity);  // Not picked up yet - 10 more ticks needed to reach 1 second


		playerData_.TryPickup(worldData_, 1, 0, 30);  // Selecting different tile will reset pickup counter
		EXPECT_EQ(
			tiles[1].GetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity),
			entity);  // Not picked up yet - 50 more to 1 second since counter reset

		playerData_.TryPickup(worldData_, 0, 0, 50);
		playerData_.TryPickup(worldData_, 0, 0, 10);
		EXPECT_EQ(
			tiles[0].GetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity),
			nullptr);  // Picked up, item given to inventory

		EXPECT_EQ(playerData_.inventoryPlayer[0].first, &item);
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 1);

		// Unique data for layer should have been deleted
		EXPECT_EQ(tiles[0].GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity).GetUniqueData(), nullptr);
	}

	TEST_F(PlayerDataPlacementTest, TryPickupResource) {
		// Create resource entity
		auto item = data::Item();

		auto* entity       = new data::ResourceEntity();
		entity->pickupTime = 3.f;
		entity->SetItem(&item);
		dataManager_.DataRawAdd("diamond", entity);

		// Create world with the resource entity at 0, 0
		auto* tiles = new ChunkTile[1024];
		worldData_.EmplaceChunk(0, 0, tiles);


		tiles[0].SetEntityPrototype(ChunkTile::ChunkLayer::resource, entity);

		// Holds the resources available at the tile, should be decremented when extracted
		auto* resource_data = tiles[0].GetLayer(ChunkTile::ChunkLayer::resource)
		                              .MakeUniqueData<data::ResourceEntityData>(2);


		//
		playerData_.TryPickup(worldData_, 0, 0, 180);
		// Resource entity should only become nullptr after all the resources are extracted
		EXPECT_EQ(
			tiles[0].GetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::resource),
			entity);

		EXPECT_EQ(resource_data->resourceAmount, 1);

		EXPECT_EQ(playerData_.inventoryPlayer[0].first, &item);  // Gave 1 resource to player
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 1);


		// All resources extracted from resource entity, should now become nullptr
		playerData_.TryPickup(worldData_, 0, 0, 60);
		playerData_.TryPickup(worldData_, 0, 0, 60);
		playerData_.TryPickup(worldData_, 0, 0, 60);
		EXPECT_EQ(
			tiles[1].GetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::resource),
			nullptr);  // Picked up, item given to inventory

		// Resource_data should be deleted

		EXPECT_EQ(playerData_.inventoryPlayer[0].first, &item);
		EXPECT_EQ(playerData_.inventoryPlayer[0].second, 2);  // Player has 2 of resource
	}

	TEST_F(PlayerDataPlacementTest, TryPickupLayered) {
		auto item = data::Item();
		// Create world with the resource entity at 0, 0
		auto* tiles = new ChunkTile[1024];
		worldData_.EmplaceChunk(0, 0, tiles);


		// Resource entity
		auto* resource_entity       = new data::ResourceEntity();
		resource_entity->pickupTime = 3.f;
		resource_entity->SetItem(&item);
		dataManager_.DataRawAdd("diamond", resource_entity);


		tiles[0].SetEntityPrototype(ChunkTile::ChunkLayer::resource, resource_entity);

		// Holds the resources available at the tile, should be decremented when extracted
		auto* resource_data = tiles[0].GetLayer(ChunkTile::ChunkLayer::resource)
		                              .MakeUniqueData<data::ResourceEntityData>(2);


		// Other entity (e.g Container_entity)
		auto* container_entity       = new data::ContainerEntity();
		container_entity->pickupTime = 1.f;
		container_entity->SetItem(&item);

		dataManager_.DataRawAdd("chester", container_entity);

		tiles[0].SetEntityPrototype(ChunkTile::ChunkLayer::entity, container_entity);

		//
		playerData_.TryPickup(worldData_, 0, 0, 60);  // Container entity takes priority
		EXPECT_EQ(
			tiles[0].GetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity),
			nullptr);  // Picked up, item given to inventory


		// Now that container entity is picked up, resource entity is next
		playerData_.TryPickup(worldData_, 0, 0, 60);
		playerData_.TryPickup(worldData_, 0, 0, 60);
		EXPECT_EQ(resource_data->resourceAmount, 2);  // Not picked up, still 60 more ticks required

		playerData_.TryPickup(worldData_, 0, 0, 60);
		EXPECT_EQ(resource_data->resourceAmount, 1);  // Picked up
	}

	TEST_F(PlayerDataPlacementTest, PlacePickupCallPickupRemoveEvents) {
		class MockEntity final : public data::Entity
		{
		public:
			PROTOTYPE_CATEGORY(test);

			mutable bool buildCalled  = false;
			mutable bool removeCalled = false;


			J_NODISCARD data::Sprite::SetT OnRGetSet(data::Orientation,
			                                         WorldData&,
			                                         const WorldData::WorldPair&) const override {
				return 0;
			}

			void OnBuild(WorldData&,
			             const WorldData::WorldPair&,
			             ChunkTileLayer&,
			             data::Orientation) const override {
				buildCalled = true;
			}

			void OnRemove(WorldData&,
			              const WorldData::WorldPair&,
			              ChunkTileLayer&) const override {
				removeCalled = true;
			}
		};
		class MockUpdateListener : public data::IUpdateListener
		{
		public:
			mutable WorldData::WorldPair emit;
			mutable WorldData::WorldPair receive;
			mutable data::UpdateType type = data::UpdateType::remove;

			void OnTileUpdate(WorldData&,
			                  const WorldData::WorldPair& emit_coords,
			                  const WorldData::WorldPair& receive_coords,
			                  const data::UpdateType type) const override {
				emit       = emit_coords;
				receive    = receive_coords;
				this->type = type;
			}
		};

		// The world tile must have a tile prototype
		auto tile_proto    = data::Tile();
		tile_proto.isWater = false;

		worldData_.EmplaceChunk(0, 0);
		worldData_.GetTile(0, 0)->SetTilePrototype(ChunkTile::ChunkLayer::base, &tile_proto);


		// Create entity
		auto item   = data::Item{};
		auto entity = MockEntity{};
		entity.SetItem(&item);


		data::Item::Stack selected_item = {&item, 1};
		playerData_.SetSelectedItem(selected_item);

		// Update listeners should be dispatched
		MockUpdateListener mock_listener;
		worldData_.updateDispatcher.Register(3, 4,
		                                     0, 0, mock_listener);
		// Change to some random data
		mock_listener.emit    = {1, 2};
		mock_listener.receive = {3, 4};

		playerData_.TryPlaceEntity(worldData_, 0, 0, true);

		EXPECT_TRUE(entity.buildCalled);
		EXPECT_EQ(mock_listener.emit.first, 0);
		EXPECT_EQ(mock_listener.emit.second, 0);

		EXPECT_EQ(mock_listener.receive.first, 3);
		EXPECT_EQ(mock_listener.receive.second, 4);

		EXPECT_EQ(mock_listener.type, data::UpdateType::place);


		// Pickup

		// Change to some random data
		mock_listener.emit    = {1, 2};
		mock_listener.receive = {3, 4};

		playerData_.TryPickup(worldData_, 0, 0, 60);
		EXPECT_TRUE(entity.removeCalled);
		EXPECT_EQ(mock_listener.emit.first, 0);
		EXPECT_EQ(mock_listener.emit.second, 0);

		EXPECT_EQ(mock_listener.receive.first, 3);
		EXPECT_EQ(mock_listener.receive.second, 4);

		EXPECT_EQ(mock_listener.type, data::UpdateType::remove);
	}

	TEST_F(PlayerDataPlacementTest, TryPlaceCallOnCanBuild) {
		class MockEntityCanBuild final : public data::Entity
		{
		public:
			PROTOTYPE_CATEGORY(test);

			J_NODISCARD data::Sprite::SetT OnRGetSet(data::Orientation,
			                                         WorldData&,
			                                         const WorldData::WorldPair&)
			const override {
				return 0;
			}

			void OnBuild(WorldData&,
			             const WorldData::WorldPair&,
			             ChunkTileLayer&,
			             data::Orientation) const override {
			}

			void OnRemove(WorldData&,
			              const WorldData::WorldPair&,
			              ChunkTileLayer&) const override {
			}

			// ======================================================================

			J_NODISCARD bool OnCanBuild(const WorldData&,
			                            const WorldData::WorldPair&) const override {
				return false;
			}
		};

		// The world tile must have a tile prototype
		auto tile_proto    = data::Tile();
		tile_proto.isWater = false;

		worldData_.EmplaceChunk(0, 0);
		auto* tile = worldData_.GetTile(0, 0);
		tile->SetTilePrototype(ChunkTile::ChunkLayer::base, &tile_proto);


		// Create entity

		auto item = data::Item{};

		data::Item::Stack selected_item = {&item, 1};
		playerData_.SetSelectedItem(selected_item);

		playerData_.TryPlaceEntity(worldData_, 0, 0, true);

		// Not placed because on_can_build returned false
		EXPECT_EQ(tile->GetEntityPrototype(jactorio::game::ChunkTile::ChunkLayer::entity), nullptr);
	}

	TEST_F(PlayerDataPlacementTest, TryPlaceCallOnNeighborUpdate) {
		// Placing or removing an entity should call on_neighbor_update for 10 adjacent tiles in clockwise order from top left

		class MockEntity final : public data::Entity
		{
		public:
			PROTOTYPE_CATEGORY(test);

			J_NODISCARD data::Sprite::SetT OnRGetSet(data::Orientation,
			                                         WorldData&,
			                                         const WorldData::WorldPair&) const override {
				return 0;
			}

			void OnBuild(WorldData&,
			             const WorldData::WorldPair&,
			             ChunkTileLayer&,
			             data::Orientation) const override {
			}


			void OnRemove(WorldData&,
			              const WorldData::WorldPair&,
			              ChunkTileLayer&) const override {
			}

			// ======================================================================
			mutable std::vector<std::pair<WorldData::WorldCoord, WorldData::WorldCoord>>
			coords;

			void OnNeighborUpdate(WorldData&,
			                      const WorldData::WorldPair& emit_world_coords,
			                      const WorldData::WorldPair& receive_world_coords,
			                      data::Orientation) const override {
				EXPECT_EQ(emit_world_coords.first, 1);
				EXPECT_EQ(emit_world_coords.second, 1);
				coords.push_back(receive_world_coords);
			}
		};

		/*
		 *     [1] [2]
		 * [A] [x] [x] [3]
		 * [9] [x] [x] [4]
		 * [8] [x] [x] [5]
		 *     [7] [6]
		 */

		// Tile
		auto tile_proto    = data::Tile();
		tile_proto.isWater = false;

		// Item

		auto item = data::Item{};

		// Entity
		auto entity_proto       = MockEntity{};
		entity_proto.tileWidth  = 2;
		entity_proto.tileHeight = 3;
		entity_proto.SetItem(&item);

		worldData_.AddChunk(Chunk{0, 0});

		// Set tiles so entity can be placed on it
		for (int y = 1; y < 4; ++y) {
			for (int x = 1; x < 3; ++x) {
				worldData_.GetTile(x, y)
				          ->GetLayer(ChunkTile::ChunkLayer::base).prototypeData = &tile_proto;
			}
		}

		// Set entity around border
		for (int x = 1; x <= 2; ++x) {
			SetEntityCoords(x, 0, &tile_proto, &entity_proto);
		}
		for (int y = 1; y <= 3; ++y) {
			SetEntityCoords(3, y, &tile_proto, &entity_proto);
		}
		for (int x = 2; x >= 1; --x) {
			SetEntityCoords(x, 4, &tile_proto, &entity_proto);
		}
		for (int y = 3; y >= 1; --y) {
			SetEntityCoords(0, y, &tile_proto, &entity_proto);
		}

		// ======================================================================

		data::Item::Stack selected_item = {&item, 1};
		playerData_.SetSelectedItem(selected_item);

		playerData_.TryPlaceEntity(worldData_, 1, 1, true);
		ASSERT_EQ(entity_proto.coords.size(), 10);

#define VALIDATE_COORDS(index, x, y)\
		{\
		auto pair = std::pair<jactorio::game::WorldData::WorldCoord, jactorio::game::WorldData::WorldCoord>{x, y};\
		EXPECT_EQ(entity_proto.coords[index], pair);\
		}

		VALIDATE_COORDS(0, 1, 0);
		VALIDATE_COORDS(1, 2, 0);

		VALIDATE_COORDS(2, 3, 1);
		VALIDATE_COORDS(3, 3, 2);
		VALIDATE_COORDS(4, 3, 3);

		VALIDATE_COORDS(5, 2, 4);
		VALIDATE_COORDS(6, 1, 4);

		VALIDATE_COORDS(7, 0, 3);
		VALIDATE_COORDS(8, 0, 2);
		VALIDATE_COORDS(9, 0, 1);

		// ======================================================================

		playerData_.TryPickup(worldData_, 1, 1, 9999);
		ASSERT_EQ(entity_proto.coords.size(), 20);
	}
}
