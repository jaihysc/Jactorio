// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_TEST_JACTORIOTESTS_H
#define JACTORIO_TEST_JACTORIOTESTS_H
#pragma once

#include "core/data_type.h"
#include "data/prototype_manager.h"
#include "data/prototype/entity/assembly_machine.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/inserter.h"
#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/entity/transport_line.h"
#include "game/world/world_data.h"

namespace jactorio
{
	///
	/// \brief Makes and registers prototype with prototype manager
	template <typename TPrototype>
	TPrototype* MakeRegisterPrototype(data::PrototypeManager& prototype_manager, const std::string& iname = "") {
		auto unique = std::make_unique<TPrototype>();
		auto ptr    = unique.get();
		prototype_manager.DataRawAdd(iname, unique.release());

		return ptr;
	}

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

	///
	/// \brief Creates a drill in the world, calling OnBuild
	inline game::ChunkTile& TestSetupDrill(game::WorldData& world_data,
	                                       game::LogicData& logic_data,
	                                       const WorldCoord& world_coord,
	                                       data::ResourceEntity& resource,
	                                       data::MiningDrill& drill) {

		game::ChunkTile* tile = world_data.GetTile(world_coord);

		// Resource needed for OnBuild
		tile->GetLayer(game::ChunkTile::ChunkLayer::resource).prototypeData = &resource;
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

		auto i_1 = std::make_unique<data::Item>();
		rt.item1 = i_1.get();

		auto i_2 = std::make_unique<data::Item>();
		rt.item2 = i_2.get();

		auto item_3    = std::make_unique<data::Item>();
		rt.itemProduct = item_3.get();

		proto_manager.DataRawAdd("@1", i_1.release());
		proto_manager.DataRawAdd("@2", i_2.release());
		proto_manager.DataRawAdd("@3", item_3.release());

		return rt;
	}
}

#endif // JACTORIO_TEST_JACTORIOTESTS_H
