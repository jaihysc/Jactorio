// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_TEST_JACTORIOTESTS_H
#define JACTORIO_TEST_JACTORIOTESTS_H
#pragma once

#include <fstream>

#include "core/data_type.h"
#include "data/prototype/abstract_proto/transport_line.h"
#include "data/prototype/assembly_machine.h"
#include "data/prototype/container_entity.h"
#include "data/prototype/inserter.h"
#include "data/prototype/mining_drill.h"
#include "data/prototype/resource_entity.h"
#include "data/prototype_manager.h"
#include "game/world/world_data.h"

#include <cereal/archives/portable_binary.hpp>

namespace jactorio
{
    // It is difficult to compute enough decimal points by hand for EXPECT_DOUBLE_EQ, thus EXPECT_NEAR is used
    constexpr double kFloatingAbsErr = 0.000000001;

    ///
    /// Inherit and override what is necessary
    class TestMockWorldObject : public data::FWorldObject
    {
    public:
        PROTOTYPE_CATEGORY(test);

        void PostLoadValidate(const data::PrototypeManager&) const override {}

        J_NODISCARD data::Sprite* OnRGetSprite(data::Sprite::SetT set) const override {
            return nullptr;
        }

        J_NODISCARD data::Sprite::SetT OnRGetSpriteSet(data::Orientation orientation,
                                                       game::WorldData& world_data,
                                                       const WorldCoord& world_coords) const override {
            return 0;
        }

        J_NODISCARD data::Sprite::FrameT OnRGetSpriteFrame(const data::UniqueDataBase& unique_data,
                                                           GameTickT game_tick) const override {
            return 0;
        }

        bool OnRShowGui(GameWorlds& /*worlds*/,
                        game::LogicData& /*logic*/,
                        game::PlayerData& /*player*/,
                        const data::PrototypeManager& /*data_manager*/,
                        game::ChunkTileLayer* /*tile_layer*/) const override {
            return false;
        }

        void OnDeserialize(game::WorldData& world_data,
                           const WorldCoord& world_coord,
                           game::ChunkTileLayer& tile_layer) const override {}
    };

    static_assert(!std::is_abstract_v<TestMockWorldObject>);


    class TestMockEntity : public data::Entity
    {
    public:
        PROTOTYPE_CATEGORY(test);

        void OnBuild(game::WorldData& world_data,
                     game::LogicData& logic_data,
                     const WorldCoord& world_coords,
                     game::ChunkTileLayer& tile_layer,
                     data::Orientation orientation) const override {}

        void OnRemove(game::WorldData& world_data,
                      game::LogicData& logic_data,
                      const WorldCoord& world_coords,
                      game::ChunkTileLayer& tile_layer) const override {}
    };
    static_assert(!std::is_abstract_v<TestMockEntity>);


    inline void TestSetupMultiTileProp(game::ChunkTileLayer& ctl,
                                       const game::MultiTileData& mt_data,
                                       data::FWorldObject& proto) {
        proto.tileWidth   = mt_data.span;
        proto.tileHeight  = mt_data.height;
        ctl.prototypeData = &proto;
    }

    ///
    /// Sets up a multi tile with proto using provided properties
    /// \tparam SetTopLeftLayer If false, non top left multi tiles will not know the top left layer
    /// \return Top left tile
    template <bool SetTopLeftLayer = true>
    game::ChunkTileLayer& TestSetupMultiTile(game::WorldData& world_data,
                                             data::FWorldObject& proto,
                                             const WorldCoord& world_coord,
                                             const game::TileLayer tile_layer,
                                             const game::MultiTileData& mt_data) {

        auto& origin_layer = world_data.GetTile(world_coord)->GetLayer(tile_layer);
        TestSetupMultiTileProp(origin_layer, mt_data, proto);

        for (int y = 0; y < proto.tileHeight; ++y) {
            for (int x = 0; x < proto.tileWidth; ++x) {
                if (x == 0 && y == 0)
                    continue;

                auto& layer = world_data.GetTile(world_coord.x + x, world_coord.y + y)->GetLayer(tile_layer);

                layer.prototypeData = &proto;
                layer.SetMultiTileIndex(y * proto.tileWidth + x);

                if constexpr (SetTopLeftLayer) {
                    layer.SetTopLeftLayer(origin_layer);
                }
            }
        }

        return origin_layer;
    }

    ///
    /// Creates a container of size 10 at coordinates
    inline game::ChunkTileLayer& TestSetupContainer(game::WorldData& world_data,
                                                    const WorldCoord& world_coords,
                                                    const data::ContainerEntity& container_entity,
                                                    const int container_capacity = 10) {
        auto& container_layer = world_data.GetTile(world_coords)->GetLayer(game::TileLayer::entity);

        container_layer.prototypeData = &container_entity;
        container_layer.MakeUniqueData<data::ContainerEntityData>(container_capacity);

        return container_layer;
    }

    ///
    /// Creates an inserter at coordinates
    inline game::ChunkTileLayer& TestSetupInserter(game::WorldData& world_data,
                                                   game::LogicData& logic_data,
                                                   const WorldCoord& world_coords,
                                                   const data::Inserter& inserter_proto,
                                                   const data::Orientation orientation) {
        using namespace jactorio;

        auto& layer = world_data.GetTile(world_coords)->GetLayer(game::TileLayer::entity);

        layer.prototypeData = &inserter_proto;
        inserter_proto.OnBuild(world_data, logic_data, world_coords, layer, orientation);

        return layer;
    }

    ///
    /// Registers and creates tile UniqueData for TransportSegment
    inline void TestRegisterTransportSegment(game::WorldData& world_data,
                                             const WorldCoord& world_coords,
                                             const std::shared_ptr<game::TransportSegment>& segment,
                                             const data::TransportLine& prototype) {
        auto* tile = world_data.GetTile(world_coords);
        assert(tile);
        auto* chunk = world_data.GetChunkW(world_coords);
        assert(chunk);

        auto& layer         = tile->GetLayer(game::TileLayer::entity);
        layer.prototypeData = &prototype;

        layer.MakeUniqueData<data::TransportLineData>(segment);

        chunk->GetLogicGroup(game::Chunk::LogicGroup::transport_line)
            .emplace_back(&tile->GetLayer(game::TileLayer::entity));
    }

    ///
    /// Creates a 2x2 multi tile assembly machine at coordinates
    /// \return top left layer
    inline game::ChunkTileLayer& TestSetupAssemblyMachine(game::WorldData& world_data,
                                                          const WorldCoord& world_coords,
                                                          data::AssemblyMachine& assembly_proto) {
        auto& origin_layer =
            TestSetupMultiTile(world_data, assembly_proto, world_coords, game::TileLayer::entity, {2, 2});
        origin_layer.MakeUniqueData<data::AssemblyMachineData>();
        return origin_layer;
    }

    inline game::ChunkTile& TestSetupResource(game::WorldData& world_data,
                                              const WorldCoord& world_coord,
                                              data::ResourceEntity& resource,
                                              const data::ResourceEntityData::ResourceCount resource_amount) {

        game::ChunkTile* tile = world_data.GetTile(world_coord);
        assert(tile);

        auto& resource_layer         = tile->GetLayer(game::TileLayer::resource);
        resource_layer.prototypeData = &resource;
        resource_layer.MakeUniqueData<data::ResourceEntityData>(resource_amount);

        return *tile;
    }

    ///
    /// Creates a drill in the world, calling OnBuild
    inline game::ChunkTile& TestSetupDrill(game::WorldData& world_data,
                                           game::LogicData& logic_data,
                                           const WorldCoord& world_coord,
                                           const data::Orientation orientation,
                                           data::ResourceEntity& resource,
                                           data::MiningDrill& drill,
                                           const data::ResourceEntityData::ResourceCount resource_amount = 100) {
        auto* tile = world_data.GetTile(world_coord);
        assert(tile);

        auto& layer = tile->GetLayer(game::TileLayer::entity);

        // Resource needed for OnBuild
        TestSetupResource(world_data, world_coord, resource, resource_amount);

        layer.prototypeData = &drill;
        drill.OnBuild(world_data, logic_data, world_coord, layer, orientation);

        return *tile;
    }


    struct TestSetupRecipeReturn
    {
        data::Recipe* recipe    = nullptr;
        data::Item* item1       = nullptr;
        data::Item* item2       = nullptr;
        data::Item* itemProduct = nullptr;
    };

    ///
    /// Sets up and registers a recipe
    /// 1a + 1b = 1c
    J_NODISCARD inline auto TestSetupRecipe(data::PrototypeManager& proto_manager) {
        TestSetupRecipeReturn rt;

        rt.recipe = &proto_manager.AddProto<data::Recipe>();

        rt.recipe->craftingTime = 1.f;
        rt.recipe->ingredients  = {{"__test/r-ingredient-1", 1}, {"__test/r-ingredient-2", 1}};
        rt.recipe->product      = {"__test/r-product", 1};

        rt.item1       = &proto_manager.AddProto<data::Item>("__test/r-ingredient-1");
        rt.item2       = &proto_manager.AddProto<data::Item>("__test/r-ingredient-2");
        rt.itemProduct = &proto_manager.AddProto<data::Item>("__test/r-product");

        return rt;
    }


    ///
    /// Serializes T and returns deserialized T
    template <typename T>
    J_NODISCARD T TestSerializeDeserialize(const T& object) {
        constexpr auto save_file = "savegame.dat";

        {
            std::ofstream out_cereal_stream(save_file, std::ios_base::binary);
            cereal::PortableBinaryOutputArchive output_archive(out_cereal_stream);
            output_archive(object);
        } // Must go out of scope to flush

        std::ifstream in_cereal_stream(save_file, std::ios_base::binary);
        cereal::PortableBinaryInputArchive iarchive(in_cereal_stream);

        T deserialized_val;
        iarchive(deserialized_val);
        return deserialized_val;
    }
} // namespace jactorio

#endif // JACTORIO_TEST_JACTORIOTESTS_H
