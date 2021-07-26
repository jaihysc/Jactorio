// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_TEST_JACTORIOTESTS_H
#define JACTORIO_TEST_JACTORIOTESTS_H
#pragma once

#include <fstream>
#include <type_traits>

#include "core/data_type.h"

#include "data/prototype_manager.h"
#include "proto/abstract/conveyor.h"
#include "proto/assembly_machine.h"
#include "proto/container_entity.h"
#include "proto/inserter.h"
#include "proto/mining_drill.h"
#include "proto/recipe.h"
#include "proto/resource_entity.h"
#include "proto/splitter.h"
#include "proto/sprite.h"

#include "game/logic/logic.h"
#include "game/world/world.h"

#include <cereal/archives/portable_binary.hpp>

namespace jactorio
{
    // It is difficult to compute enough decimal points by hand for EXPECT_DOUBLE_EQ, thus EXPECT_NEAR is used
    constexpr double kFloatingAbsErr = 0.000000001;

    /// Inherit and override what is necessary
    class TestMockWorldObject : public proto::FWorldObject
    {
    public:
        PROTOTYPE_CATEGORY(test);

        void PostLoadValidate(const data::PrototypeManager& /*proto*/) const override {}

        bool OnRShowGui(const gui::Context& /*context*/, game::ChunkTile* /*tile*/) const override {
            return false;
        }

        void OnDeserialize(game::World& world, const WorldCoord& coord, game::ChunkTile& tile) const override {}
    };

    static_assert(!std::is_abstract_v<TestMockWorldObject>);


    class TestMockEntity : public proto::Entity
    {
    public:
        PROTOTYPE_CATEGORY(test);

        void OnBuild(game::World& world,
                     game::Logic& logic,
                     const WorldCoord& coord,
                     Orientation orientation) const override {}

        void OnRemove(game::World& world, game::Logic& logic, const WorldCoord& coord) const override {}
    };
    static_assert(!std::is_abstract_v<TestMockEntity>);


    /// Sets up a multi tile with dimensions specified by proto, coord is top left, on the provided specified tile
    /// \return Top left tile
    inline game::ChunkTile& TestSetupMultiTile(game::World& world,
                                               const WorldCoord& coord,
                                               const game::TileLayer tlayer,
                                               const Orientation orientation,
                                               const proto::FWorldObject& proto) {

        auto* tl_tile = world.GetTile(coord, tlayer);
        assert(tl_tile != nullptr);

        tl_tile->SetPrototype(orientation, proto);

        for (int y = 0; y < proto.GetHeight(orientation); ++y) {
            for (int x = 0; x < proto.GetWidth(orientation); ++x) {
                if (x == 0 && y == 0)
                    continue;

                auto* tile = world.GetTile({coord.x + x, coord.y + y}, tlayer);
                assert(tile != nullptr);

                tile->SetPrototype(orientation, &proto);
                tile->SetupMultiTile(y * proto.GetWidth(orientation) + x, *tl_tile);
            }
        }

        return *tl_tile;
    }

    /// Creates a container of provided size at coord
    inline game::ChunkTile& TestSetupContainer(game::World& world,
                                               const WorldCoord& coord,
                                               const Orientation orientation,
                                               const proto::ContainerEntity& container_entity,
                                               const int container_capacity = 10) {
        auto& container_layer =
            TestSetupMultiTile(world, coord, game::TileLayer::entity, orientation, container_entity);

        container_layer.SetPrototype(orientation, container_entity);
        container_layer.MakeUniqueData<proto::ContainerEntityData>(container_capacity);

        return container_layer;
    }

    /// Creates an inserter at coord using OnBuild (Registers for logic updates)
    inline game::ChunkTile& TestSetupInserter(game::World& world,
                                              game::Logic& logic,
                                              const WorldCoord& coord,
                                              const Orientation orientation,
                                              const proto::Inserter& inserter_proto) {
        auto* tile = world.GetTile(coord, game::TileLayer::entity);
        assert(tile != nullptr);

        tile->SetPrototype(orientation, &inserter_proto);
        inserter_proto.OnBuild(world, logic, coord, orientation);

        return *tile;
    }

    /// Creates conveyor at coord, registers tile for logic updates
    inline void TestCreateConveyorSegment(game::World& world,
                                          const WorldCoord& coord,
                                          const std::shared_ptr<game::ConveyorStruct>& con_struct_p,
                                          const proto::Conveyor& con_proto) {
        auto* chunk = world.GetChunkW(coord);
        assert(chunk != nullptr);

        auto* tile = world.GetTile(coord, game::TileLayer::entity);
        assert(tile != nullptr);

        tile->SetPrototype(con_struct_p->direction, &con_proto);
        tile->MakeUniqueData<proto::ConveyorData>(con_struct_p);

        world.LogicRegister(game::LogicGroup::conveyor, coord, game::TileLayer::entity);
    }

    /// Creates a assembly machine at coordinates
    /// \return top left tile
    inline game::ChunkTile& TestSetupAssemblyMachine(game::World& world,
                                                     const WorldCoord& coord,
                                                     const Orientation orientation,
                                                     proto::AssemblyMachine& assembly_proto) {
        auto& origin_layer = TestSetupMultiTile(world, coord, game::TileLayer::entity, orientation, assembly_proto);
        origin_layer.MakeUniqueData<proto::AssemblyMachineData>();
        return origin_layer;
    }

    /// Creates resource with orientation up at coord
    inline game::ChunkTile& TestSetupResource(game::World& world,
                                              const WorldCoord& coord,
                                              proto::ResourceEntity& resource,
                                              const proto::ResourceEntityData::ResourceCount resource_amount) {

        auto* tile = world.GetTile(coord, game::TileLayer::resource);
        assert(tile != nullptr);

        tile->SetPrototype(Orientation::up, &resource);
        tile->MakeUniqueData<proto::ResourceEntityData>(resource_amount);

        return *tile;
    }

    /// Creates a drill in the world with orientation using OnBuild
    /// \remark Ensure a resource exists or drill will not get built
    inline game::ChunkTile& TestSetupDrill(game::World& world,
                                           game::Logic& logic,
                                           const WorldCoord& coord,
                                           const Orientation orientation,
                                           proto::MiningDrill& drill) {
        auto* tile = world.GetTile(coord, game::TileLayer::entity);
        assert(tile != nullptr);

        tile->SetPrototype(orientation, &drill);
        drill.OnBuild(world, logic, coord, orientation);

        return *tile;
    }

    // TODO be more consistent in ALWAYS calling on build to create these
    /// Creates conveyor at tile with provided conveyor structure
    inline auto& TestSetupConveyor(game::World& world,
                                   const WorldCoord& coord,
                                   const proto::Conveyor& con_proto,
                                   const std::shared_ptr<game::ConveyorStruct>& con_struct_p) {

        auto* tile = world.GetTile(coord, game::TileLayer::entity);
        assert(tile != nullptr);

        tile->SetPrototype(con_struct_p->direction, con_proto);
        return tile->MakeUniqueData<proto::ConveyorData>(con_struct_p);
    }

    /// Creates conveyor at tile its own conveyor structure, dimensions specified by con_proto, coord is top left
    inline auto& TestSetupConveyor(
        game::World& world,
        const WorldCoord& coord,
        const Orientation orien,
        const proto::Conveyor& con_proto,
        const game::ConveyorStruct::TerminationType ttype = game::ConveyorStruct::TerminationType::straight,
        const std::uint8_t len                            = 1) {

        const auto con_struct = std::make_shared<game::ConveyorStruct>(orien, ttype, len);

        return TestSetupConveyor(world, coord, con_proto, con_struct);
    }

    /// Creates splitter data at tile, no conveyor structure
    inline auto& TestSetupBlankSplitter(game::World& world,
                                        const WorldCoord& coord,
                                        const Orientation orien,
                                        const proto::Splitter& splitter) {
        auto& top_left = TestSetupMultiTile(world, coord, game::TileLayer::entity, orien, splitter);
        return top_left.MakeUniqueData<proto::SplitterData>(orien);
    }

    /// Creates splitter data at tile with conveyor structures
    inline auto& TestSetupSplitter(game::World& world,
                                   const WorldCoord& coord,
                                   const Orientation orien,
                                   const proto::Splitter& splitter) {

        auto& splitter_data = TestSetupBlankSplitter(world, coord, orien, splitter);

        splitter_data.left.structure =
            std::make_shared<game::ConveyorStruct>(orien, game::ConveyorStruct::TerminationType::straight, 1);

        splitter_data.right.structure =
            std::make_shared<game::ConveyorStruct>(orien, game::ConveyorStruct::TerminationType::straight, 1);

        return splitter_data;
    }


    struct TestSetupRecipeReturn
    {
        proto::Recipe* recipe    = nullptr;
        proto::Item* item1       = nullptr;
        proto::Item* item2       = nullptr;
        proto::Item* itemProduct = nullptr;
    };

    /// Sets up and registers a recipe
    /// 1a + 1b = 1c
    J_NODISCARD inline auto TestSetupRecipe(data::PrototypeManager& proto) {
        TestSetupRecipeReturn rt;

        rt.recipe = &proto.Make<proto::Recipe>();

        rt.recipe->craftingTime = 1.f;
        rt.recipe->ingredients  = {{"__test/r-ingredient-1", 1}, {"__test/r-ingredient-2", 1}};
        rt.recipe->product      = {"__test/r-product", 1};

        rt.item1       = &proto.Make<proto::Item>("__test/r-ingredient-1");
        rt.item2       = &proto.Make<proto::Item>("__test/r-ingredient-2");
        rt.itemProduct = &proto.Make<proto::Item>("__test/r-product");

        return rt;
    }


    /// Serializes T
    template <typename T, typename TArchiver = cereal::PortableBinaryOutputArchive>
    void TestSerialize(const T& object) {
        constexpr auto save_file = "savegame.dat";

        std::ofstream out_cereal_stream(save_file, std::ios_base::binary);
        TArchiver output_archive(out_cereal_stream);
        output_archive(object);
    }

    /// Returns deserialized T into provided object
    template <typename T, typename TArchiver = cereal::PortableBinaryInputArchive>
    void TestDeserialize(T& out) {
        constexpr auto save_file = "savegame.dat";

        std::ifstream in_cereal_stream(save_file, std::ios_base::binary);
        TArchiver iarchive(in_cereal_stream);

        iarchive(out);
    }

    /// Returns deserialized T
    template <typename T, typename TArchiver = cereal::PortableBinaryInputArchive>
    J_NODISCARD T TestDeserialize() {
        static_assert(std::is_default_constructible_v<T>);

        T deserialized_val;
        TestDeserialize<T, TArchiver>(deserialized_val);
        return std::move(deserialized_val); // Force a copy to catch bad copy/move constructors
    }


    /// Serializes T and returns deserialized T
    template <typename T>
    J_NODISCARD auto TestSerializeDeserialize(const T& object) {
        TestSerialize(object);
        return TestDeserialize<T>();
    }
} // namespace jactorio

#endif // JACTORIO_TEST_JACTORIOTESTS_H
