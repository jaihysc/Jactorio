// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/player/player_data.h"

#include "jactorioTests.h"
#include "data/prototype_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/interface/update_listener.h"
#include "data/prototype/tile/tile.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	class PlayerDataPlacementTest : public testing::Test
	{
	protected:
		PlayerData playerData_{};
		LogicData logicData_{};
		WorldData worldData_{};

		data::PrototypeManager dataManager_{};

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
		EXPECT_EQ(playerData_.placementOrientation, data::Orientation::up);

		playerData_.RotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, data::Orientation::right);

		playerData_.RotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, data::Orientation::down);

		playerData_.RotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, data::Orientation::left);

		playerData_.RotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, data::Orientation::up);
	}

	TEST_F(PlayerDataPlacementTest, CounterRotatePlacementOrientation) {
		EXPECT_EQ(playerData_.placementOrientation, data::Orientation::up);

		playerData_.CounterRotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, data::Orientation::left);

		playerData_.CounterRotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, data::Orientation::down);

		playerData_.CounterRotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, data::Orientation::right);

		playerData_.CounterRotatePlacementOrientation();
		EXPECT_EQ(playerData_.placementOrientation, data::Orientation::up);
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
		worldData_.EmplaceChunk(0, 0);

		auto& tile  = *worldData_.GetTile(0, 0);
		auto& tile2 = *worldData_.GetTile(1, 0);

		tile.SetTilePrototype(&tile_proto);

		tile2.SetTilePrototype(&tile_proto);
		tile2.SetEntityPrototype(entity2.get());


		// Edge cases
		EXPECT_FALSE(playerData_.TryPlaceEntity(worldData_, logicData_, 0, 0));  // Placing with no items selected

		playerData_.SetSelectedItem({&item_no_entity, 2});

		tile.SetEntityPrototype(entity.get());

		EXPECT_FALSE(playerData_.TryPlaceEntity(worldData_, logicData_, 0, 0));  // Item holds no reference to an entity
		EXPECT_EQ(tile.GetEntityPrototype(),
		          entity.get());  // Should not delete item at this location


		// Placement tests

		// Place at 0, 0
		playerData_.SetSelectedItem({&item, 2});

		tile.SetEntityPrototype(nullptr);

		EXPECT_TRUE(playerData_.TryPlaceEntity(worldData_, logicData_, 0, 0));  // Place on empty tile 0, 0

		EXPECT_EQ(
			tile.GetEntityPrototype(),
			entity.get());
		EXPECT_EQ(playerData_.GetSelectedItemStack()->count, 1);  // 1 less item 

		// The on_build() method should get called, creating unique data on the tile which holds the inventory
		EXPECT_NE(tile.GetLayer(ChunkTile::ChunkLayer::entity).GetUniqueData(), nullptr);


		// Do not place at 1, 0

		// A tile already exists on 1, 0 - Should not override it
		EXPECT_FALSE(playerData_.TryPlaceEntity(worldData_, logicData_, 1, 0));
		EXPECT_EQ(tile2.GetEntityPrototype(), entity2.get());
	}

	TEST_F(PlayerDataPlacementTest, TryActivateLayer) {
		// Create entity
		auto item           = data::Item();
		auto item_no_entity = data::Item();  // Does not hold an entity reference

		auto entity       = std::make_unique<data::ContainerEntity>();
		entity->placeable = true;
		entity->SetItem(&item);


		auto tile_proto    = data::Tile();
		tile_proto.isWater = false;

		// Create world with entity at 0, 0
		worldData_.EmplaceChunk(0, 0);
		auto* tile = worldData_.GetTile(0, 0);

		tile->SetTilePrototype(&tile_proto);

		// No entity, do not activate layer 
		EXPECT_FALSE(playerData_.TryActivateLayer(worldData_, {0, 0}));


		// If selected item's entity is placeable, do not set activated_layer
		tile->SetEntityPrototype(entity.get());

		playerData_.SetSelectedItem({&item, 2});

		EXPECT_FALSE(playerData_.TryPlaceEntity(worldData_, logicData_, 0, 0));
		EXPECT_EQ(playerData_.GetActivatedLayer(), nullptr);


		// Clicking on an entity with no placeable items selected will set activated_layer
		playerData_.SetSelectedItem({&item_no_entity, 2});

		EXPECT_TRUE(playerData_.TryActivateLayer(worldData_, {0, 0}));
		EXPECT_EQ(playerData_.GetActivatedLayer(),
		          &tile->GetLayer(ChunkTile::ChunkLayer::entity));


		// Clicking again will NOT unset
		EXPECT_TRUE(playerData_.TryActivateLayer(worldData_, {0, 0}));
		EXPECT_EQ(playerData_.GetActivatedLayer(),
		          &tile->GetLayer(ChunkTile::ChunkLayer::entity));


		// Activated layer can be set to nullptr to unactivate layers
		playerData_.SetActivatedLayer(nullptr);
		EXPECT_EQ(playerData_.GetActivatedLayer(), nullptr);
	}

	TEST_F(PlayerDataPlacementTest, TryPickupEntityDeactivateLayer) {
		// Picking up an entity wil unset activated layer if activated layer was the entity

		// Create entity
		auto item = data::Item();

		auto entity        = std::make_unique<data::ContainerEntity>();
		entity->tileWidth  = 3;
		entity->tileHeight = 4;
		entity->SetItem(&item);


		auto tile_proto    = data::Tile();
		tile_proto.isWater = false;

		// Create world with entity at 0, 0
		worldData_.EmplaceChunk(0, 0);


		for (uint32_t y = 0; y < entity->tileHeight; ++y) {
			for (uint32_t x = 0; x < entity->tileWidth; ++x) {
				auto* tile = worldData_.GetTile(x, y);
				tile->SetTilePrototype(&tile_proto);
			}
		}


		// Place entity
		playerData_.SetSelectedItem({&item, 2});

		EXPECT_TRUE(playerData_.TryPlaceEntity(worldData_, logicData_, 0, 0));


		// Entity is non-placeable, therefore when clicking on an entity, it will get activated_layer
		playerData_.DecrementSelectedItem();
		auto* tile = worldData_.GetTile(0, 0);

		EXPECT_TRUE(playerData_.TryActivateLayer(worldData_, {2, 3}));
		EXPECT_EQ(playerData_.GetActivatedLayer(),
		          &tile->GetLayer(ChunkTile::ChunkLayer::entity));


		// Picking up entity will unset
		playerData_.TryPickup(worldData_, logicData_, 0, 1, 1000);
		EXPECT_EQ(playerData_.GetActivatedLayer(), nullptr);

	}

	TEST_F(PlayerDataPlacementTest, TryPickupEntity) {
		// Create entity
		auto item = data::Item();

		auto& entity      = dataManager_.AddProto<data::ContainerEntity>();
		entity.pickupTime = 1.f;
		entity.SetItem(&item);

		// Create world with entity at 0, 0
		worldData_.EmplaceChunk(0, 0);

		auto& tile  = *worldData_.GetTile(0, 0);
		auto& tile2 = *worldData_.GetTile(1, 0);

		tile.SetEntityPrototype(&entity);
		tile2.SetEntityPrototype(&entity);

		// Create unique data by calling build event for prototype with layer
		{
			WorldData world_data{};
			entity.OnBuild(world_data, logicData_,
			               {},
			               tile.GetLayer(ChunkTile::ChunkLayer::entity), data::Orientation::up);
		}


		// 
		EXPECT_EQ(playerData_.GetPickupPercentage(), 0.f);  // Defaults to 0
		playerData_.TryPickup(worldData_, logicData_, 0, 2, 990);  // Will not attempt to pickup non entity tiles


		// Test pickup
		playerData_.TryPickup(worldData_, logicData_, 0, 0, 30);
		EXPECT_EQ(playerData_.GetPickupPercentage(), 0.5f);  // 50% picked up 30 ticks out of 60
		EXPECT_EQ(
			tile.GetEntityPrototype(),
			&entity);  // Not picked up yet - 10 more ticks needed to reach 1 second


		playerData_.TryPickup(worldData_, logicData_, 1, 0, 30);  // Selecting different tile will reset pickup counter
		EXPECT_EQ(
			tile2.GetEntityPrototype(),
			&entity);  // Not picked up yet - 50 more to 1 second since counter reset

		playerData_.TryPickup(worldData_, logicData_, 0, 0, 50);
		playerData_.TryPickup(worldData_, logicData_, 0, 0, 10);
		EXPECT_EQ(
			tile.GetEntityPrototype(),
			nullptr);  // Picked up, item given to inventory

		EXPECT_EQ(playerData_.inventoryPlayer[0].item, &item);
		EXPECT_EQ(playerData_.inventoryPlayer[0].count, 1);

		// Unique data for layer should have been deleted
		EXPECT_EQ(tile.GetLayer(ChunkTile::ChunkLayer::entity).GetUniqueData(), nullptr);
	}

	TEST_F(PlayerDataPlacementTest, TryPickupResource) {
		// Create resource entity
		auto item = data::Item();

		auto& entity      = dataManager_.AddProto<data::ResourceEntity>();
		entity.pickupTime = 3.f;
		entity.SetItem(&item);

		// Create world with the resource entity at 0, 0
		worldData_.EmplaceChunk(0, 0);

		auto& tile  = *worldData_.GetTile(0, 0);
		auto& tile2 = *worldData_.GetTile(1, 0);

		tile.SetEntityPrototype(&entity, ChunkTile::ChunkLayer::resource);

		// Holds the resources available at the tile, should be decremented when extracted
		auto* resource_data = tile.GetLayer(ChunkTile::ChunkLayer::resource)
		                          .MakeUniqueData<data::ResourceEntityData>(2);


		//
		playerData_.TryPickup(worldData_, logicData_, 0, 0, 180);
		// Resource entity should only become nullptr after all the resources are extracted
		EXPECT_EQ(
			tile.GetEntityPrototype(ChunkTile::ChunkLayer::resource),
			&entity);

		EXPECT_EQ(resource_data->resourceAmount, 1);

		EXPECT_EQ(playerData_.inventoryPlayer[0].item, &item);  // Gave 1 resource to player
		EXPECT_EQ(playerData_.inventoryPlayer[0].count, 1);


		// All resources extracted from resource entity, should now become nullptr
		playerData_.TryPickup(worldData_, logicData_, 0, 0, 60);
		playerData_.TryPickup(worldData_, logicData_, 0, 0, 60);
		playerData_.TryPickup(worldData_, logicData_, 0, 0, 60);
		EXPECT_EQ(
			tile2.GetEntityPrototype(ChunkTile::ChunkLayer::resource),
			nullptr);  // Picked up, item given to inventory

		// Resource_data should be deleted

		EXPECT_EQ(playerData_.inventoryPlayer[0].item, &item);
		EXPECT_EQ(playerData_.inventoryPlayer[0].count, 2);  // Player has 2 of resource
	}

	TEST_F(PlayerDataPlacementTest, TryPickupLayered) {
		auto item = data::Item();
		// Create world with the resource entity at 0, 0
		worldData_.EmplaceChunk(0, 0);
		auto& tile = *worldData_.GetTile(0, 0);


		// Resource entity
		auto& resource_entity      = dataManager_.AddProto<data::ResourceEntity>();
		resource_entity.pickupTime = 3.f;
		resource_entity.SetItem(&item);


		tile.SetEntityPrototype(&resource_entity, ChunkTile::ChunkLayer::resource);

		// Holds the resources available at the tile, should be decremented when extracted
		auto* resource_data = tile.GetLayer(ChunkTile::ChunkLayer::resource)
		                          .MakeUniqueData<data::ResourceEntityData>(2);


		// Other entity (e.g Container_entity)
		auto& container_entity      = dataManager_.AddProto<data::ContainerEntity>();
		container_entity.pickupTime = 1.f;
		container_entity.SetItem(&item);


		tile.SetEntityPrototype(&container_entity);

		//
		playerData_.TryPickup(worldData_, logicData_, 0, 0, 60);  // Container entity takes priority
		EXPECT_EQ(tile.GetEntityPrototype(), nullptr);  // Picked up, item given to inventory


		// Now that container entity is picked up, resource entity is next
		playerData_.TryPickup(worldData_, logicData_, 0, 0, 60);
		playerData_.TryPickup(worldData_, logicData_, 0, 0, 60);
		EXPECT_EQ(resource_data->resourceAmount, 2);  // Not picked up, still 60 more ticks required

		playerData_.TryPickup(worldData_, logicData_, 0, 0, 60);
		EXPECT_EQ(resource_data->resourceAmount, 1);  // Picked up
	}


	// ======================================================================


	class MockEntityPlacement final : public data::Entity
	{
	public:
		PROTOTYPE_CATEGORY(test);

		mutable bool buildCalled  = false;
		mutable bool removeCalled = false;

		mutable bool onCanBuildReturn = true;

		mutable std::vector<WorldCoord> emitCoords;
		mutable std::vector<WorldCoord> receiveCoords;

		J_NODISCARD data::Sprite::SetT OnRGetSpriteSet(data::Orientation,
		                                               WorldData&,
		                                               const WorldCoord&) const override {
			return 0;
		}

		void OnBuild(WorldData&,
		             LogicData&,
		             const WorldCoord&,
		             ChunkTileLayer&, data::Orientation) const override {
			buildCalled = true;
		}

		void OnRemove(WorldData&,
		              LogicData&,
		              const WorldCoord&, ChunkTileLayer&) const override {
			removeCalled = true;
		}


		J_NODISCARD bool OnCanBuild(const WorldData&,
		                            const WorldCoord&) const override {
			return onCanBuildReturn;
		}


		void OnNeighborUpdate(WorldData&,
		                      LogicData&,
		                      const WorldCoord& emit_world_coords,
		                      const WorldCoord& receive_world_coords, data::Orientation) const override {
			emitCoords.push_back(emit_world_coords);
			receiveCoords.push_back(receive_world_coords);
		}
	};

	TEST_F(PlayerDataPlacementTest, PlacePickupCallPickupRemoveEvents) {
		class MockUpdateListener : public data::IUpdateListener
		{
		public:
			mutable WorldCoord emit;
			mutable WorldCoord receive;
			mutable data::UpdateType type = data::UpdateType::remove;

			void OnTileUpdate(WorldData&,
			                  const WorldCoord& emit_coords,
			                  const WorldCoord& receive_coords,
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
		worldData_.GetTile(0, 0)->SetTilePrototype(&tile_proto);


		// Create entity
		auto item   = data::Item{};
		auto entity = MockEntityPlacement{};
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

		playerData_.TryPlaceEntity(worldData_, logicData_, 0, 0);

		EXPECT_TRUE(entity.buildCalled);
		EXPECT_EQ(mock_listener.emit.x, 0);
		EXPECT_EQ(mock_listener.emit.y, 0);

		EXPECT_EQ(mock_listener.receive.x, 3);
		EXPECT_EQ(mock_listener.receive.y, 4);

		EXPECT_EQ(mock_listener.type, data::UpdateType::place);


		// Pickup

		// Change to some random data
		mock_listener.emit    = {1, 2};
		mock_listener.receive = {3, 4};

		playerData_.TryPickup(worldData_, logicData_, 0, 0, 60);
		EXPECT_TRUE(entity.removeCalled);
		EXPECT_EQ(mock_listener.emit.x, 0);
		EXPECT_EQ(mock_listener.emit.y, 0);

		EXPECT_EQ(mock_listener.receive.x, 3);
		EXPECT_EQ(mock_listener.receive.y, 4);

		EXPECT_EQ(mock_listener.type, data::UpdateType::remove);
	}

	TEST_F(PlayerDataPlacementTest, TryPlaceCallOnCanBuild) {
		// The world tile must have a tile prototype
		auto tile_proto    = data::Tile();
		tile_proto.isWater = false;

		worldData_.EmplaceChunk(0, 0);
		auto* tile = worldData_.GetTile(0, 0);
		tile->SetTilePrototype(&tile_proto);


		// Create entity
		auto item = data::Item{};

		auto entity             = MockEntityPlacement{};
		entity.onCanBuildReturn = false;
		entity.SetItem(&item);


		data::Item::Stack selected_item = {&item, 1};
		playerData_.SetSelectedItem(selected_item);

		playerData_.TryPlaceEntity(worldData_, logicData_, 0, 0);

		// Not placed because onCanBuild returned false
		EXPECT_EQ(tile->GetEntityPrototype(), nullptr);
	}

	TEST_F(PlayerDataPlacementTest, TryPlaceTryRemoveCallOnNeighborUpdate) {
		// Placing or removing an entity should call on_neighbor_update for 10 adjacent tiles in clockwise order from top left

		//     [1] [2]
		// [A] [x] [x] [3]
		// [9] [x] [x] [4]
		// [8] [x] [x] [5]
		//     [7] [6]

		auto tile_proto    = data::Tile();
		tile_proto.isWater = false;

		auto item = data::Item{};

		auto entity_proto       = MockEntityPlacement{};
		entity_proto.tileWidth  = 2;
		entity_proto.tileHeight = 3;
		entity_proto.SetItem(&item);

		worldData_.EmplaceChunk(0, 0);

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
		// Place

		data::Item::Stack selected_item = {&item, 1};
		playerData_.SetSelectedItem(selected_item);

		playerData_.TryPlaceEntity(worldData_, logicData_, 1, 1);
		ASSERT_EQ(entity_proto.emitCoords.size(), 10);
		ASSERT_EQ(entity_proto.receiveCoords.size(), 10);


		auto validate_coords = [&](const size_t index,
		                           const WorldCoord& emit_coords, const WorldCoord& receive_coords) {
			EXPECT_EQ(entity_proto.emitCoords[index], emit_coords);
			EXPECT_EQ(entity_proto.receiveCoords[index], receive_coords);
		};

		validate_coords(0, {1, 1}, {1, 0});
		validate_coords(1, {2, 1}, {2, 0});

		validate_coords(2, {2, 1}, {3, 1});
		validate_coords(3, {2, 2}, {3, 2});
		validate_coords(4, {2, 3}, {3, 3});

		validate_coords(5, {2, 3}, {2, 4});
		validate_coords(6, {1, 3}, {1, 4});

		validate_coords(7, {1, 3}, {0, 3});
		validate_coords(8, {1, 2}, {0, 2});
		validate_coords(9, {1, 1}, {0, 1});

		// ======================================================================
		// Remove

		playerData_.TryPickup(worldData_, logicData_, 2, 3, 9999);  // Bottom right corner
		EXPECT_EQ(entity_proto.emitCoords.size(), 20);
		EXPECT_EQ(entity_proto.receiveCoords.size(), 20);

		validate_coords(10, {1, 1}, {1, 0});
		validate_coords(11, {2, 1}, {2, 0});

		validate_coords(12, {2, 1}, {3, 1});
		validate_coords(13, {2, 2}, {3, 2});
		validate_coords(14, {2, 3}, {3, 3});

		validate_coords(15, {2, 3}, {2, 4});
		validate_coords(16, {1, 3}, {1, 4});

		validate_coords(17, {1, 3}, {0, 3});
		validate_coords(18, {1, 2}, {0, 2});
		validate_coords(19, {1, 1}, {0, 1});
	}
}
