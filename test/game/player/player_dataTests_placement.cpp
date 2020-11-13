// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/player/player_data.h"

#include "jactorioTests.h"

#include "proto/tile.h"

namespace jactorio::game
{
    class PlayerDataPlacementTest : public testing::Test
    {
    protected:
        PlayerData playerData_;
        PlayerData::Inventory& playerInv_   = playerData_.inventory;
        PlayerData::Placement& playerPlace_ = playerData_.placement;

        LogicData logicData_;
        WorldData worldData_;

        data::PrototypeManager dataManager_;

        // Creates the base tile and entity at world coords
        void SetEntityCoords(const int world_x,
                             const int world_y,
                             const proto::Tile* tile_proto,
                             const proto::Entity* entity_proto) {
            worldData_.GetTile(world_x, world_y)->GetLayer(TileLayer::base).prototypeData = tile_proto;

            worldData_.GetTile(world_x, world_y)->GetLayer(TileLayer::entity).prototypeData = entity_proto;
        }
    };

    TEST_F(PlayerDataPlacementTest, RotatePlacementOrientation) {
        EXPECT_EQ(playerPlace_.orientation, proto::Orientation::up);

        playerPlace_.RotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, proto::Orientation::right);

        playerPlace_.RotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, proto::Orientation::down);

        playerPlace_.RotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, proto::Orientation::left);

        playerPlace_.RotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, proto::Orientation::up);
    }

    TEST_F(PlayerDataPlacementTest, CounterRotatePlacementOrientation) {
        EXPECT_EQ(playerPlace_.orientation, proto::Orientation::up);

        playerPlace_.CounterRotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, proto::Orientation::left);

        playerPlace_.CounterRotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, proto::Orientation::down);

        playerPlace_.CounterRotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, proto::Orientation::right);

        playerPlace_.CounterRotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, proto::Orientation::up);
    }

    TEST_F(PlayerDataPlacementTest, TryPlaceEntity) {
        // Create entity
        auto item           = proto::Item();
        auto item_no_entity = proto::Item(); // Does not hold an entity reference

        auto entity = std::make_unique<proto::ContainerEntity>();
        entity->SetItem(&item);


        auto entity2 = std::make_unique<proto::ContainerEntity>();


        auto tile_proto    = proto::Tile();
        tile_proto.isWater = false;

        // Create world with entity at 0, 0
        worldData_.EmplaceChunk(0, 0);

        auto& tile  = *worldData_.GetTile(0, 0);
        auto& tile2 = *worldData_.GetTile(1, 0);

        tile.SetTilePrototype(&tile_proto);

        tile2.SetTilePrototype(&tile_proto);
        tile2.SetEntityPrototype(entity2.get());


        // Edge cases
        EXPECT_FALSE(playerPlace_.TryPlaceEntity(worldData_, logicData_, 0, 0)); // Placing with no items selected

        playerInv_.SetSelectedItem({&item_no_entity, 2});

        tile.SetEntityPrototype(entity.get());

        EXPECT_FALSE(playerPlace_.TryPlaceEntity(worldData_, logicData_, 0, 0)); // Item holds no reference to an entity
        EXPECT_EQ(tile.GetEntityPrototype(),
                  entity.get()); // Should not delete item at this location


        // Placement tests

        // Place at 0, 0
        playerInv_.SetSelectedItem({&item, 2});

        tile.SetEntityPrototype(nullptr);

        EXPECT_TRUE(playerPlace_.TryPlaceEntity(worldData_, logicData_, 0, 0)); // Place on empty tile 0, 0

        EXPECT_EQ(tile.GetEntityPrototype(), entity.get());
        EXPECT_EQ(playerInv_.GetSelectedItem()->count, 1); // 1 less item

        // The on_build() method should get called, creating unique data on the tile which holds the inventory
        EXPECT_NE(tile.GetLayer(TileLayer::entity).GetUniqueData(), nullptr);


        // Do not place at 1, 0

        // A tile already exists on 1, 0 - Should not override it
        EXPECT_FALSE(playerPlace_.TryPlaceEntity(worldData_, logicData_, 1, 0));
        EXPECT_EQ(tile2.GetEntityPrototype(), entity2.get());
    }

    TEST_F(PlayerDataPlacementTest, TryActivateLayer) {
        // Create entity
        auto item           = proto::Item();
        auto item_no_entity = proto::Item(); // Does not hold an entity reference

        auto entity       = std::make_unique<proto::ContainerEntity>();
        entity->placeable = true;
        entity->SetItem(&item);


        auto tile_proto    = proto::Tile();
        tile_proto.isWater = false;

        // Create world with entity at 0, 0
        worldData_.EmplaceChunk(0, 0);
        auto* tile = worldData_.GetTile(0, 0);

        tile->SetTilePrototype(&tile_proto);

        // No entity, do not activate layer
        EXPECT_FALSE(playerPlace_.TryActivateLayer(worldData_, {0, 0}));


        // If selected item's entity is placeable, do not set activated_layer
        tile->SetEntityPrototype(entity.get());

        playerInv_.SetSelectedItem({&item, 2});

        EXPECT_FALSE(playerPlace_.TryPlaceEntity(worldData_, logicData_, 0, 0));
        EXPECT_EQ(playerPlace_.GetActivatedLayer(), nullptr);


        // Clicking on an entity with no placeable items selected will set activated_layer
        playerInv_.SetSelectedItem({&item_no_entity, 2});

        EXPECT_TRUE(playerPlace_.TryActivateLayer(worldData_, {0, 0}));
        EXPECT_EQ(playerPlace_.GetActivatedLayer(), &tile->GetLayer(TileLayer::entity));


        // Clicking again will NOT unset
        EXPECT_TRUE(playerPlace_.TryActivateLayer(worldData_, {0, 0}));
        EXPECT_EQ(playerPlace_.GetActivatedLayer(), &tile->GetLayer(TileLayer::entity));


        // Activated layer can be set to nullptr to unactivate layers
        playerPlace_.SetActivatedLayer(nullptr);
        EXPECT_EQ(playerPlace_.GetActivatedLayer(), nullptr);
    }

    TEST_F(PlayerDataPlacementTest, TryPickupEntityDeactivateLayer) {
        // Picking up an entity wil unset activated layer if activated layer was the entity

        // Create entity
        auto item = proto::Item();

        auto entity        = std::make_unique<proto::ContainerEntity>();
        entity->tileWidth  = 3;
        entity->tileHeight = 4;
        entity->SetItem(&item);


        auto tile_proto    = proto::Tile();
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
        playerInv_.SetSelectedItem({&item, 2});

        EXPECT_TRUE(playerPlace_.TryPlaceEntity(worldData_, logicData_, 0, 0));


        // Entity is non-placeable, therefore when clicking on an entity, it will get activated_layer
        playerInv_.DecrementSelectedItem();
        auto* tile = worldData_.GetTile(0, 0);

        EXPECT_TRUE(playerPlace_.TryActivateLayer(worldData_, {2, 3}));
        EXPECT_EQ(playerPlace_.GetActivatedLayer(), &tile->GetLayer(TileLayer::entity));


        // Picking up entity will unset
        playerPlace_.TryPickup(worldData_, logicData_, 0, 1, 1000);
        EXPECT_EQ(playerPlace_.GetActivatedLayer(), nullptr);
    }

    TEST_F(PlayerDataPlacementTest, TryPickupEntity) {
        // Create entity
        auto item = proto::Item();

        auto& entity      = dataManager_.AddProto<proto::ContainerEntity>();
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
            entity.OnBuild(world_data, logicData_, {}, tile.GetLayer(TileLayer::entity), proto::Orientation::up);
        }


        //
        EXPECT_EQ(playerPlace_.GetPickupPercentage(), 0.f);        // Defaults to 0
        playerPlace_.TryPickup(worldData_, logicData_, 0, 2, 990); // Will not attempt to pickup non entity tiles


        // Test pickup
        playerPlace_.TryPickup(worldData_, logicData_, 0, 0, 30);
        EXPECT_EQ(playerPlace_.GetPickupPercentage(), 0.5f); // 50% picked up 30 ticks out of 60
        EXPECT_EQ(tile.GetEntityPrototype(),
                  &entity); // Not picked up yet - 10 more ticks needed to reach 1 second


        playerPlace_.TryPickup(worldData_, logicData_, 1, 0, 30); // Selecting different tile will reset pickup counter
        EXPECT_EQ(tile2.GetEntityPrototype(),
                  &entity); // Not picked up yet - 50 more to 1 second since counter reset

        playerPlace_.TryPickup(worldData_, logicData_, 0, 0, 50);
        playerPlace_.TryPickup(worldData_, logicData_, 0, 0, 10);
        EXPECT_EQ(tile.GetEntityPrototype(),
                  nullptr); // Picked up, item given to inventory

        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 1);

        // Unique data for layer should have been deleted
        EXPECT_EQ(tile.GetLayer(TileLayer::entity).GetUniqueData(), nullptr);
    }

    TEST_F(PlayerDataPlacementTest, TryPickupResource) {
        // Create resource entity
        auto item = proto::Item();

        auto& entity      = dataManager_.AddProto<proto::ResourceEntity>();
        entity.pickupTime = 3.f;
        entity.SetItem(&item);

        // Create world with the resource entity at 0, 0
        worldData_.EmplaceChunk(0, 0);

        auto& tile  = *worldData_.GetTile(0, 0);
        auto& tile2 = *worldData_.GetTile(1, 0);

        tile.SetEntityPrototype(&entity, TileLayer::resource);

        // Holds the resources available at the tile, should be decremented when extracted
        auto* resource_data = tile.GetLayer(TileLayer::resource).MakeUniqueData<proto::ResourceEntityData>(2);


        //
        playerPlace_.TryPickup(worldData_, logicData_, 0, 0, 180);
        // Resource entity should only become nullptr after all the resources are extracted
        EXPECT_EQ(tile.GetEntityPrototype(TileLayer::resource), &entity);

        EXPECT_EQ(resource_data->resourceAmount, 1);

        EXPECT_EQ(playerInv_.inventory[0].item, &item); // Gave 1 resource to player
        EXPECT_EQ(playerInv_.inventory[0].count, 1);


        // All resources extracted from resource entity, should now become nullptr
        playerPlace_.TryPickup(worldData_, logicData_, 0, 0, 60);
        playerPlace_.TryPickup(worldData_, logicData_, 0, 0, 60);
        playerPlace_.TryPickup(worldData_, logicData_, 0, 0, 60);
        EXPECT_EQ(tile2.GetEntityPrototype(TileLayer::resource),
                  nullptr); // Picked up, item given to inventory

        // Resource_data should be deleted

        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 2); // Player has 2 of resource
    }

    TEST_F(PlayerDataPlacementTest, TryPickupLayered) {
        auto item = proto::Item();
        // Create world with the resource entity at 0, 0
        worldData_.EmplaceChunk(0, 0);
        auto& tile = *worldData_.GetTile(0, 0);


        // Resource entity
        auto& resource_entity      = dataManager_.AddProto<proto::ResourceEntity>();
        resource_entity.pickupTime = 3.f;
        resource_entity.SetItem(&item);


        tile.SetEntityPrototype(&resource_entity, TileLayer::resource);

        // Holds the resources available at the tile, should be decremented when extracted
        auto* resource_data = tile.GetLayer(TileLayer::resource).MakeUniqueData<proto::ResourceEntityData>(2);


        // Other entity (e.g Container_entity)
        auto& container_entity      = dataManager_.AddProto<proto::ContainerEntity>();
        container_entity.pickupTime = 1.f;
        container_entity.SetItem(&item);


        tile.SetEntityPrototype(&container_entity);

        //
        playerPlace_.TryPickup(worldData_, logicData_, 0, 0, 60); // Container entity takes priority
        EXPECT_EQ(tile.GetEntityPrototype(), nullptr);            // Picked up, item given to inventory


        // Now that container entity is picked up, resource entity is next
        playerPlace_.TryPickup(worldData_, logicData_, 0, 0, 60);
        playerPlace_.TryPickup(worldData_, logicData_, 0, 0, 60);
        EXPECT_EQ(resource_data->resourceAmount, 2); // Not picked up, still 60 more ticks required

        playerPlace_.TryPickup(worldData_, logicData_, 0, 0, 60);
        EXPECT_EQ(resource_data->resourceAmount, 1); // Picked up
    }


    // ======================================================================


    class MockEntityPlacement final : public TestMockEntity
    {
    public:
        mutable bool buildCalled  = false;
        mutable bool removeCalled = false;

        mutable bool onCanBuildReturn = true;

        mutable std::vector<WorldCoord> emitCoords;
        mutable std::vector<WorldCoord> receiveCoords;

        void OnBuild(WorldData& /*world_data*/,
                     LogicData& /*logic_data*/,
                     const WorldCoord& /*world_coords*/,
                     ChunkTileLayer& /*tile_layer*/,
                     proto::Orientation /*orientation*/) const override {
            buildCalled = true;
        }

        void OnRemove(WorldData& /*world_data*/,
                      LogicData& /*logic_data*/,
                      const WorldCoord& /*world_coords*/,
                      ChunkTileLayer& /*tile_layer*/) const override {
            removeCalled = true;
        }


        J_NODISCARD bool OnCanBuild(const WorldData& /*world_data*/,
                                    const WorldCoord& /*world_coords*/) const override {
            return onCanBuildReturn;
        }


        void OnNeighborUpdate(WorldData& /*world_data*/,
                              LogicData& /*logic_data*/,
                              const WorldCoord& emit_world_coords,
                              const WorldCoord& receive_world_coords,
                              proto::Orientation /*emit_orientation*/) const override {
            emitCoords.push_back(emit_world_coords);
            receiveCoords.push_back(receive_world_coords);
        }
    };

    TEST_F(PlayerDataPlacementTest, PlacePickupCallPickupRemoveEvents) {
        class MockUpdateListener : public TestMockEntity
        {
        public:
            mutable WorldCoord emit;
            mutable WorldCoord receive;
            mutable proto::UpdateType type = proto::UpdateType::remove;

            void OnTileUpdate(WorldData& /*world_data*/,
                              const WorldCoord& emit_coords,
                              const WorldCoord& receive_coords,
                              const proto::UpdateType type) const override {
                emit       = emit_coords;
                receive    = receive_coords;
                this->type = type;
            }
        };

        // The world tile must have a tile prototype
        auto tile_proto    = proto::Tile();
        tile_proto.isWater = false;

        worldData_.EmplaceChunk(0, 0);
        worldData_.GetTile(0, 0)->SetTilePrototype(&tile_proto);


        // Create entity
        auto item   = proto::Item{};
        auto entity = MockEntityPlacement{};
        entity.SetItem(&item);


        proto::ItemStack selected_item = {&item, 1};
        playerInv_.SetSelectedItem(selected_item);

        // Update listeners should be dispatched
        MockUpdateListener mock_listener;
        worldData_.updateDispatcher.Register(3, 4, 0, 0, mock_listener);
        // Change to some random data
        mock_listener.emit    = {1, 2};
        mock_listener.receive = {3, 4};

        playerPlace_.TryPlaceEntity(worldData_, logicData_, 0, 0);

        EXPECT_TRUE(entity.buildCalled);
        EXPECT_EQ(mock_listener.emit.x, 0);
        EXPECT_EQ(mock_listener.emit.y, 0);

        EXPECT_EQ(mock_listener.receive.x, 3);
        EXPECT_EQ(mock_listener.receive.y, 4);

        EXPECT_EQ(mock_listener.type, proto::UpdateType::place);


        // Pickup

        // Change to some random data
        mock_listener.emit    = {1, 2};
        mock_listener.receive = {3, 4};

        playerPlace_.TryPickup(worldData_, logicData_, 0, 0, 60);
        EXPECT_TRUE(entity.removeCalled);
        EXPECT_EQ(mock_listener.emit.x, 0);
        EXPECT_EQ(mock_listener.emit.y, 0);

        EXPECT_EQ(mock_listener.receive.x, 3);
        EXPECT_EQ(mock_listener.receive.y, 4);

        EXPECT_EQ(mock_listener.type, proto::UpdateType::remove);
    }

    TEST_F(PlayerDataPlacementTest, TryPlaceCallOnCanBuild) {
        // The world tile must have a tile prototype
        auto tile_proto    = proto::Tile();
        tile_proto.isWater = false;

        worldData_.EmplaceChunk(0, 0);
        auto* tile = worldData_.GetTile(0, 0);
        tile->SetTilePrototype(&tile_proto);


        // Create entity
        auto item = proto::Item{};

        auto entity             = MockEntityPlacement{};
        entity.onCanBuildReturn = false;
        entity.SetItem(&item);


        proto::ItemStack selected_item = {&item, 1};
        playerInv_.SetSelectedItem(selected_item);

        playerPlace_.TryPlaceEntity(worldData_, logicData_, 0, 0);

        // Not placed because onCanBuild returned false
        EXPECT_EQ(tile->GetEntityPrototype(), nullptr);
    }

    TEST_F(PlayerDataPlacementTest, TryPlaceTryRemoveCallOnNeighborUpdate) {
        // Placing or removing an entity should call on_neighbor_update for 10 adjacent tiles in clockwise order from
        // top left

        //     [1] [2]
        // [A] [x] [x] [3]
        // [9] [x] [x] [4]
        // [8] [x] [x] [5]
        //     [7] [6]

        auto tile_proto    = proto::Tile();
        tile_proto.isWater = false;

        auto item = proto::Item{};

        auto entity_proto       = MockEntityPlacement{};
        entity_proto.tileWidth  = 2;
        entity_proto.tileHeight = 3;
        entity_proto.SetItem(&item);

        worldData_.EmplaceChunk(0, 0);

        // Set tiles so entity can be placed on it
        for (int y = 1; y < 4; ++y) {
            for (int x = 1; x < 3; ++x) {
                worldData_.GetTile(x, y)->GetLayer(TileLayer::base).prototypeData = &tile_proto;
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

        proto::ItemStack selected_item = {&item, 1};
        playerInv_.SetSelectedItem(selected_item);

        playerPlace_.TryPlaceEntity(worldData_, logicData_, 1, 1);
        ASSERT_EQ(entity_proto.emitCoords.size(), 10);
        ASSERT_EQ(entity_proto.receiveCoords.size(), 10);


        auto validate_coords =
            [&](const size_t index, const WorldCoord& emit_coords, const WorldCoord& receive_coords) {
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

        playerPlace_.TryPickup(worldData_, logicData_, 2, 3, 9999); // Bottom right corner
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
} // namespace jactorio::game
