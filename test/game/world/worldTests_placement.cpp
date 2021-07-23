// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/world/world.h"

#include "jactorioTests.h"

#include "proto/container_entity.h"
#include "proto/sprite.h"
#include "proto/tile.h"

namespace jactorio::game
{
    // Creates world and generates a test world within it
    class WorldPlacementTest : public testing::Test
    {
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

            for (ChunkTileCoordAxis y = 0; y < Chunk::kChunkWidth; ++y) {
                bool y_water = false;

                // Water at Y index 1, 4, 8
                if (y == 1 || y == 4 || y == 8)
                    y_water = true;

                for (ChunkTileCoordAxis x = 0; x < Chunk::kChunkWidth; ++x) {
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
        proto::ContainerEntity entity_;
        proto::Sprite sprite_;

        proto::Tile waterTile_;
        proto::Tile landTile_;

        World world_;

        void SetUp() override {
            GenerateTestWorld(world_, &waterTile_, &landTile_);

            // Conveyor utility requires tex coord id when setting up conveyor
            entity_.sprite     = &sprite_;
            sprite_.texCoordId = 1234;
        }


        /// Checks that entity with top left at coord with dimensions has been placed
        void ValidatePlaced(const WorldCoord& coord, const proto::Entity& entity, const Dimension& dimension = {1, 1}) {
            int entity_index     = 0;
            int tex_coord_offset = 0;
            for (WorldCoordAxis y = coord.y; y < coord.y + dimension.y; ++y) {
                for (WorldCoordAxis x = coord.x; x < coord.x + dimension.x; ++x) {
                    auto* tile = world_.GetTile({x, y}, TileLayer::entity);

                    ASSERT_NE(tile, nullptr);
                    EXPECT_EQ(tile->GetPrototype(), &entity);
                    EXPECT_EQ(tile->GetMultiTileIndex(), entity_index++);

                    EXPECT_EQ(world_.GetTexCoordId({x, y}, TileLayer::entity), 1234 + tex_coord_offset++);

                    // Ensure tile width and height are properly set
                    EXPECT_EQ(tile->GetDimension().x, dimension.x);
                    EXPECT_EQ(tile->GetDimension().y, dimension.y);
                }
            }
        }

        /// Checks that entity with top left at coord with dimensions has been deleted
        void ValidateEmpty(const WorldCoord& coord, const Dimension& dimension = {1, 1}) {
            for (WorldCoordAxis y = coord.y; y < coord.y + dimension.y; ++y) {
                for (WorldCoordAxis x = coord.x; x < coord.x + dimension.x; ++x) {
                    auto* tile = world_.GetTile({x, y}, TileLayer::entity);

                    ASSERT_NE(tile, nullptr);
                    EXPECT_EQ(tile->GetPrototype(), nullptr);
                    // Entity index is undefined since no entity exists now
                    EXPECT_EQ(world_.GetTexCoordId({x, y}, TileLayer::entity), 0);
                }
            }
        }
    };


    TEST_F(WorldPlacementTest, PlaceEntity1x1Valid) {
        EXPECT_TRUE(world_.Place({0, 0}, Orientation::up, entity_));

        ValidatePlaced({0, 0}, entity_);
    }

    TEST_F(WorldPlacementTest, PlaceEntity1x1Invalid) {
        // Invalid, placing on a base tile which is water
        EXPECT_FALSE(world_.Place({1, 0}, Orientation::up, entity_));

        ValidateEmpty({1, 0});
    }


    TEST_F(WorldPlacementTest, RemoveEntity1x1Valid) {
        EXPECT_TRUE(world_.Place({10, 9}, Orientation::up, entity_));
        EXPECT_TRUE(world_.Remove({10, 9}, Orientation::up));

        ValidateEmpty({10, 9});
    }

    TEST_F(WorldPlacementTest, RemoveNonExistentEntity1x1) {
        EXPECT_FALSE(world_.Remove({11, 9}, Orientation::up));
        ValidateEmpty({11, 9});
    }


    // Multi tile

    TEST_F(WorldPlacementTest, PlaceEntity3x3Valid) {
        const Dimension dimension(3, 3);
        entity_.SetDimension(dimension);

        EXPECT_TRUE(world_.Place({5, 5}, Orientation::up, entity_));

        ValidatePlaced({5, 5}, entity_, dimension);
    }

    TEST_F(WorldPlacementTest, PlaceEntity2x3InvalidTopLeft) {
        const Dimension dimension(2, 3);
        entity_.SetDimension(dimension);

        // x = 4 is invalid tile
        EXPECT_FALSE(world_.Place({4, 5}, Orientation::up, entity_));

        ValidateEmpty({4, 5}, dimension);
    }

    TEST_F(WorldPlacementTest, PlaceEntity3x3InvalidNonTopLeft) {
        const Dimension dimension(3, 3);
        entity_.SetDimension(dimension);

        // y = 1 is invalid tile
        EXPECT_FALSE(world_.Place({9, 2}, Orientation::up, entity_));

        ValidateEmpty({9, 2}, dimension);
    }

    /// When the placed entity overlaps another entity, the placement is also invalid
    TEST_F(WorldPlacementTest, PlaceEntity3x3OverlapExisting) {
        const Dimension dimension(3, 3);
        entity_.SetDimension(dimension);

        EXPECT_TRUE(world_.Place({9, 10}, Orientation::up, entity_));
        EXPECT_FALSE(world_.Place({9, 9}, Orientation::up, entity_));

        ValidateEmpty({9, 9}, {3, 1});
        ValidatePlaced({9, 10}, entity_, dimension);
    }


    TEST_F(WorldPlacementTest, RemoveEntity3x3Valid) {
        const Dimension dimension(3, 3);
        entity_.SetDimension(dimension);

        EXPECT_TRUE(world_.Place({5, 5}, Orientation::up, entity_));
        EXPECT_TRUE(world_.Remove({5, 5}, Orientation::up));

        ValidateEmpty({5, 5}, dimension);
    }

    /// When removing an entity, specifying anywhere will remove the entire entity
    TEST_F(WorldPlacementTest, RemoveEntity3x3ValidSelectNonTopLeft) {
        const Dimension dimension(3, 3);
        entity_.SetDimension(dimension);

        EXPECT_TRUE(world_.Place({5, 5}, Orientation::up, entity_));
        EXPECT_TRUE(world_.Remove({7, 5}, Orientation::up));

        ValidateEmpty({5, 5}, dimension);
    }


    // 3 x 4

    TEST_F(WorldPlacementTest, PlaceEntity3x4Valid) {
        const Dimension dimension(3, 4);
        entity_.SetDimension(dimension);

        EXPECT_TRUE(world_.Place({9, 10}, Orientation::up, entity_));

        ValidatePlaced({9, 10}, entity_, dimension);
    }

    TEST_F(WorldPlacementTest, RemoveEntity3x4Valid) {
        const Dimension dimension(3, 4);
        entity_.SetDimension(dimension);


        EXPECT_TRUE(world_.Place({9, 10}, Orientation::up, entity_));
        EXPECT_TRUE(world_.Remove({9, 13}, Orientation::up));

        ValidateEmpty({9, 10}, dimension);
    }

    TEST_F(WorldPlacementTest, CallGetTexCoordId) {
        class Mock : public TestMockEntity
        {
        public:
            mutable int timesCalled = 0;
            SpriteTexCoordIndexT OnGetTexCoordId(const World& /*world*/,
                                                 const WorldCoord& coord,
                                                 const Orientation orientation) const override {
                EXPECT_EQ(coord, WorldCoord(9, 10));
                EXPECT_EQ(orientation, Orientation::right);
                timesCalled++;
                return 1234;
            }
        };

        Mock mock;
        mock.SetDimension({2, 4});
        world_.Place({9, 10}, Direction::right, mock);

        ValidatePlaced({9, 10}, mock, {4, 2}); // Flipped since facing right

        EXPECT_EQ(mock.timesCalled, 1);
    }
} // namespace jactorio::game
