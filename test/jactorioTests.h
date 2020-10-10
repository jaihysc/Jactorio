// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_TEST_JACTORIOTESTS_H
#define JACTORIO_TEST_JACTORIOTESTS_H
#pragma once

#include <fstream>

#include "core/data_type.h"

#include "data/prototype_manager.h"
#include "proto/abstract/conveyor.h"
#include "proto/assembly_machine.h"
#include "proto/container_entity.h"
#include "proto/inserter.h"
#include "proto/mining_drill.h"
#include "proto/recipe.h"
#include "proto/resource_entity.h"
#include "proto/sprite.h"

#include "game/logic/logic_data.h"
#include "game/world/world_data.h"

#include <cereal/archives/portable_binary.hpp>

namespace jactorio
{
    // It is difficult to compute enough decimal points by hand for EXPECT_DOUBLE_EQ, thus EXPECT_NEAR is used
    constexpr double kFloatingAbsErr = 0.000000001;

    ///
    /// Inherit and override what is necessary
    class TestMockWorldObject : public proto::FWorldObject
    {
    public:
        PROTOTYPE_CATEGORY(test);

        void PostLoadValidate(const data::PrototypeManager& /*proto_manager*/) const override {}

        J_NODISCARD proto::Sprite* OnRGetSprite(SpriteSetT /*set*/) const override {
            return nullptr;
        }

        J_NODISCARD SpriteSetT OnRGetSpriteSet(proto::Orientation /*orientation*/,
                                               game::WorldData& /*world_data*/,
                                               const WorldCoord& /*world_coords*/) const override {
            return 0;
        }

        J_NODISCARD SpriteFrameT OnRGetSpriteFrame(const proto::UniqueDataBase& /*unique_data*/,
                                                   GameTickT /*game_tick*/) const override {
            return 0;
        }

        bool OnRShowGui(const render::GuiRenderer& /*g_rendr*/, game::ChunkTileLayer* /*tile_layer*/) const override {
            return false;
        }

        void OnDeserialize(game::WorldData& world_data,
                           const WorldCoord& world_coord,
                           game::ChunkTileLayer& tile_layer) const override {}
    };

    static_assert(!std::is_abstract_v<TestMockWorldObject>);


    class TestMockEntity : public proto::Entity
    {
    public:
        PROTOTYPE_CATEGORY(test);

        void OnBuild(game::WorldData& world_data,
                     game::LogicData& logic_data,
                     const WorldCoord& world_coords,
                     game::ChunkTileLayer& tile_layer,
                     proto::Orientation orientation) const override {}

        void OnRemove(game::WorldData& world_data,
                      game::LogicData& logic_data,
                      const WorldCoord& world_coords,
                      game::ChunkTileLayer& tile_layer) const override {}
    };
    static_assert(!std::is_abstract_v<TestMockEntity>);


    inline void TestSetupMultiTileProp(game::ChunkTileLayer& ctl,
                                       const game::MultiTileData& mt_data,
                                       proto::FWorldObject& proto) {
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
                                             proto::FWorldObject& proto,
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
                                                    const proto::ContainerEntity& container_entity,
                                                    const int container_capacity = 10) {
        auto& container_layer = world_data.GetTile(world_coords)->GetLayer(game::TileLayer::entity);

        container_layer.prototypeData = &container_entity;
        container_layer.MakeUniqueData<proto::ContainerEntityData>(container_capacity);

        return container_layer;
    }

    ///
    /// Creates an inserter at coordinates
    inline game::ChunkTileLayer& TestSetupInserter(game::WorldData& world_data,
                                                   game::LogicData& logic_data,
                                                   const WorldCoord& world_coords,
                                                   const proto::Inserter& inserter_proto,
                                                   const proto::Orientation orientation) {
        using namespace jactorio;

        auto& layer = world_data.GetTile(world_coords)->GetLayer(game::TileLayer::entity);

        layer.prototypeData = &inserter_proto;
        inserter_proto.OnBuild(world_data, logic_data, world_coords, layer, orientation);

        return layer;
    }

    ///
    /// Registers and creates tile UniqueData for ConveyorSegment
    inline void TestRegisterConveyorSegment(game::WorldData& world_data,
                                             const WorldCoord& world_coords,
                                             const std::shared_ptr<game::ConveyorSegment>& segment,
                                             const proto::Conveyor& prototype) {
        auto* tile = world_data.GetTile(world_coords);
        assert(tile);
        auto* chunk = world_data.GetChunkW(world_coords);
        assert(chunk);

        auto& layer         = tile->GetLayer(game::TileLayer::entity);
        layer.prototypeData = &prototype;

        layer.MakeUniqueData<proto::ConveyorData>(segment);

        chunk->GetLogicGroup(game::Chunk::LogicGroup::conveyor)
            .emplace_back(&tile->GetLayer(game::TileLayer::entity));
    }

    ///
    /// Creates a 2x2 multi tile assembly machine at coordinates
    /// \return top left layer
    inline game::ChunkTileLayer& TestSetupAssemblyMachine(game::WorldData& world_data,
                                                          const WorldCoord& world_coords,
                                                          proto::AssemblyMachine& assembly_proto) {
        auto& origin_layer =
            TestSetupMultiTile(world_data, assembly_proto, world_coords, game::TileLayer::entity, {2, 2});
        origin_layer.MakeUniqueData<proto::AssemblyMachineData>();
        return origin_layer;
    }

    inline game::ChunkTile& TestSetupResource(game::WorldData& world_data,
                                              const WorldCoord& world_coord,
                                              proto::ResourceEntity& resource,
                                              const proto::ResourceEntityData::ResourceCount resource_amount) {

        game::ChunkTile* tile = world_data.GetTile(world_coord);
        assert(tile);

        auto& resource_layer         = tile->GetLayer(game::TileLayer::resource);
        resource_layer.prototypeData = &resource;
        resource_layer.MakeUniqueData<proto::ResourceEntityData>(resource_amount);

        return *tile;
    }

    ///
    /// Creates a drill in the world, calling OnBuild
    inline game::ChunkTile& TestSetupDrill(game::WorldData& world_data,
                                           game::LogicData& logic_data,
                                           const WorldCoord& world_coord,
                                           const proto::Orientation orientation,
                                           proto::ResourceEntity& resource,
                                           proto::MiningDrill& drill,
                                           const proto::ResourceEntityData::ResourceCount resource_amount = 100) {
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
        proto::Recipe* recipe    = nullptr;
        proto::Item* item1       = nullptr;
        proto::Item* item2       = nullptr;
        proto::Item* itemProduct = nullptr;
    };

    ///
    /// Sets up and registers a recipe
    /// 1a + 1b = 1c
    J_NODISCARD inline auto TestSetupRecipe(data::PrototypeManager& proto_manager) {
        TestSetupRecipeReturn rt;

        rt.recipe = &proto_manager.AddProto<proto::Recipe>();

        rt.recipe->craftingTime = 1.f;
        rt.recipe->ingredients  = {{"__test/r-ingredient-1", 1}, {"__test/r-ingredient-2", 1}};
        rt.recipe->product      = {"__test/r-product", 1};

        rt.item1       = &proto_manager.AddProto<proto::Item>("__test/r-ingredient-1");
        rt.item2       = &proto_manager.AddProto<proto::Item>("__test/r-ingredient-2");
        rt.itemProduct = &proto_manager.AddProto<proto::Item>("__test/r-product");

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
