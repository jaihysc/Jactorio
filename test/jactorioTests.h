// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_TEST_JACTORIOTESTS_H
#define JACTORIO_TEST_JACTORIOTESTS_H
#pragma once

#include "core/data_type.h"
#include "data/prototype_manager.h"
#include "data/prototype/assembly_machine.h"
#include "data/prototype/container_entity.h"
#include "data/prototype/inserter.h"
#include "data/prototype/mining_drill.h"
#include "data/prototype/resource_entity.h"
#include "data/prototype/abstract_proto/transport_line.h"
#include "game/world/world_data.h"

namespace jactorio
{
	// It is difficult to compute enough decimal points by hand for EXPECT_DOUBLE_EQ, thus EXPECT_NEAR is used
	constexpr double kFloatingAbsErr = 0.000000001;

	///
	/// \brief Creates a container of size 10 at coordinates
	inline game::ChunkTileLayer& TestSetupContainer(game::WorldData& world_data,
	                                                const WorldCoord& world_coords,
	                                                const data::ContainerEntity& container_entity,
	                                                const int container_capacity = 10) {
		auto& container_layer = world_data.GetTile(world_coords)
		                                  ->GetLayer(game::ChunkTile::ChunkLayer::entity);

		container_layer.prototypeData = &container_entity;
		container_layer.MakeUniqueData<data::ContainerEntityData>(container_capacity);

		return container_layer;
	}

	///
	/// \brief Creates an inserter at coordinates
	inline game::ChunkTileLayer& TestSetupInserter(game::WorldData& world_data,
	                                               game::LogicData& logic_data,
	                                               const WorldCoord& world_coords,
	                                               const data::Inserter& inserter_proto,
	                                               const data::Orientation orientation) {
		using namespace jactorio;

		auto& layer = world_data.GetTile(world_coords)->GetLayer(game::ChunkTile::ChunkLayer::entity);

		layer.prototypeData = &inserter_proto;
		inserter_proto.OnBuild(world_data, logic_data, world_coords, layer, orientation);

		return layer;
	}

	///
	/// \brief Registers and creates tile UniqueData for TransportSegment
	inline void TestRegisterTransportSegment(game::WorldData& world_data,
	                                         const WorldCoord& world_coords,
	                                         const std::shared_ptr<game::TransportSegment>& segment,
	                                         const data::TransportLine& prototype) {
		auto* tile = world_data.GetTile(world_coords);
		assert(tile);
		auto* chunk = world_data.GetChunkW(world_coords);
		assert(chunk);

		auto& layer         = tile->GetLayer(game::ChunkTile::ChunkLayer::entity);
		layer.prototypeData = &prototype;

		layer.MakeUniqueData<data::TransportLineData>(segment);

		chunk->GetLogicGroup(game::Chunk::LogicGroup::transport_line)
		     .emplace_back(&tile->GetLayer(game::ChunkTile::ChunkLayer::entity));
	}

	///
	/// \brief Creates a 2x2 multi tile assembly machine at coordinates
	/// \return top left layer
	inline game::ChunkTileLayer& TestSetupAssemblyMachine(game::WorldData& world_data,
	                                                      const WorldCoord& world_coords,
	                                                      const data::AssemblyMachine& assembly_proto) {
		auto& origin_layer = world_data.GetTile(world_coords)->GetLayer(game::ChunkTile::ChunkLayer::entity);
		origin_layer.InitMultiTileProp(2, 2);

		for (int y = 0; y < 2; ++y) {
			for (int x = 0; x < 2; ++x) {
				if (x == 0 && y == 0)
					continue;

				auto& layer = world_data.GetTile(world_coords.x + x, world_coords.y + y)
				                        ->GetLayer(game::ChunkTile::ChunkLayer::entity);

				layer.prototypeData  = &assembly_proto;
				layer.multiTileIndex = y * 2 + x;
				layer.SetMultiTileParent(&origin_layer);
			}
		}

		origin_layer.MakeUniqueData<data::AssemblyMachineData>();

		return origin_layer;
	}

	inline game::ChunkTile& TestSetupResource(game::WorldData& world_data,
	                                          const WorldCoord& world_coord,
	                                          data::ResourceEntity& resource,
	                                          const data::ResourceEntityData::ResourceCount resource_amount) {

		game::ChunkTile* tile = world_data.GetTile(world_coord);
		assert(tile);

		auto& resource_layer         = tile->GetLayer(game::ChunkTile::ChunkLayer::resource);
		resource_layer.prototypeData = &resource;
		resource_layer.MakeUniqueData<data::ResourceEntityData>(resource_amount);

		return *tile;
	}

	///
	/// \brief Creates a drill in the world, calling OnBuild
	inline game::ChunkTile& TestSetupDrill(game::WorldData& world_data,
	                                       game::LogicData& logic_data,
	                                       const WorldCoord& world_coord,
	                                       data::ResourceEntity& resource,
	                                       data::MiningDrill& drill,
	                                       const data::ResourceEntityData::ResourceCount resource_amount = 100) {
		game::ChunkTile* tile = world_data.GetTile(world_coord);
		assert(tile);

		// Resource needed for OnBuild
		TestSetupResource(world_data, world_coord, resource, resource_amount);

		drill.OnBuild(world_data, logic_data,
		              world_coord, tile->GetLayer(game::ChunkTile::ChunkLayer::entity),
		              data::Orientation::right);

		return *tile;
	}


	struct TestSetupRecipeReturn
	{
		data::Recipe recipe{};
		data::Item* item1       = nullptr;
		data::Item* item2       = nullptr;
		data::Item* itemProduct = nullptr;
	};

	///
	/// \brief Sets up and registers a recipe
	/// 1a + 1b = 1c
	J_NODISCARD inline auto TestSetupRecipe(data::PrototypeManager& proto_manager) {
		TestSetupRecipeReturn rt{};

		rt.recipe.craftingTime = 1.f;
		rt.recipe.ingredients  = {{"@1", 1}, {"@2", 1}};
		rt.recipe.product      = {"@3", 1};

		rt.item1 = &proto_manager.AddProto<data::Item>("@1");
		rt.item2 = &proto_manager.AddProto<data::Item>("@2");
		rt.itemProduct = &proto_manager.AddProto<data::Item>("@3");

		return rt;
	}
}

#endif // JACTORIO_TEST_JACTORIOTESTS_H
