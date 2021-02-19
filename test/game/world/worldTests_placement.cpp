// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/world/world.h"

#include <memory>

#include "proto/container_entity.h"
#include "proto/tile.h"

namespace jactorio::game
{
    // NOTE
    // !Checking if the selected tile is too far from the player is not done in these tests!
    //

    // Creates world and generates a test world within it
    class WorldPlacementTest : public testing::Test
    {
    public:
        static constexpr auto kChunkWidth = World::kChunkWidth;

    private:
        static void GenerateTestWorld(World& world, proto::Tile* water_tile, proto::Tile* land_tile) {
            // Generates a quarter chunk on which to test entity placement (16 x 16)
            // Following indices begin at 0:
            // Row of water: [1, 4, 8]
            // Column of water: [1, 4, 8]

            /* # is land
             * - is water
               #-##-###-#######
               ----------------
               #-##-###-#######
               #-##-###-#######
               ----------------
               #-##-###-#######
               #-##-###-#######
               #-##-###-#######
               ----------------
               #-##-###-#######
               #-##-###-#######
               #-##-###-#######
               #-##-###-#######
               #-##-###-#######
               #-##-###-#######
               #-##-###-#######
             */

            water_tile->isWater = true;
            land_tile->isWater  = false;


            auto& chunk = world.EmplaceChunk({0, 0});

            for (ChunkTileCoordAxis y = 0; y < kChunkWidth; ++y) {
                bool y_water = false;

                // Water at Y index 1, 4, 8
                if (y == 1 || y == 4 || y == 8)
                    y_water = true;

                for (ChunkTileCoordAxis x = 0; x < kChunkWidth; ++x) {
                    bool x_water = false;

                    // Water at X index 1, 4, 8
                    if (x == 1 || x == 4 || x == 8)
                        x_water = true;

                    auto* tile_ptr = land_tile;
                    if (x_water || y_water) {
                        tile_ptr = water_tile;
                    }

                    chunk.GetCTile({x, y}, TileLayer::base).SetPrototype(Orientation::up, tile_ptr);
                }
            }
        }

    protected:
        std::unique_ptr<proto::Tile> waterTile_ = std::make_unique<proto::Tile>();
        std::unique_ptr<proto::Tile> landTile_  = std::make_unique<proto::Tile>();

        World world_;

        void SetUp() override {
            GenerateTestWorld(world_, waterTile_.get(), landTile_.get());
        }
    };


    TEST_F(WorldPlacementTest, PlaceEntity1x1Valid) {
        // Place an entity at various locations, checking that it does not place on invalid tiles
        const auto entity = std::make_unique<proto::ContainerEntity>();

        const auto* chunk = world_.GetChunkC({0, 0});

        EXPECT_TRUE(world_.Place({0, 0}, Orientation::up, entity.get()));

        EXPECT_EQ(chunk->Tiles(TileLayer::entity)[0].GetPrototype(), entity.get());

        EXPECT_FALSE(chunk->Tiles(TileLayer::entity)[0].IsMultiTile());
    }

    TEST_F(WorldPlacementTest, PlaceEntity1x1Invalid) {
        // Place an entity at various locations, checking that it does not place on invalid tiles
        const auto entity = std::make_unique<proto::ContainerEntity>();

        // Invalid, placing on a base tile which is water
        const auto* chunk = world_.GetChunkC({0, 0});

        EXPECT_FALSE(world_.Place({1, 0}, Orientation::up, entity.get()));
        EXPECT_EQ(chunk->Tiles(TileLayer::entity)[0].GetPrototype(), nullptr);
    }


    TEST_F(WorldPlacementTest, RemoveEntity1x1Valid) {
        // An existing tile location should have its entity and tile sprite pointer set to nullptr
        // To remove, pass a nullptr as entity
        const auto entity = std::make_unique<proto::ContainerEntity>();


        const auto* chunk = world_.GetChunkC({0, 0});

        // Place entity, taken from the test above (place_entity_1x1_valid)
        {
            // const auto* chunk = world.get_chunk(0, 0);

            EXPECT_TRUE(world_.Place({0, 0}, Orientation::up, entity.get()));

            EXPECT_EQ(chunk->Tiles(TileLayer::entity)[0].GetPrototype(), entity.get());
        }

        // Valid Removal
        EXPECT_TRUE(world_.Place({0, 0}, Orientation::up, nullptr));

        // Should all be nullptr after being removed
        EXPECT_EQ(chunk->Tiles(TileLayer::entity)[0].GetPrototype(), nullptr);
    }

    TEST_F(WorldPlacementTest, RemoveEntity1x1Invalid) {
        // Removing a location with nullptr entity and sprite does nothing, returns false to indicate nothing was
        // removed
        const auto* chunk = world_.GetChunkC({0, 0});

        // Invalid Removal
        EXPECT_FALSE(world_.Place({0, 0}, Orientation::up, nullptr));

        // Should all remain nullptr
        EXPECT_EQ(chunk->Tiles(TileLayer::entity)[0].GetPrototype(), nullptr);
    }


    TEST_F(WorldPlacementTest, PlaceEntity3x3Valid) {
        // For entities spanning > 1 tiles, the given location is the top left of the entity

        // Place an entity at various locations, checking that it does not place on invalid tiles
        const auto entity = std::make_unique<proto::ContainerEntity>();
        entity->SetWidth(3);
        entity->SetHeight(3);

        const auto* chunk = world_.GetChunkC({0, 0});


        EXPECT_TRUE(world_.Place({5, 5}, Orientation::up, entity.get()));

        int entity_index = 0;
        for (int y = 5; y < 5 + 3; ++y) {
            for (int x = 5; x < 5 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles(TileLayer::entity)[index].GetPrototype(), entity.get());

                // Should count up according to the rules specified in entity_index
                EXPECT_EQ(chunk->Tiles(TileLayer::entity)[index].GetMultiTileIndex(), entity_index++);
            }
        }
    }

    TEST_F(WorldPlacementTest, PlaceEntity3x3Invalid1) {
        // For entities spanning > 1 tiles, the given location is the top left of the entity

        // Place an entity at various locations, checking that it does not place on invalid tiles
        const auto entity = std::make_unique<proto::ContainerEntity>();
        entity->SetWidth(3);
        entity->SetHeight(3);

        const auto* chunk = world_.GetChunkC({0, 0});


        EXPECT_FALSE(world_.Place({4, 5}, Orientation::up, entity.get()));

        for (int y = 5; y < 5 + 3; ++y) {
            for (int x = 5; x < 5 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles(TileLayer::entity)[index].GetPrototype(), nullptr);
            }
        }
    }

    TEST_F(WorldPlacementTest, PlaceEntity3x3Invalid2) {
        // For entities spanning > 1 tiles, the given location is the top left of the entity

        // Place an entity at various locations, checking that it does not place on invalid tiles
        const auto entity = std::make_unique<proto::ContainerEntity>();
        entity->SetWidth(3);
        entity->SetHeight(3);

        const auto* chunk = world_.GetChunkC({0, 0});


        EXPECT_FALSE(world_.Place({9, 2}, Orientation::up, entity.get()));

        for (int y = 5; y < 5 + 3; ++y) {
            for (int x = 5; x < 5 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles(TileLayer::entity)[index].GetPrototype(), nullptr);
            }
        }
    }

    TEST_F(WorldPlacementTest, PlaceEntity3x3Invalid3) {
        // When the placed entity overlaps another entity, the placement is also invalid
        const auto entity = std::make_unique<proto::ContainerEntity>();
        entity->SetWidth(3);
        entity->SetHeight(3);


        EXPECT_TRUE(world_.Place({9, 10}, Orientation::up, entity.get()));
        EXPECT_FALSE(world_.Place({9, 9}, Orientation::up, entity.get()));
    }

    TEST_F(WorldPlacementTest, RemoveEntity3x3Valid1) {
        // When removing an entity, specifying anywhere will remove the entire entity
        const auto entity = std::make_unique<proto::ContainerEntity>();
        entity->SetWidth(3);
        entity->SetHeight(3);

        const auto* chunk = world_.GetChunkC({0, 0});


        EXPECT_TRUE(world_.Place({5, 5}, Orientation::up, entity.get()));
        EXPECT_TRUE(world_.Place({5, 5}, Orientation::up, nullptr));

        // Check that it has been deleted
        for (int y = 5; y < 5 + 3; ++y) {
            for (int x = 5; x < 5 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles(TileLayer::entity)[index].GetPrototype(), nullptr);

                // Entity index is undefined since no entity exists now
            }
        }
    }

    TEST_F(WorldPlacementTest, RemoveEntity3x3Valid2) {
        // When removing an entity, specifying anywhere will remove the entire entity
        const auto entity = std::make_unique<proto::ContainerEntity>();
        entity->SetWidth(3);
        entity->SetHeight(3);

        const auto* chunk = world_.GetChunkC({0, 0});


        EXPECT_TRUE(world_.Place({5, 5}, Orientation::up, entity.get()));
        EXPECT_TRUE(world_.Place({7, 5}, Orientation::up, nullptr));
        // Check that it has been deleted
        for (int y = 5; y < 5 + 3; ++y) {
            for (int x = 5; x < 5 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles(TileLayer::entity)[index].GetPrototype(), nullptr);

                // Entity index is undefined since no entity exists now
            }
        }
    }


    // 3 x 4

    TEST_F(WorldPlacementTest, PlaceEntity3x4Valid) {
        // Ensure that irregular shaped multi-tiles fully remove

        // For entities spanning > 1 tiles, the given location is the top left of the entity
        const auto entity = std::make_unique<proto::ContainerEntity>();
        entity->SetWidth(3);
        entity->SetHeight(4);

        const auto* chunk = world_.GetChunkC({0, 0});


        EXPECT_TRUE(world_.Place({9, 10}, Orientation::up, entity.get()));

        int entity_index = 0;
        for (int y = 10; y < 10 + 4; ++y) {
            for (int x = 9; x < 9 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles(TileLayer::entity)[index].GetPrototype(), entity.get());

                // Should count up according to the rules specified in entity_index
                EXPECT_EQ(chunk->Tiles(TileLayer::entity)[index].GetMultiTileIndex(), entity_index++);


                // Ensure tile width and height are properly set
                EXPECT_EQ(chunk->Tiles(TileLayer::entity)[index].GetDimensions().span, 3);
                EXPECT_EQ(chunk->Tiles(TileLayer::entity)[index].GetDimensions().height, 4);
            }
        }
    }

    TEST_F(WorldPlacementTest, RemoveEntity3x4Valid) {
        // Ensure that irregular shaped multi-tiles fully remove
        const auto entity = std::make_unique<proto::ContainerEntity>();
        entity->SetWidth(3);
        entity->SetHeight(4);

        const auto* chunk = world_.GetChunkC({0, 0});


        EXPECT_TRUE(world_.Place({9, 10}, Orientation::up, entity.get()));
        EXPECT_TRUE(world_.Place({9, 13}, Orientation::up, nullptr));

        for (int y = 10; y < 10 + 4; ++y) {
            for (int x = 9; x < 9 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles(TileLayer::entity)[index].GetPrototype(), nullptr);
            }
        }
    }
} // namespace jactorio::game
