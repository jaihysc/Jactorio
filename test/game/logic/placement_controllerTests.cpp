// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/logic/placement_controller.h"

#include <memory>

#include "game/world/world_data.h"
#include "proto/container_entity.h"
#include "proto/tile.h"

namespace jactorio::game
{
    // NOTE
    // !Checking if the selected tile is too far from the player is not done in these tests!
    //

    // Creates world_data and generates a test world within it
    class PlacementControllerTest : public testing::Test
    {
    public:
        static constexpr auto kChunkWidth = WorldData::kChunkWidth;

    private:
        static void GenerateTestWorld(WorldData& world_data, proto::Tile* water_tile, proto::Tile* land_tile) {
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


            auto& chunk = world_data.EmplaceChunk(0, 0);

            auto& chunk_tiles = chunk.Tiles();
            for (int y = 0; y < kChunkWidth; ++y) {
                bool y_water = false;

                // Water at Y index 1, 4, 8
                if (y == 1 || y == 4 || y == 8)
                    y_water = true;

                for (int x = 0; x < kChunkWidth; ++x) {
                    bool x_water = false;

                    // Water at X index 1, 4, 8
                    if (x == 1 || x == 4 || x == 8)
                        x_water = true;

                    auto* tile_ptr = land_tile;
                    if (x_water || y_water) {
                        tile_ptr = water_tile;
                    }

                    chunk_tiles[y * kChunkWidth + x].SetTilePrototype(tile_ptr);
                }
            }
        }

    protected:
        std::unique_ptr<proto::Tile> waterTile_ = std::make_unique<proto::Tile>();
        std::unique_ptr<proto::Tile> landTile_  = std::make_unique<proto::Tile>();

        WorldData worldData_{};

        void SetUp() override {
            GenerateTestWorld(worldData_, waterTile_.get(), landTile_.get());
        }
    };


    TEST_F(PlacementControllerTest, PlaceEntity1x1Valid) {
        // Place an entity at various locations, checking that it does not place on invalid tiles
        const auto entity = std::make_unique<proto::ContainerEntity>();

        const auto* chunk = worldData_.GetChunkC(0, 0);

        EXPECT_EQ(jactorio::game::PlaceEntityAtCoords(worldData_, entity.get(), 0, 0), true);

        // Set entity and sprite layer
        EXPECT_EQ(chunk->Tiles()[0].GetEntityPrototype(), entity.get());

        EXPECT_FALSE(chunk->Tiles()[0].GetLayer(jactorio::game::TileLayer::entity).IsMultiTile());
    }

    TEST_F(PlacementControllerTest, PlaceEntity1x1Invalid) {
        // Place an entity at various locations, checking that it does not place on invalid tiles
        const auto entity = std::make_unique<proto::ContainerEntity>();

        // Invalid, placing on a base tile which is water
        const auto* chunk = worldData_.GetChunkC(0, 0);

        EXPECT_EQ(jactorio::game::PlaceEntityAtCoords(worldData_, entity.get(), 1, 0), false);
        EXPECT_EQ(chunk->Tiles()[0].GetEntityPrototype(), nullptr);
    }


    TEST_F(PlacementControllerTest, RemoveEntity1x1Valid) {
        // An existing tile location should have its entity and layer sprite pointer set to nullptr
        // To remove, pass a nullptr as entity
        const auto entity = std::make_unique<proto::ContainerEntity>();


        const auto* chunk = worldData_.GetChunkC(0, 0);

        // Place entity, taken from the test above (place_entity_1x1_valid)
        {
            // const auto* chunk = world_data.get_chunk(0, 0);

            EXPECT_TRUE(jactorio::game::PlaceEntityAtCoords(worldData_, entity.get(), 0, 0));

            // Set entity and sprite layer
            EXPECT_EQ(chunk->Tiles()[0].GetEntityPrototype(), entity.get());
        }

        // Valid Removal
        EXPECT_TRUE(jactorio::game::PlaceEntityAtCoords(worldData_, nullptr, 0, 0));

        // Should all be nullptr after being removed
        EXPECT_EQ(chunk->Tiles()[0].GetEntityPrototype(), nullptr);
    }

    TEST_F(PlacementControllerTest, RemoveEntity1x1Invalid) {
        // Removing a location with nullptr entity and sprite does nothing, returns false to indicate nothing was
        // removed
        const auto* chunk = worldData_.GetChunkC(0, 0);

        // Invalid Removal
        EXPECT_FALSE(jactorio::game::PlaceEntityAtCoords(worldData_, nullptr, 0, 0));

        // Should all remain nullptr
        EXPECT_EQ(chunk->Tiles()[0].GetEntityPrototype(), nullptr);
    }


    TEST_F(PlacementControllerTest, PlaceEntity3x3Valid) {
        // For entities spanning > 1 tiles, the given location is the top left of the entity

        // Place an entity at various locations, checking that it does not place on invalid tiles
        const auto entity  = std::make_unique<proto::ContainerEntity>();
        entity->tileWidth  = 3;
        entity->tileHeight = 3;

        const auto* chunk = worldData_.GetChunkC(0, 0);


        EXPECT_TRUE(jactorio::game::PlaceEntityAtCoords(worldData_, entity.get(), 5, 5));

        // Expect entity and sprite layer to be set, as well as entity_index
        int entity_index = 0;
        for (int y = 5; y < 5 + 3; ++y) {
            for (int x = 5; x < 5 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles()[index].GetEntityPrototype(), entity.get());

                // Should count up according to the rules specified in entity_index
                EXPECT_EQ(chunk->Tiles()[index].GetLayer(jactorio::game::TileLayer::entity).GetMultiTileIndex(),
                          entity_index++);
            }
        }
    }

    TEST_F(PlacementControllerTest, PlaceEntity3x3Invalid1) {
        // For entities spanning > 1 tiles, the given location is the top left of the entity

        // Place an entity at various locations, checking that it does not place on invalid tiles
        const auto entity  = std::make_unique<proto::ContainerEntity>();
        entity->tileWidth  = 3;
        entity->tileHeight = 3;

        const auto* chunk = worldData_.GetChunkC(0, 0);


        EXPECT_FALSE(jactorio::game::PlaceEntityAtCoords(worldData_, entity.get(), 4, 5));

        // Expect entity and sprite layer to be set, as well as entity_index
        for (int y = 5; y < 5 + 3; ++y) {
            for (int x = 5; x < 5 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles()[index].GetEntityPrototype(), nullptr);
            }
        }
    }

    TEST_F(PlacementControllerTest, PlaceEntity3x3Invalid2) {
        // For entities spanning > 1 tiles, the given location is the top left of the entity

        // Place an entity at various locations, checking that it does not place on invalid tiles
        const auto entity  = std::make_unique<proto::ContainerEntity>();
        entity->tileWidth  = 3;
        entity->tileHeight = 3;

        const auto* chunk = worldData_.GetChunkC(0, 0);


        EXPECT_FALSE(jactorio::game::PlaceEntityAtCoords(worldData_, entity.get(), 9, 2));

        // Expect entity and sprite layer to be set, as well as entity_index
        for (int y = 5; y < 5 + 3; ++y) {
            for (int x = 5; x < 5 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles()[index].GetEntityPrototype(), nullptr);
            }
        }
    }

    TEST_F(PlacementControllerTest, PlaceEntity3x3Invalid3) {
        // When the placed entity overlaps another entity, the placement is also invalid
        const auto entity  = std::make_unique<proto::ContainerEntity>();
        entity->tileWidth  = 3;
        entity->tileHeight = 3;


        EXPECT_TRUE(jactorio::game::PlaceEntityAtCoords(worldData_, entity.get(), 9, 10));
        EXPECT_FALSE(jactorio::game::PlaceEntityAtCoords(worldData_, entity.get(), 9, 9));
    }

    TEST_F(PlacementControllerTest, RemoveEntity3x3Valid1) {
        // When removing an entity, specifying anywhere will remove the entire entity
        const auto entity  = std::make_unique<proto::ContainerEntity>();
        entity->tileWidth  = 3;
        entity->tileHeight = 3;

        const auto* chunk = worldData_.GetChunkC(0, 0);


        EXPECT_TRUE(jactorio::game::PlaceEntityAtCoords(worldData_, entity.get(), 5, 5));
        EXPECT_TRUE(jactorio::game::PlaceEntityAtCoords(worldData_, nullptr, 5, 5));

        // Check that it has been deleted
        for (int y = 5; y < 5 + 3; ++y) {
            for (int x = 5; x < 5 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles()[index].GetEntityPrototype(), nullptr);

                // Entity index is undefined since no entity exists now
            }
        }
    }

    TEST_F(PlacementControllerTest, RemoveEntity3x3Valid2) {
        // When removing an entity, specifying anywhere will remove the entire entity
        const auto entity  = std::make_unique<proto::ContainerEntity>();
        entity->tileWidth  = 3;
        entity->tileHeight = 3;

        const auto* chunk = worldData_.GetChunkC(0, 0);


        EXPECT_TRUE(jactorio::game::PlaceEntityAtCoords(worldData_, entity.get(), 5, 5));
        EXPECT_TRUE(jactorio::game::PlaceEntityAtCoords(worldData_, nullptr, 7, 5));
        // Check that it has been deleted
        for (int y = 5; y < 5 + 3; ++y) {
            for (int x = 5; x < 5 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles()[index].GetEntityPrototype(), nullptr);

                // Entity index is undefined since no entity exists now
            }
        }
    }


    // 3 x 4

    TEST_F(PlacementControllerTest, PlaceEntity3x4Valid) {
        // Ensure that irregular shaped multi-tiles fully remove

        // For entities spanning > 1 tiles, the given location is the top left of the entity
        const auto entity  = std::make_unique<proto::ContainerEntity>();
        entity->tileWidth  = 3;
        entity->tileHeight = 4;

        const auto* chunk = worldData_.GetChunkC(0, 0);


        EXPECT_TRUE(jactorio::game::PlaceEntityAtCoords(worldData_, entity.get(), 9, 10));

        // Expect entity and sprite layer to be set, as well as entity_index
        int entity_index = 0;
        for (int y = 10; y < 10 + 4; ++y) {
            for (int x = 9; x < 9 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles()[index].GetEntityPrototype(), entity.get());

                // Should count up according to the rules specified in entity_index
                EXPECT_EQ(chunk->Tiles()[index].GetLayer(jactorio::game::TileLayer::entity).GetMultiTileIndex(),
                          entity_index++);


                // Ensure tile width and height are properly set
                EXPECT_EQ(chunk->Tiles()[index].GetLayer(jactorio::game::TileLayer::entity).GetMultiTileData().span, 3);
                EXPECT_EQ(chunk->Tiles()[index].GetLayer(jactorio::game::TileLayer::entity).GetMultiTileData().height,
                          4);
            }
        }
    }

    TEST_F(PlacementControllerTest, RemoveEntity3x4Valid) {
        // Ensure that irregular shaped multi-tiles fully remove
        const auto entity  = std::make_unique<proto::ContainerEntity>();
        entity->tileWidth  = 3;
        entity->tileHeight = 4;

        const auto* chunk = worldData_.GetChunkC(0, 0);


        EXPECT_TRUE(jactorio::game::PlaceEntityAtCoords(worldData_, entity.get(), 9, 10));
        EXPECT_TRUE(jactorio::game::PlaceEntityAtCoords(worldData_, nullptr, 9, 13));

        // Expect entity and sprite layer to be set, as well as entity_index
        for (int y = 10; y < 10 + 4; ++y) {
            for (int x = 9; x < 9 + 3; ++x) {
                const auto index = y * kChunkWidth + x;
                EXPECT_EQ(chunk->Tiles()[index].GetEntityPrototype(), nullptr);
            }
        }
    }
} // namespace jactorio::game
