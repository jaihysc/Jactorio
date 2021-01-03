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

        J_NODISCARD SpriteSetT OnRGetSpriteSet(Orientation /*orientation*/,
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
                     Orientation orientation) const override {}

        void OnRemove(game::WorldData& world_data,
                      game::LogicData& logic_data,
                      const WorldCoord& world_coords,
                      game::ChunkTileLayer& tile_layer) const override {}
    };
    static_assert(!std::is_abstract_v<TestMockEntity>);


    ///
    /// Creates prototype with provided orientation and multi tile properties, attaches it to ctl to allow multi tile
    /// properties to be accessed
    inline void TestSetupMultiTileProp(game::ChunkTileLayer& ctl,
                                       const Orientation orientation,
                                       const game::MultiTileData& mt_data,
                                       proto::FWorldObject& proto) {
        proto.SetWidth(mt_data.span);
        proto.SetHeight(mt_data.height);
        ctl.SetPrototype(orientation, &proto);
    }

    ///
    /// Sets up a multi tile with proto at coord on the provided specified tile layer
    /// \tparam SetTopLeftLayer If false, non top left multi tiles will not know the top left layer
    /// \return Top left tile
    template <bool SetTopLeftLayer = true>
    game::ChunkTileLayer& TestSetupMultiTile(game::WorldData& world,
                                             const WorldCoord& coord,
                                             const game::TileLayer tile_layer,
                                             const Orientation orientation,
                                             proto::FWorldObject& proto,
                                             const game::MultiTileData& mt_data) {

        auto& origin_layer = world.GetTile(coord)->GetLayer(tile_layer);
        TestSetupMultiTileProp(origin_layer, orientation, mt_data, proto);

        for (int y = 0; y < proto.GetHeight(); ++y) {
            for (int x = 0; x < proto.GetWidth(); ++x) {
                if (x == 0 && y == 0)
                    continue;

                auto& layer = world.GetTile(coord.x + x, coord.y + y)->GetLayer(tile_layer);

                layer.SetPrototype(orientation, &proto);
                layer.SetMultiTileIndex(y * proto.GetWidth() + x);

                if constexpr (SetTopLeftLayer) {
                    layer.SetTopLeftLayer(origin_layer);
                }
            }
        }

        return origin_layer;
    }

    ///
    /// Creates a container of provided size at coord
    inline game::ChunkTileLayer& TestSetupContainer(game::WorldData& world,
                                                    const WorldCoord& coord,
                                                    const Orientation orientation,
                                                    const proto::ContainerEntity& container_entity,
                                                    const int container_capacity = 10) {
        auto& container_layer = world.GetTile(coord)->GetLayer(game::TileLayer::entity);

        container_layer.SetPrototype(orientation, &container_entity);
        container_layer.MakeUniqueData<proto::ContainerEntityData>(container_capacity);

        return container_layer;
    }

    ///
    /// Creates an inserter at coord
    inline game::ChunkTileLayer& TestSetupInserter(game::WorldData& world,
                                                   game::LogicData& logic,
                                                   const WorldCoord& coord,
                                                   const Orientation orientation,
                                                   const proto::Inserter& inserter_proto) {
        auto& layer = world.GetTile(coord)->GetLayer(game::TileLayer::entity);

        layer.SetPrototype(orientation, &inserter_proto);
        inserter_proto.OnBuild(world, logic, coord, layer, orientation);

        return layer;
    }

    ///
    /// Creates conveyor at coord, registers tile for logic updates
    inline void TestCreateConveyorSegment(game::WorldData& world,
                                          const WorldCoord& coord,
                                          const std::shared_ptr<game::ConveyorStruct>& con_struct_p,
                                          const proto::Conveyor& con_proto) {
        auto* tile = world.GetTile(coord);
        assert(tile != nullptr);
        auto* chunk = world.GetChunkW(coord);
        assert(chunk != nullptr);

        auto& layer = tile->GetLayer(game::TileLayer::entity);
        layer.SetPrototype(con_struct_p->direction, &con_proto);

        layer.MakeUniqueData<proto::ConveyorData>(con_struct_p);

        chunk->GetLogicGroup(game::Chunk::LogicGroup::conveyor).emplace_back(&tile->GetLayer(game::TileLayer::entity));
    }

    ///
    /// Creates a 2x2 multi tile assembly machine at coordinates
    /// \return top left layer
    inline game::ChunkTileLayer& TestSetupAssemblyMachine(game::WorldData& world,
                                                          const WorldCoord& coord,
                                                          const Orientation orientation,
                                                          proto::AssemblyMachine& assembly_proto) {
        auto& origin_layer =
            TestSetupMultiTile(world, coord, game::TileLayer::entity, orientation, assembly_proto, {2, 2});
        origin_layer.MakeUniqueData<proto::AssemblyMachineData>();
        return origin_layer;
    }

    ///
    /// Creates resource with orientation up at coord
    inline game::ChunkTile& TestSetupResource(game::WorldData& world,
                                              const WorldCoord& coord,
                                              proto::ResourceEntity& resource,
                                              const proto::ResourceEntityData::ResourceCount resource_amount) {

        game::ChunkTile* tile = world.GetTile(coord);
        assert(tile != nullptr);

        auto& resource_layer = tile->GetLayer(game::TileLayer::resource);
        resource_layer.SetPrototype(Orientation::up, &resource);
        resource_layer.MakeUniqueData<proto::ResourceEntityData>(resource_amount);

        return *tile;
    }

    ///
    /// Creates a drill in the world with orientation, calling OnBuild
    inline game::ChunkTile& TestSetupDrill(game::WorldData& world,
                                           game::LogicData& logic,
                                           const WorldCoord& coord,
                                           const Orientation orientation,
                                           proto::ResourceEntity& resource,
                                           proto::MiningDrill& drill,
                                           const proto::ResourceEntityData::ResourceCount resource_amount = 100) {
        auto* tile = world.GetTile(coord);
        assert(tile != nullptr);

        auto& layer = tile->GetLayer(game::TileLayer::entity);

        // Resource needed for OnBuild
        TestSetupResource(world, coord, resource, resource_amount);

        layer.SetPrototype(orientation, &drill);
        drill.OnBuild(world, logic, coord, layer, orientation);

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
