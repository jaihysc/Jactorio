// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/player/player.h"

#include "jactorioTests.h"

#include "proto/tile.h"

namespace jactorio::game
{
    TEST(PlayerPlacement, RotatePlacementOrientation) {
        Player player;
        auto& place = player.placement;

        EXPECT_EQ(place.orientation, Orientation::up);

        place.RotateOrientation();
        EXPECT_EQ(place.orientation, Orientation::right);

        place.RotateOrientation();
        EXPECT_EQ(place.orientation, Orientation::down);

        place.RotateOrientation();
        EXPECT_EQ(place.orientation, Orientation::left);

        place.RotateOrientation();
        EXPECT_EQ(place.orientation, Orientation::up);
    }

    TEST(PlayerPlacement, CounterRotatePlacementOrientation) {
        Player player;
        auto& place = player.placement;

        EXPECT_EQ(place.orientation, Orientation::up);

        place.CounterRotateOrientation();
        EXPECT_EQ(place.orientation, Orientation::left);

        place.CounterRotateOrientation();
        EXPECT_EQ(place.orientation, Orientation::down);

        place.CounterRotateOrientation();
        EXPECT_EQ(place.orientation, Orientation::right);

        place.CounterRotateOrientation();
        EXPECT_EQ(place.orientation, Orientation::up);
    }

    TEST(PlayerPlacement, TryActivateTile) {
        Player player;
        Logic logic;
        World world;

        // Create entity
        proto::Item item;
        proto::Item item_no_entity; // Does not hold an entity reference

        proto::ContainerEntity entity;
        entity.placeable = true;
        entity.SetItem(&item);


        proto::Tile tile_proto;
        tile_proto.isWater = false;

        // Create world with entity at 0, 0
        world.EmplaceChunk({0, 0});


        auto* tile_base   = world.GetTile({3, 4}, TileLayer::base);
        auto* tile_entity = world.GetTile({3, 4}, TileLayer::entity);
        tile_base->SetPrototype(Orientation::up, &tile_proto);

        // No entity, do not activate
        EXPECT_FALSE(player.placement.TryActivateTile(world, {3, 4}));

        // If selected item's entity is placeable, DO set activated_layer
        tile_entity->SetPrototype(Orientation::up, &entity);
        player.inventory.SetSelectedItem({&item, 2});

        EXPECT_FALSE(player.placement.TryPlaceEntity(world, logic, {3, 4}));
        auto [activated_tile, activated_coord] = player.placement.GetActivatedTile();
        EXPECT_EQ(activated_tile, nullptr);

        // Clicking on an entity with no placeable items selected will set activated_layer
        player.inventory.SetSelectedItem({&item_no_entity, 2});

        EXPECT_TRUE(player.placement.TryActivateTile(world, {3, 4}));
        std::tie(activated_tile, activated_coord) = player.placement.GetActivatedTile();
        EXPECT_EQ(activated_tile, tile_entity);
        EXPECT_EQ(activated_coord, WorldCoord(3, 4));

        // Clicking again will NOT unset
        EXPECT_TRUE(player.placement.TryActivateTile(world, {3, 4}));
        std::tie(activated_tile, activated_coord) = player.placement.GetActivatedTile();
        EXPECT_EQ(activated_tile, tile_entity);
        EXPECT_EQ(activated_coord, WorldCoord(3, 4));

        // Activated tile can be deactivated
        player.placement.DeactivateTile();
        std::tie(activated_tile, activated_coord) = player.placement.GetActivatedTile();
        EXPECT_EQ(activated_tile, nullptr);
    }

    // ======================================================================

    class PlayerPlacementTest : public testing::Test
    {
    protected:
        /// Fills chunk with buildable tile
        /// Sets up item and container_
        void SetUp() override {
            auto& chunk = world_.EmplaceChunk({0, 0});

            container_.SetItem(&item_);
            container_.sprite = &sprite_; // Conveyor utility requires tex coord id when setting up conveyor

            tile_.isWater = false;

            for (auto& tile : chunk.Tiles(TileLayer::base)) {
                tile.SetPrototype(Orientation::up, &tile_);
            }
        }

        Player player_;
        Logic logic_;
        World world_;

        /// Tile making up the chunk 0, 0
        proto::Tile tile_;

        /// Item for container_
        proto::Item item_;
        proto::ContainerEntity container_;
        proto::Sprite sprite_;
    };


    /// Valid placement at tile {0, 0}
    TEST_F(PlayerPlacementTest, PlaceValid) {
        player_.inventory.SetSelectedItem({&item_, 2});
        EXPECT_TRUE(player_.placement.TryPlaceEntity(world_, logic_, {0, 0}));

        EXPECT_EQ(world_.GetTile({0, 0}, TileLayer::entity)->GetPrototype(), &container_);

        // 1 less item
        EXPECT_EQ(*player_.inventory.GetSelectedItem(), ItemStack({&item_, 1}));
    }

    /// An entity already exists at {1, 2}, building another entity that overlaps it is invalid
    TEST_F(PlayerPlacementTest, PlaceTileAlreadyHasEntity) {
        container_.SetDimension({2, 3});

        player_.inventory.SetSelectedItem({&item_, 2});
        EXPECT_TRUE(player_.placement.TryPlaceEntity(world_, logic_, {1, 2}));

        // A tile already exists, Should not override it
        EXPECT_FALSE(player_.placement.TryPlaceEntity(world_, logic_, {0, 0}));

        EXPECT_EQ(world_.GetTile({0, 0}, TileLayer::entity)->GetPrototype(), nullptr);
        EXPECT_EQ(world_.GetTile({1, 2}, TileLayer::entity)->GetPrototype(), &container_);

        EXPECT_EQ(*player_.inventory.GetSelectedItem(), ItemStack({&item_, 1}));
    }

    /// No item selected, nothing to place
    TEST_F(PlayerPlacementTest, PlaceNoItemSelected) {
        EXPECT_FALSE(player_.placement.TryPlaceEntity(world_, logic_, {3, 11}));
        EXPECT_EQ(world_.GetTile({3, 11}, TileLayer::entity)->GetPrototype(), nullptr);
    }

    /// No entity associated with item selected, nothing to place
    TEST_F(PlayerPlacementTest, PlaceItemWithNoEntitySelected) {
        item_.entityPrototype = nullptr;
        player_.inventory.SetSelectedItem({&item_, 1});

        EXPECT_FALSE(player_.placement.TryPlaceEntity(world_, logic_, {3, 11}));
        EXPECT_EQ(world_.GetTile({3, 11}, TileLayer::entity)->GetPrototype(), nullptr);

        EXPECT_EQ(*player_.inventory.GetSelectedItem(), ItemStack({&item_, 1}));
    }

    /// If entity can be built at location,
    /// OnCanBuild can override to not build at location
    TEST_F(PlayerPlacementTest, PlaceCallOnCanBuild) {
        class Mock : public TestMockEntity
        {
        public:
            bool OnCanBuild(const World& /*world*/,
                            const WorldCoord& /*coord*/,
                            const Orientation /*orien*/) const override {
                return onCanBuildReturn;
            }

            mutable bool onCanBuildReturn = true;
        } mock;
        mock.onCanBuildReturn = false;
        mock.SetItem(&item_);

        player_.inventory.SetSelectedItem({&item_, 1});
        player_.placement.TryPlaceEntity(world_, logic_, {0, 0});

        // Not placed because onCanBuild returned false
        EXPECT_EQ(world_.GetTile({0, 0}, TileLayer::entity)->GetPrototype(), nullptr);
    }


    // Remove

    /// Picking up an entity wil unset activated tile if activated tile was the entity
    TEST_F(PlayerPlacementTest, TryPickupEntityDeactivateTile) {
        container_.SetDimension({3, 4});

        player_.inventory.SetSelectedItem({&item_, 2});
        EXPECT_TRUE(player_.placement.TryPlaceEntity(world_, logic_, {0, 0}));

        // Bit of a hack to get rid of the selected item, since it has internal members, cannot be set to {nullptr, 0}
        player_.inventory.DecrementSelectedItem();

        EXPECT_TRUE(player_.placement.TryActivateTile(world_, {2, 3}));
        EXPECT_EQ(player_.placement.GetActivatedTile().first, world_.GetTile({0, 0}, TileLayer::entity));


        player_.placement.TryPickup(world_, logic_, {1, 2});
        EXPECT_EQ(player_.placement.GetActivatedTile().first, nullptr);
    }

    TEST_F(PlayerPlacementTest, PickupEntity) {
        auto* tile = world_.GetTile({0, 0}, TileLayer::entity);

        player_.inventory.SetSelectedItem({&item_, 1});
        EXPECT_TRUE(player_.placement.TryPlaceEntity(world_, logic_, {0, 0}));


        player_.placement.TryPickup(world_, logic_, {0, 2}); // Will not attempt to pickup non entity tiles
        EXPECT_NE(tile->GetPrototype(), nullptr);

        player_.placement.TryPickup(world_, logic_, {0, 0});
        EXPECT_EQ(tile->GetPrototype(), nullptr); // Picked up, item given to inventory

        EXPECT_EQ(player_.inventory.inventory[0].item, &item_);
        EXPECT_EQ(player_.inventory.inventory[0].count, 1);
    }

    TEST_F(PlayerPlacementTest, PickupResource) {
        proto::Item item;
        proto::ResourceEntity resource;
        resource.pickupTime = 3.f;
        resource.SetItem(&item);

        auto* tile = world_.GetTile({0, 0}, TileLayer::resource);
        tile->SetPrototype(Orientation::up, &resource);
        auto& resource_data = tile->MakeUniqueData<proto::ResourceEntityData>(2);
        world_.SetTexCoordId({0, 0}, TileLayer::resource, 1234);


        player_.placement.TryPickup(world_, logic_, {0, 0}, 180);
        // Resource entity should only become nullptr after all the resources are extracted
        EXPECT_EQ(tile->GetPrototype(), &resource);
        EXPECT_EQ(resource_data.resourceAmount, 1);

        EXPECT_EQ(player_.inventory.inventory[0].item, &item); // Gave 1 resource to player
        EXPECT_EQ(player_.inventory.inventory[0].count, 1);


        // All resources extracted from resource entity, should now become nullptr
        player_.placement.TryPickup(world_, logic_, {0, 0}, 60);
        player_.placement.TryPickup(world_, logic_, {0, 0}, 60);
        EXPECT_EQ(tile->GetPrototype(), &resource);

        player_.placement.TryPickup(world_, logic_, {0, 0}, 60);
        EXPECT_EQ(tile->GetPrototype(), nullptr); // Picked up, item given to inventory
        EXPECT_EQ(world_.GetTexCoordId({0, 0}, TileLayer::resource), 0);

        EXPECT_EQ(player_.inventory.inventory[0].item, &item);
        EXPECT_EQ(player_.inventory.inventory[0].count, 2);
    }

    /// For a tile, pickup entity first, then resource
    TEST_F(PlayerPlacementTest, PickupLayerOrder) {
        auto* tile_resource = world_.GetTile({0, 0}, TileLayer::resource);
        auto* tile_entity   = world_.GetTile({0, 0}, TileLayer::entity);

        // Resource entity
        proto::Item resource_item;
        proto::ResourceEntity resource_entity;
        resource_entity.pickupTime = 3.f;
        resource_entity.SetItem(&resource_item);

        tile_resource->SetPrototype(Orientation::up, &resource_entity);
        tile_resource->MakeUniqueData<proto::ResourceEntityData>(1);

        // Container entity
        proto::Item container_item;
        proto::ContainerEntity container_entity;
        container_entity.SetItem(&container_item);

        tile_entity->SetPrototype(Orientation::up, &container_entity);


        player_.placement.TryPickup(world_, logic_, {0, 0}); // Container entity takes priority
        EXPECT_EQ(tile_entity->GetPrototype(), nullptr);

        player_.placement.TryPickup(world_, logic_, {0, 0}, 180);
        EXPECT_EQ(tile_resource->GetPrototype(), nullptr);
    }

    /// Picking up an entity which is rotated (treated as 2x1 instead of 1x2)
    TEST_F(PlayerPlacementTest, PickupRotatedEntity) {
        container_.SetDimension({1, 2});
        TestSetupContainer(world_, {1, 1}, Orientation::right, container_); // Tiles {1, 1}, {2, 1}

        player_.placement.TryPickup(world_, logic_, {2, 1});

        EXPECT_EQ(world_.GetTile({1, 1}, TileLayer::entity)->GetPrototype(), nullptr); // Left container
        EXPECT_EQ(world_.GetTile({2, 1}, TileLayer::entity)->GetPrototype(), nullptr); // Right container
    }

    TEST_F(PlayerPlacementTest, PlacePickupCallEvents) {
        class MockEntity : public TestMockEntity
        {
        public:
            void OnBuild(World& /*world*/,
                         Logic& /*logic*/,
                         const WorldCoord& /*coord*/,
                         Orientation /*orientation*/) const override {
                onBuildCalled = true;
            }

            void OnRemove(World& /*world*/, Logic& /*logic*/, const WorldCoord& /*coord*/) const override {
                onRemoveCalled = true;
            }

            void OnTileUpdate(World& /*world*/,
                              const WorldCoord& emit_coord,
                              const WorldCoord& receive_coord,
                              const proto::UpdateType type) const override {
                EXPECT_EQ(emit_coord, WorldCoord(3, 3));
                EXPECT_EQ(receive_coord, WorldCoord(5, 6));
                updateType = type;
                onUpdateCalled++;
            }

            mutable bool onBuildCalled           = false;
            mutable bool onRemoveCalled          = false;
            mutable int onUpdateCalled           = 0;
            mutable proto::UpdateType updateType = proto::UpdateType::place;
        } mock;
        proto::Sprite sprite;
        mock.sprite = &sprite;

        mock.SetDimension({3, 2});
        mock.SetItem(&item_);


        world_.updateDispatcher.Register({5, 6}, {3, 3}, mock);

        player_.inventory.SetSelectedItem({&item_, 2});
        EXPECT_TRUE(player_.placement.TryPlaceEntity(world_, logic_, {1, 2}));

        EXPECT_TRUE(mock.onBuildCalled);
        EXPECT_EQ(mock.updateType, proto::UpdateType::place);
        EXPECT_EQ(mock.onUpdateCalled, 1);

        // Pickup
        player_.placement.TryPickup(world_, logic_, {1, 2});
        EXPECT_TRUE(mock.onRemoveCalled);
        EXPECT_EQ(mock.updateType, proto::UpdateType::remove);
        EXPECT_EQ(mock.onUpdateCalled, 2);
    }

    TEST_F(PlayerPlacementTest, PlacePickupCallOnNeighborUpdate) {
        // Placing or removing an entity should call on_neighbor_update for 10 adjacent tiles in clockwise order
        // from top left

        //     [1] [2]
        // [A] [x] [x] [3]
        // [9] [x] [x] [4]
        // [8] [x] [x] [5]
        //     [7] [6]

        class Mock final : public TestMockEntity
        {
        public:
            mutable std::vector<WorldCoord> emitCoords;
            mutable std::vector<WorldCoord> receiveCoords;

            void OnNeighborUpdate(World& /*world*/,
                                  Logic& /*logic*/,
                                  const WorldCoord& emit_coord,
                                  const WorldCoord& receive_coord,
                                  Orientation /*emit_orientation*/) const override {
                emitCoords.push_back(emit_coord);
                receiveCoords.push_back(receive_coord);
            }
        } mock;
        proto::Sprite sprite;
        mock.sprite = &sprite;

        proto::Item item;
        mock.SetDimension({2, 3});
        mock.SetItem(&item);

        // Set listeners around border
        for (int x = 1; x <= 2; ++x) {
            world_.GetTile({x, 0}, TileLayer::entity)->SetPrototype(Orientation::up, &mock);
        }
        for (int y = 1; y <= 3; ++y) {
            world_.GetTile({3, y}, TileLayer::entity)->SetPrototype(Orientation::up, &mock);
        }
        for (int x = 2; x >= 1; --x) {
            world_.GetTile({x, 4}, TileLayer::entity)->SetPrototype(Orientation::up, &mock);
        }
        for (int y = 3; y >= 1; --y) {
            world_.GetTile({0, y}, TileLayer::entity)->SetPrototype(Orientation::up, &mock);
        }

        // Place
        player_.inventory.SetSelectedItem({&item, 1});

        player_.placement.TryPlaceEntity(world_, logic_, {1, 1});
        ASSERT_EQ(mock.emitCoords.size(), 10);
        ASSERT_EQ(mock.receiveCoords.size(), 10);


        auto validate_coords =
            [&](const size_t index, const WorldCoord& emit_coords, const WorldCoord& receive_coords) {
                EXPECT_EQ(mock.emitCoords[index], emit_coords);
                EXPECT_EQ(mock.receiveCoords[index], receive_coords);
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

        // Remove
        player_.placement.TryPickup(world_, logic_, {2, 3}); // Bottom right corner
        ASSERT_EQ(mock.emitCoords.size(), 20);
        ASSERT_EQ(mock.receiveCoords.size(), 20);

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
