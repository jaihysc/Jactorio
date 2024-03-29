// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/world/world.h"

#include "jactorioTests.h"

#include "proto/noise_layer.h"

namespace jactorio::game
{
    class WorldTest : public testing::Test
    {
    protected:
        World world_;
    };

    TEST_F(WorldTest, WorldCToChunkC) {
        EXPECT_EQ(World::WorldCToChunkC(-33), -2);
        EXPECT_EQ(World::WorldCToChunkC(-32), -1);
        EXPECT_EQ(World::WorldCToChunkC(-1), -1);
        EXPECT_EQ(World::WorldCToChunkC(31), 0);
        EXPECT_EQ(World::WorldCToChunkC(32), 1);

        EXPECT_EQ(World::WorldCToChunkC({160, 999999}).x, 5);
        EXPECT_EQ(World::WorldCToChunkC({999999, -64}).y, -2);
    }

    TEST_F(WorldTest, ChunkCToWorldC) {
        EXPECT_EQ(World::ChunkCToWorldC(0), 0);
        EXPECT_EQ(World::ChunkCToWorldC(1), 32);
        EXPECT_EQ(World::ChunkCToWorldC(5), 160);
        EXPECT_EQ(World::ChunkCToWorldC(-1), -32);
        EXPECT_EQ(World::ChunkCToWorldC(-2), -64);

        EXPECT_EQ(World::ChunkCToWorldC({-6, 2323232}).x, -192);
        EXPECT_EQ(World::ChunkCToWorldC({12321, 420}).y, 13440);
    }

    TEST_F(WorldTest, WorldCToOverlayC) {
        EXPECT_FLOAT_EQ(World::WorldCToOverlayC(0), 0);
        EXPECT_FLOAT_EQ(World::WorldCToOverlayC(31), 31);
        EXPECT_FLOAT_EQ(World::WorldCToOverlayC(32), 0);
        EXPECT_FLOAT_EQ(World::WorldCToOverlayC(63), 31);

        EXPECT_FLOAT_EQ(World::WorldCToOverlayC(-1), 31);
        EXPECT_FLOAT_EQ(World::WorldCToOverlayC(-32), 0);
        EXPECT_FLOAT_EQ(World::WorldCToOverlayC(-33), 31);
        EXPECT_FLOAT_EQ(World::WorldCToOverlayC(-65), 31);
    }

    TEST_F(WorldTest, AddChunk) {
        const auto& added_chunk = world_.EmplaceChunk({5, 1});

        EXPECT_EQ(added_chunk.GetPosition().x, 5);
        EXPECT_EQ(added_chunk.GetPosition().y, 1);

        // Should not initialize other chunks
        EXPECT_EQ(world_.GetChunkC({-1, -1}), nullptr);
        EXPECT_EQ(world_.GetChunkC({1, 1}), nullptr);


        auto [ptr, readable_chunks] = world_.GetChunkTexCoordIds({0, 1});
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(readable_chunks, 6);

        EXPECT_EQ(world_.GetChunkTexCoordIds({5, 1}).second, 1);
        EXPECT_EQ(world_.GetChunkTexCoordIds({5, 0}).second, 0);
        EXPECT_EQ(world_.GetChunkTexCoordIds({-1, 1}).second, 0);
        EXPECT_EQ(world_.GetChunkTexCoordIds({6, 1}).second, 0);
    }

    TEST_F(WorldTest, AddChunkNegative) {
        // Chunks initialized with empty tiles
        // Returns pointer to chunk which was added
        auto& added_chunk = world_.EmplaceChunk({-5, -1});

        // Chunk knows its own location
        EXPECT_EQ(added_chunk.GetPosition().x, -5);
        EXPECT_EQ(added_chunk.GetPosition().y, -1);


        // Should not initialize other chunks
        EXPECT_EQ(world_.GetChunkC({-1, -1}), nullptr);
        EXPECT_EQ(world_.GetChunkC({1, 1}), nullptr);

        auto [ptr, readable_chunks] = world_.GetChunkTexCoordIds({-5, -1});
        EXPECT_NE(ptr, nullptr);
        EXPECT_EQ(readable_chunks, 5);
    }

    TEST_F(WorldTest, DeleteChunk) {
        world_.EmplaceChunk({3, 2});
        world_.GetChunkTexCoordIds({3, 2}).first[0] = 100; // Check that this was zeroed

        world_.DeleteChunk({3, 2});

        EXPECT_EQ(world_.GetChunkC({3, 2}), nullptr);

        auto [ptr, readable_chunks] = world_.GetChunkTexCoordIds({3, 2});
        EXPECT_EQ(readable_chunks, 1); // Still readable
        for (int i = 0; i < Chunk::kChunkArea * kTileLayerCount; ++i) {
            EXPECT_EQ(ptr[i], 0);
        }

        // No effect, no chunk
        world_.DeleteChunk({2000, 2000});
    }

    TEST_F(WorldTest, GetChunkChunkCoords) {
        const auto& added_chunk = world_.EmplaceChunk({5, 1});

        EXPECT_EQ(world_.GetChunkC({0, 0}), nullptr);
        EXPECT_EQ(world_.GetChunkC({5, 1}), &added_chunk);
        EXPECT_EQ(world_.GetChunkC({5, 1}), &added_chunk);
    }

    TEST_F(WorldTest, GetTileWorldCoords) {
        // World coords 0, 0 - Chunk 0 0, position 0 0
        {
            auto& chunk = world_.EmplaceChunk({0, 0});

            EXPECT_EQ(world_.GetTile({0, 0}, TileLayer::base), &chunk.Tiles(TileLayer::base)[0]);
            EXPECT_NE(world_.GetTile({0, 1}, TileLayer::entity), &chunk.Tiles(TileLayer::entity)[0]);
        }
        world_.Clear();

        // World coords -31, -31 - Chunk -1 -1, position 1 1
        {
            auto& chunk = world_.EmplaceChunk({-1, -1});
            auto& tiles = chunk.Tiles(TileLayer::base);

            EXPECT_EQ(world_.GetTile({-31, -31}, TileLayer::base), &tiles[33]);
        }
        world_.Clear();

        // World coords -32, 0 - Chunk -1 0, position 0 0
        {
            auto& chunk = world_.EmplaceChunk({-1, 0});
            auto& tiles = chunk.Tiles(TileLayer::base);

            EXPECT_EQ(world_.GetTile({-32, 0}, TileLayer::base), &tiles[0]);
        }
    }

    TEST_F(WorldTest, GetChunkWorldCoords) {
        {
            const auto& chunk = world_.EmplaceChunk({0, 0});
            EXPECT_EQ(world_.GetChunkW({31, 31}), &chunk);

            EXPECT_EQ(world_.GetChunkW({31, 31}), &chunk);
        }

        {
            const auto& chunk = world_.EmplaceChunk({-1, 0});
            EXPECT_EQ(world_.GetChunkW({-1, 0}), &chunk);

            EXPECT_EQ(world_.GetChunkW({-1, 0}), &chunk);
        }
    }

    TEST_F(WorldTest, GetSetTexCoordId) {
        world_.EmplaceChunk({3, 1});

        world_.SetTexCoordId({106, 60}, TileLayer::resource, 4321);

        auto [ptr, readable_chunks] = world_.GetChunkTexCoordIds({3, 1});
        EXPECT_EQ(readable_chunks, 1);
        EXPECT_EQ(
            ptr[(Chunk::kChunkWidth * (60 % Chunk::kChunkWidth) + (106 % Chunk::kChunkWidth)) * kTileLayerCount + 1],
            4321);

        EXPECT_EQ(world_.GetTexCoordId({106, 60}, TileLayer::resource), 4321);
    }

    TEST_F(WorldTest, SerializeTexCoordIds) {
        world_.EmplaceChunk({1, 2});
        {
            auto [ptr, readable_amount] = world_.GetChunkTexCoordIds({1, 2});
            for (int i = 0; i < 1000; ++i) {
                ptr[i] = i;
            }
        }

        auto result = TestSerializeDeserialize(world_);

        auto [ptr, readable_amount] = result.GetChunkTexCoordIds({1, 2});
        EXPECT_EQ(readable_amount, 1);
        for (int i = 0; i < 1000; ++i) {
            EXPECT_EQ(ptr[i], i);
        }
    }

    TEST_F(WorldTest, EnableDisableAnimation) {
        world_.EmplaceChunk({0, 0});
        constexpr auto animation_offset = 101;

        proto::ContainerEntity container;
        container.SetDimension({2, 3});
        TestSetupMultiTile(world_, {1, 2}, TileLayer::entity, Direction::up, container);

        auto check_disabled = [this, animation_offset]() {
            for (int y = 2; y <= 4; ++y) {
                for (int x = 1; x <= 2; ++x) {
                    EXPECT_EQ(world_.GetTexCoordId({x, y}, TileLayer::entity), animation_offset);
                }
            }
        };
        world_.DisableAnimation({2, 2}, TileLayer::entity, animation_offset); // Does not need to be top left
        check_disabled();
        world_.DisableAnimation({2, 2}, TileLayer::entity, animation_offset); // Calling multiple times does nothing
        world_.DisableAnimation({2, 2}, TileLayer::entity, animation_offset);
        check_disabled();


        auto check_enabled = [this]() {
            for (int y = 2; y <= 4; ++y) {
                for (int x = 1; x <= 2; ++x) {
                    EXPECT_EQ(world_.GetTexCoordId({x, y}, TileLayer::entity), 0);
                }
            }
        };
        world_.EnableAnimation({2, 4}, TileLayer::entity, animation_offset); // Does not need to be top left
        check_enabled();
        world_.EnableAnimation({2, 4}, TileLayer::entity, animation_offset); // Calling multiple times does nothing
        world_.EnableAnimation({2, 4}, TileLayer::entity, animation_offset);
        check_enabled();
    }

    TEST_F(WorldTest, GenerateChunk) {
        auto& chunk = world_.EmplaceChunk({1, 2});

        // Does nothing since chunk already exists
        world_.QueueChunkGeneration({1, 2});

        data::PrototypeManager proto;
        proto::Tile tile;
        auto& noise_layer = proto.Make<proto::NoiseLayer<proto::Tile>>();
        noise_layer.Add(1, &tile);
        noise_layer.normalize = true;

        world_.GenChunk(proto, 200);
        EXPECT_NE(chunk.GetCTile({0, 0}, TileLayer::base).GetPrototype(), &tile);
    }

    TEST_F(WorldTest, Clear) {
        auto& added_chunk = world_.EmplaceChunk({6, 6});

        EXPECT_EQ(world_.GetChunkC({6, 6}), &added_chunk);
        world_.LogicRegister(LogicGroup::inserter, {192, 192}, TileLayer::entity);
        world_.QueueChunkGeneration({0, 0});


        world_.Clear();


        const data::PrototypeManager proto;
        world_.GenChunk(proto);

        EXPECT_EQ(world_.GetChunkC({0, 0}), nullptr);

        EXPECT_EQ(world_.GetChunkC({6, 6}), nullptr);
        EXPECT_TRUE(world_.LogicGet(LogicGroup::inserter).empty());
        EXPECT_EQ(world_.GetChunkTexCoordIds({6, 6}).second, 0);
    }


    // Logic chunks

    TEST_F(WorldTest, LogicRegister) {
        world_.EmplaceChunk({1, 0}); // 32, 0 is chunk coords 1, 0
        world_.LogicRegister(LogicGroup::inserter, {32, 0}, TileLayer::entity);

        auto& logic_list = world_.LogicGet(LogicGroup::inserter);
        EXPECT_EQ(logic_list.size(), 1);

        // Registering again will not duplicate
        // Does not care about modified prototypes / unique data
        const proto::ContainerEntity container;
        world_.GetTile({32, 0}, TileLayer::entity)->SetPrototype(Direction::up, container);
        world_.LogicRegister(LogicGroup::inserter, {32, 0}, TileLayer::entity);
        EXPECT_EQ(logic_list.size(), 1);
    }

    TEST_F(WorldTest, LogicRemove) {
        world_.EmplaceChunk({1, 0});
        world_.LogicRegister(LogicGroup::inserter, {32, 0}, TileLayer::entity);
        world_.LogicRegister(LogicGroup::inserter, {33, 0}, TileLayer::entity);
        world_.LogicRegister(LogicGroup::conveyor, {33, 0}, TileLayer::entity);

        world_.LogicRemove(LogicGroup::inserter, {32, 0}, TileLayer::entity);
        EXPECT_EQ(world_.LogicGet(LogicGroup::inserter).size(), 1);
        EXPECT_EQ(world_.LogicGet(LogicGroup::conveyor).size(), 1);

        EXPECT_EQ(world_.LogicGet(LogicGroup::inserter)[0].coord, WorldCoord(33, 0));

        // Removing again does nothing
        world_.LogicRemove(LogicGroup::inserter, {32, 0}, TileLayer::entity);
        EXPECT_EQ(world_.LogicGet(LogicGroup::inserter).size(), 1);
        EXPECT_EQ(world_.LogicGet(LogicGroup::conveyor).size(), 1);
    }

    class WorldDeserialize : public testing::Test
    {
    protected:
        World world_;
        Logic logic_;
    };

    TEST_F(WorldDeserialize, SameChunk) {
        world_.EmplaceChunk({0, 0});

        data::PrototypeManager proto;
        auto& container = proto.Make<proto::ContainerEntity>();

        container.SetDimension({3, 2});
        TestSetupMultiTile(world_, {1, 0}, TileLayer::base, Orientation::up, container);


        proto.GenerateRelocationTable();
        data::active_prototype_manager = &proto;

        world_ = TestSerializeDeserialize(world_);

        world_.DeserializePostProcess();


        /// Checks that multi-tile tile is linked to top left
        auto expect_tl_resolved = [this, &container](const WorldCoord& coord, const TileLayer tlayer) {
            auto* top_left = world_.GetTile(coord, tlayer)->GetTopLeft();
            EXPECT_EQ(top_left, world_.GetTile({1, 0}, TileLayer::base));
            EXPECT_EQ(top_left->GetPrototype(), &container);
        };

        expect_tl_resolved({2, 0}, TileLayer::base);
        expect_tl_resolved({3, 0}, TileLayer::base);

        expect_tl_resolved({1, 1}, TileLayer::base);
        expect_tl_resolved({2, 1}, TileLayer::base);
        expect_tl_resolved({3, 1}, TileLayer::base);
    }

    TEST_F(WorldDeserialize, ResolveMultiTilesFirst) {
        world_.EmplaceChunk({0, 0});

        /*
         *   I
         * A A  Must resolve Assembly machine multi tile first before calling OnDeserialize for Inserter
         * A A
         */

        data::PrototypeManager proto;
        data::UniqueDataManager unique;


        auto& asm_machine = proto.Make<proto::AssemblyMachine>();
        asm_machine.SetDimension({2, 2});
        TestSetupAssemblyMachine(world_, {0, 2}, Orientation::up, asm_machine);

        auto& inserter = proto.Make<proto::Inserter>();
        TestSetupInserter(world_, logic_, {1, 1}, Orientation::down, inserter);


        data::active_prototype_manager   = &proto;
        data::active_unique_data_manager = &unique;
        proto.GenerateRelocationTable();

        auto result = TestSerializeDeserialize(world_);
        result.DeserializePostProcess();

        auto* result_inserter_data = result.GetTile({1, 1}, TileLayer::entity)->GetUniqueData<proto::InserterData>();

        EXPECT_TRUE(result_inserter_data->dropoff.IsInitialized());
    }

    TEST_F(WorldDeserialize, CallOnDeserialize) {
        class MockWorldObject : public TestMockWorldObject
        {
        public:
            void OnDeserialize(World& /*world*/, const WorldCoord& coord, ChunkTile& tile) const override {
                EXPECT_EQ(coord.x, 5);
                EXPECT_EQ(coord.y, 6);
                ++onDeserializeCalled;

                chunkTile = &tile;
            }

            mutable int onDeserializeCalled = 0;
            mutable ChunkTile* chunkTile    = nullptr;
        };

        world_.EmplaceChunk({0, 0});

        data::PrototypeManager proto;
        data::UniqueDataManager unique;


        auto& mock_obj = proto.Make<MockWorldObject>();
        mock_obj.SetDimension({1, 2});

        auto* tile = world_.GetTile({5, 6}, TileLayer::entity);
        tile->SetPrototype(Orientation::up, &mock_obj);

        auto* tile2 = world_.GetTile({5, 7}, TileLayer::entity);
        tile2->SetPrototype(Orientation::up, &mock_obj);
        tile2->SetupMultiTile(1, *tile);

        data::active_prototype_manager   = &proto;
        data::active_unique_data_manager = &unique;
        proto.GenerateRelocationTable();

        // Must serialize and deserialize, as DeserializePostProcess expects the tiles
        // to be not linked to top left and have a multi-tile index set
        auto result = TestSerializeDeserialize(world_);
        result.DeserializePostProcess();

        // Was bug, causing incorrect mining drill dropoff location
        // should only call OnDeserialize for multi-tiles once at top left
        EXPECT_EQ(mock_obj.onDeserializeCalled, 1);
        EXPECT_EQ(mock_obj.chunkTile->GetPrototype(), tile->GetPrototype());
        EXPECT_EQ(mock_obj.chunkTile->GetMultiTileIndex(), 0);
    }

    TEST_F(WorldDeserialize, DeserializeLogicChunks) {
        world_.EmplaceChunk({0, 0});
        world_.LogicRegister(LogicGroup::inserter, {2, 3}, TileLayer::resource);
        const auto old_logic_object = world_.LogicGet(LogicGroup::inserter)[0];

        auto result             = TestSerializeDeserialize(world_);
        auto& result_logic_list = result.LogicGet(LogicGroup::inserter);

        ASSERT_EQ(result_logic_list.size(), 1);
        EXPECT_EQ(old_logic_object, result_logic_list[0]);
    }
} // namespace jactorio::game
