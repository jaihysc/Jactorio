// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/player/player.h"

#include "jactorioTests.h"

#include "proto/tile.h"

namespace jactorio::game
{
    class PlayerPlacementTest : public testing::Test
    {
    protected:
        Player player_;
        Player::Inventory& playerInv_   = player_.inventory;
        Player::Placement& playerPlace_ = player_.placement;

        Logic logic_;
        World world_;

        data::PrototypeManager proto_;

        /// Sets the base tile and entity at coord
        void SetEntityCoords(const WorldCoord& coord,
                             const proto::Tile* tile_proto,
                             const proto::Entity* entity_proto) {
            world_.GetTile(coord, TileLayer::base)->SetPrototype(Orientation::up, tile_proto);
            world_.GetTile(coord, TileLayer::entity)->SetPrototype(Orientation::up, entity_proto);
        }
    };

    TEST_F(PlayerPlacementTest, RotatePlacementOrientation) {
        EXPECT_EQ(playerPlace_.orientation, Orientation::up);

        playerPlace_.RotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, Orientation::right);

        playerPlace_.RotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, Orientation::down);

        playerPlace_.RotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, Orientation::left);

        playerPlace_.RotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, Orientation::up);
    }

    TEST_F(PlayerPlacementTest, CounterRotatePlacementOrientation) {
        EXPECT_EQ(playerPlace_.orientation, Orientation::up);

        playerPlace_.CounterRotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, Orientation::left);

        playerPlace_.CounterRotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, Orientation::down);

        playerPlace_.CounterRotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, Orientation::right);

        playerPlace_.CounterRotateOrientation();
        EXPECT_EQ(playerPlace_.orientation, Orientation::up);
    }

    /// Place entity at 0, 0
    TEST_F(PlayerPlacementTest, TryPlaceEntity) {
        // Create entity
        proto::Item item;
        proto::Item item_no_entity; // Does not hold an entity reference

        proto::ContainerEntity entity;
        entity.SetItem(&item);

        proto::ContainerEntity entity2;


        proto::Tile tile_proto;
        tile_proto.isWater = false;

        world_.EmplaceChunk({0, 0});

        auto& tile1_base   = *world_.GetTile({0, 0}, TileLayer::base);
        auto& tile1_entity = *world_.GetTile({0, 0}, TileLayer::entity);

        auto& tile2_base   = *world_.GetTile({1, 0}, TileLayer::base);
        auto& tile2_entity = *world_.GetTile({1, 0}, TileLayer::entity);


        tile1_base.SetPrototype(Orientation::up, &tile_proto);

        tile2_base.SetPrototype(Orientation::up, &tile_proto);
        tile2_entity.SetPrototype(Orientation::up, &entity2);


        // Edge cases
        EXPECT_FALSE(playerPlace_.TryPlaceEntity(world_, logic_, {0, 0})); // Placing with no items selected

        playerInv_.SetSelectedItem({&item_no_entity, 2});

        tile1_entity.SetPrototype(Orientation::up, &entity);

        EXPECT_FALSE(playerPlace_.TryPlaceEntity(world_, logic_, {0, 0})); // Item holds no reference to an entity
        EXPECT_EQ(tile1_entity.GetPrototype(), &entity);                   // Should not delete at this location


        // Placement tests

        // Place at 0, 0
        playerInv_.SetSelectedItem({&item, 2});

        tile1_entity.SetPrototype(nullptr);

        EXPECT_TRUE(playerPlace_.TryPlaceEntity(world_, logic_, {0, 0})); // Place on empty tile 0, 0

        EXPECT_EQ(tile1_entity.GetPrototype(), &entity);
        EXPECT_EQ(playerInv_.GetSelectedItem()->count, 1); // 1 less item

        // The OnBuild() method should get called, creating unique data on the tile which holds the inventory
        EXPECT_NE(tile1_entity.GetUniqueData(), nullptr);


        // Do not place at 1, 0

        // A tile already exists on 1, 0 - Should not override it
        EXPECT_FALSE(playerPlace_.TryPlaceEntity(world_, logic_, {1, 0}));
        EXPECT_EQ(tile2_entity.GetPrototype(), &entity2);
    }

    TEST_F(PlayerPlacementTest, TryActivateTile) {
        // Create entity
        proto::Item item;
        proto::Item item_no_entity; // Does not hold an entity reference

        proto::ContainerEntity entity;
        entity.placeable = true;
        entity.SetItem(&item);


        proto::Tile tile_proto;
        tile_proto.isWater = false;

        // Create world with entity at 0, 0
        world_.EmplaceChunk({0, 0});


        auto* tile_base   = world_.GetTile({0, 0}, TileLayer::base);
        auto* tile_entity = world_.GetTile({0, 0}, TileLayer::entity);

        tile_base->SetPrototype(Orientation::up, &tile_proto);

        // No entity, do not activate
        EXPECT_FALSE(playerPlace_.TryActivateTile(world_, {0, 0}));


        // If selected item's entity is placeable, do not set activated_layer
        tile_entity->SetPrototype(Orientation::up, &entity);

        playerInv_.SetSelectedItem({&item, 2});

        EXPECT_FALSE(playerPlace_.TryPlaceEntity(world_, logic_, {0, 0}));
        EXPECT_EQ(playerPlace_.GetActivatedTile(), nullptr);


        // Clicking on an entity with no placeable items selected will set activated_layer
        playerInv_.SetSelectedItem({&item_no_entity, 2});

        EXPECT_TRUE(playerPlace_.TryActivateTile(world_, {0, 0}));
        EXPECT_EQ(playerPlace_.GetActivatedTile(), tile_entity);


        // Clicking again will NOT unset
        EXPECT_TRUE(playerPlace_.TryActivateTile(world_, {0, 0}));
        EXPECT_EQ(playerPlace_.GetActivatedTile(), tile_entity);


        // Activated tile can be set to nullptr to deactivate
        playerPlace_.SetActivatedTile(nullptr);
        EXPECT_EQ(playerPlace_.GetActivatedTile(), nullptr);
    }

    /// Picking up an entity wil unset activated tile if activated tile was the entity
    TEST_F(PlayerPlacementTest, TryPickupEntityDeactivateTile) {

        // Create entity
        proto::Item item;

        proto::ContainerEntity entity;
        entity.SetWidth(3);
        entity.SetHeight(4);
        entity.SetItem(&item);


        proto::Tile tile_proto;
        tile_proto.isWater = false;

        // Create world with entity at 0, 0
        world_.EmplaceChunk({0, 0});


        for (WorldCoordAxis y = 0; y < entity.GetHeight(Orientation::up); ++y) {
            for (WorldCoordAxis x = 0; x < entity.GetWidth(Orientation::up); ++x) {
                auto* tile = world_.GetTile({x, y}, TileLayer::base);
                tile->SetPrototype(Orientation::up, &tile_proto);
            }
        }


        // Place entity
        playerInv_.SetSelectedItem({&item, 2});

        EXPECT_TRUE(playerPlace_.TryPlaceEntity(world_, logic_, {0, 0}));


        // Entity is non-placeable, therefore when clicking on an entity, it will get activated_layer
        playerInv_.DecrementSelectedItem();

        EXPECT_TRUE(playerPlace_.TryActivateTile(world_, {2, 3}));
        EXPECT_EQ(playerPlace_.GetActivatedTile(), world_.GetTile({0, 0}, TileLayer::entity));


        // Picking up entity will unset
        playerPlace_.TryPickup(world_, logic_, {0, 1}, 1000);
        EXPECT_EQ(playerPlace_.GetActivatedTile(), nullptr);
    }

    TEST_F(PlayerPlacementTest, TryPickupEntity) {
        // Create entity
        proto::Item item;

        auto& entity      = proto_.Make<proto::ContainerEntity>();
        entity.pickupTime = 1.f;
        entity.SetItem(&item);

        // Create world with entity at 0, 0
        world_.EmplaceChunk({0, 0});

        auto* tile  = world_.GetTile({0, 0}, TileLayer::entity);
        auto* tile2 = world_.GetTile({1, 0}, TileLayer::entity);

        tile->SetPrototype(Orientation::up, &entity);
        tile2->SetPrototype(Orientation::up, &entity);


        // Create unique data by calling build event for prototype with tile
        entity.OnBuild(world_, logic_, {0, 0}, TileLayer::entity, Orientation::up);


        //
        EXPECT_EQ(playerPlace_.GetPickupPercentage(), 0.f);  // Defaults to 0
        playerPlace_.TryPickup(world_, logic_, {0, 2}, 990); // Will not attempt to pickup non entity tiles


        // Test pickup
        playerPlace_.TryPickup(world_, logic_, {0, 0}, 30);
        EXPECT_EQ(playerPlace_.GetPickupPercentage(), 0.5f); // 50% picked up 30 ticks out of 60
        EXPECT_EQ(tile->GetPrototype(), &entity); // Not picked up yet - 10 more ticks needed to reach 1 second


        playerPlace_.TryPickup(world_, logic_, {1, 0}, 30); // Selecting different tile will reset pickup counter
        EXPECT_EQ(tile2->GetPrototype(), &entity); // Not picked up yet - 50 more to 1 second since counter reset

        playerPlace_.TryPickup(world_, logic_, {0, 0}, 50);
        playerPlace_.TryPickup(world_, logic_, {0, 0}, 10);
        EXPECT_EQ(tile->GetPrototype(), nullptr); // Picked up, item given to inventory

        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 1);

        // Unique data for tile should have been deleted
        EXPECT_EQ(tile->GetUniqueData(), nullptr);
    }

    TEST_F(PlayerPlacementTest, TryPickupResource) {
        // Create resource entity
        proto::Item item;

        auto& entity      = proto_.Make<proto::ResourceEntity>();
        entity.pickupTime = 3.f;
        entity.SetItem(&item);

        // Create world with the resource entity at 0, 0
        world_.EmplaceChunk({0, 0});

        auto* tile  = world_.GetTile({0, 0}, TileLayer::resource);
        auto* tile2 = world_.GetTile({1, 0}, TileLayer::resource);

        tile->SetPrototype(Orientation::up, &entity);

        // Holds the resources available at the tile, should be decremented when extracted
        auto& resource_data = tile->MakeUniqueData<proto::ResourceEntityData>(2);


        //
        playerPlace_.TryPickup(world_, logic_, {0, 0}, 180);
        // Resource entity should only become nullptr after all the resources are extracted
        EXPECT_EQ(tile->GetPrototype(), &entity);

        EXPECT_EQ(resource_data.resourceAmount, 1);

        EXPECT_EQ(playerInv_.inventory[0].item, &item); // Gave 1 resource to player
        EXPECT_EQ(playerInv_.inventory[0].count, 1);


        // All resources extracted from resource entity, should now become nullptr
        playerPlace_.TryPickup(world_, logic_, {0, 0}, 60);
        playerPlace_.TryPickup(world_, logic_, {0, 0}, 60);
        playerPlace_.TryPickup(world_, logic_, {0, 0}, 60);
        EXPECT_EQ(tile2->GetPrototype(), nullptr); // Picked up, item given to inventory

        // Resource_data should be deleted

        EXPECT_EQ(playerInv_.inventory[0].item, &item);
        EXPECT_EQ(playerInv_.inventory[0].count, 2); // Player has 2 of resource
    }

    TEST_F(PlayerPlacementTest, TryPickupLayered) {
        proto::Item item;
        // Create world with the resource entity at 0, 0
        world_.EmplaceChunk({0, 0});
        auto* tile_resource = world_.GetTile({0, 0}, TileLayer::resource);
        auto* tile_entity   = world_.GetTile({0, 0}, TileLayer::entity);


        // Resource entity
        auto& resource_entity      = proto_.Make<proto::ResourceEntity>();
        resource_entity.pickupTime = 3.f;
        resource_entity.SetItem(&item);


        tile_resource->SetPrototype(Orientation::up, &resource_entity);

        // Holds the resources available at the tile, should be decremented when extracted
        auto& resource_data = tile_resource->MakeUniqueData<proto::ResourceEntityData>(2);


        // Other entity (e.g Container_entity)
        auto& container_entity      = proto_.Make<proto::ContainerEntity>();
        container_entity.pickupTime = 1.f;
        container_entity.SetItem(&item);


        tile_entity->SetPrototype(Orientation::up, &container_entity);

        //
        playerPlace_.TryPickup(world_, logic_, {0, 0}, 60); // Container entity takes priority
        EXPECT_EQ(tile_entity->GetPrototype(), nullptr);    // Picked up, item given to inventory


        // Now that container entity is picked up, resource entity is next
        playerPlace_.TryPickup(world_, logic_, {0, 0}, 60);
        playerPlace_.TryPickup(world_, logic_, {0, 0}, 60);
        EXPECT_EQ(resource_data.resourceAmount, 2); // Not picked up, still 60 more ticks required

        playerPlace_.TryPickup(world_, logic_, {0, 0}, 60);
        EXPECT_EQ(resource_data.resourceAmount, 1); // Picked up
    }

    /// Picking up an entity which is rotated (treated as 2x1 instead of 1x2)
    TEST_F(PlayerPlacementTest, TryPickupRotated) {
        world_.EmplaceChunk({0, 0});

        proto::Item item;
        proto::ContainerEntity container;
        container.SetItem(&item);

        container.SetDimension({1, 2});
        TestSetupContainer(world_, {1, 1}, Orientation::right, container); // Tiles {1, 1}, {2, 1}

        playerPlace_.TryPickup(world_, logic_, {2, 1}, 9999);

        EXPECT_EQ(world_.GetTile({1, 1}, TileLayer::entity)->GetPrototype(), nullptr); // Left container
        EXPECT_EQ(world_.GetTile({2, 1}, TileLayer::entity)->GetPrototype(), nullptr); // Right container
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

        void OnBuild(World& /*world*/,
                     Logic& /*logic*/,
                     const WorldCoord& /*coord*/,
                     TileLayer /*tlayer*/,
                     Orientation /*orientation*/) const override {
            buildCalled = true;
        }

        void OnRemove(World& /*world*/,
                      Logic& /*logic*/,
                      const WorldCoord& /*coord*/,
                      TileLayer /*tlayer*/) const override {
            removeCalled = true;
        }


        J_NODISCARD bool OnCanBuild(const World& /*world*/,
                                    const WorldCoord& /*coord*/,
                                    const Orientation /*orien*/) const override {
            return onCanBuildReturn;
        }


        void OnNeighborUpdate(World& /*world*/,
                              Logic& /*logic*/,
                              const WorldCoord& emit_coord,
                              const WorldCoord& receive_coord,
                              Orientation /*emit_orientation*/) const override {
            emitCoords.push_back(emit_coord);
            receiveCoords.push_back(receive_coord);
        }
    };

    TEST_F(PlayerPlacementTest, PlacePickupCallPickupRemoveEvents) {
        class MockUpdateListener : public TestMockEntity
        {
        public:
            mutable WorldCoord emit;
            mutable WorldCoord receive;
            mutable proto::UpdateType type = proto::UpdateType::remove;

            void OnTileUpdate(World& /*world*/,
                              const WorldCoord& emit_coords,
                              const WorldCoord& receive_coords,
                              const proto::UpdateType type) const override {
                emit       = emit_coords;
                receive    = receive_coords;
                this->type = type;
            }
        };

        // The world tile must have a tile prototype
        proto::Tile tile_proto;
        tile_proto.isWater = false;

        world_.EmplaceChunk({0, 0});
        world_.GetTile({0, 0}, TileLayer::base)->SetPrototype(Orientation::up, &tile_proto);


        // Create entity
        proto::Item item;
        MockEntityPlacement entity;
        entity.SetItem(&item);


        ItemStack selected_item = {&item, 1};
        playerInv_.SetSelectedItem(selected_item);

        // Update listeners should be dispatched
        MockUpdateListener mock_listener;
        world_.updateDispatcher.Register({3, 4}, {0, 0}, mock_listener);
        // Change to some random data
        mock_listener.emit    = {1, 2};
        mock_listener.receive = {3, 4};

        playerPlace_.TryPlaceEntity(world_, logic_, {0, 0});

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

        playerPlace_.TryPickup(world_, logic_, {0, 0}, 60);
        EXPECT_TRUE(entity.removeCalled);
        EXPECT_EQ(mock_listener.emit.x, 0);
        EXPECT_EQ(mock_listener.emit.y, 0);

        EXPECT_EQ(mock_listener.receive.x, 3);
        EXPECT_EQ(mock_listener.receive.y, 4);

        EXPECT_EQ(mock_listener.type, proto::UpdateType::remove);
    }

    TEST_F(PlayerPlacementTest, TryPlaceCallOnCanBuild) {
        // The world tile must have a tile prototype
        proto::Tile tile_proto;
        tile_proto.isWater = false;

        world_.EmplaceChunk({0, 0});

        world_.GetTile({0, 0}, TileLayer::base)->SetPrototype(Orientation::up, &tile_proto);


        // Create entity
        proto::Item item;

        MockEntityPlacement entity;
        entity.onCanBuildReturn = false;
        entity.SetItem(&item);


        ItemStack selected_item = {&item, 1};
        playerInv_.SetSelectedItem(selected_item);

        playerPlace_.TryPlaceEntity(world_, logic_, {0, 0});

        // Not placed because onCanBuild returned false
        EXPECT_EQ(world_.GetTile({0, 0}, TileLayer::entity)->GetPrototype(), nullptr);
    }

    TEST_F(PlayerPlacementTest, TryPlaceTryRemoveCallOnNeighborUpdate) {
        // Placing or removing an entity should call on_neighbor_update for 10 adjacent tiles in clockwise order
        // from top left

        //     [1] [2]
        // [A] [x] [x] [3]
        // [9] [x] [x] [4]
        // [8] [x] [x] [5]
        //     [7] [6]

        proto::Tile tile_proto;
        tile_proto.isWater = false;

        proto::Item item;

        MockEntityPlacement entity_proto;
        entity_proto.SetWidth(2);
        entity_proto.SetHeight(3);
        entity_proto.SetItem(&item);

        world_.EmplaceChunk({0, 0});

        // Set tiles so entity can be placed on it
        for (int y = 1; y < 4; ++y) {
            for (int x = 1; x < 3; ++x) {
                world_.GetTile({x, y}, TileLayer::base)->SetPrototype(Orientation::up, &tile_proto);
            }
        }

        // Set entity around border
        for (int x = 1; x <= 2; ++x) {
            SetEntityCoords({x, 0}, &tile_proto, &entity_proto);
        }
        for (int y = 1; y <= 3; ++y) {
            SetEntityCoords({3, y}, &tile_proto, &entity_proto);
        }
        for (int x = 2; x >= 1; --x) {
            SetEntityCoords({x, 4}, &tile_proto, &entity_proto);
        }
        for (int y = 3; y >= 1; --y) {
            SetEntityCoords({0, y}, &tile_proto, &entity_proto);
        }

        // ======================================================================
        // Place

        ItemStack selected_item = {&item, 1};
        playerInv_.SetSelectedItem(selected_item);

        playerPlace_.TryPlaceEntity(world_, logic_, {1, 1});
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

        playerPlace_.TryPickup(world_, logic_, {2, 3}, 9999); // Bottom right corner
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
