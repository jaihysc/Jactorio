// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 06/18/2020

#ifndef JACTORIO_TEST_JACTORIOTESTS_H
#define JACTORIO_TEST_JACTORIOTESTS_H
#pragma once

#include "data/data_manager.h"
#include "data/prototype/entity/assembly_machine.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/inserter.h"
#include "data/prototype/entity/transport_line.h"
#include "game/logic/placement_controller.h"
#include "game/world/world_data.h"

///
/// \brief Creates a container of size 10 at coordinates
inline jactorio::game::ChunkTileLayer& TestSetupContainer(jactorio::game::WorldData& world_data,
                                                          const jactorio::game::WorldData::WorldPair& world_coords,
                                                          const jactorio::data::ContainerEntity& container_entity) {
	auto& container_layer = world_data.GetTile(world_coords)
	                                  ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);

	container_layer.prototypeData = &container_entity;
	container_layer.MakeUniqueData<jactorio::data::ContainerEntityData>(10);

	return container_layer;
}

///
/// \brief Creates an inserter at coordinates
inline jactorio::game::ChunkTileLayer& TestSetupInserter(jactorio::game::WorldData& world_data,
                                                         jactorio::game::LogicData& logic_data,
                                                         const jactorio::game::WorldData::WorldPair& world_coords,
                                                         const jactorio::data::Inserter& inserter_proto,
                                                         const jactorio::data::Orientation orientation) {
	using namespace jactorio;

	auto& layer = world_data.GetTile(world_coords)->GetLayer(game::ChunkTile::ChunkLayer::entity);

	layer.prototypeData = &inserter_proto;
	inserter_proto.OnBuild(world_data, logic_data, world_coords, layer, orientation);

	return layer;
}

///
/// \brief Registers and creates tile UniqueData for TransportSegment
inline void TestRegisterTransportSegment(jactorio::game::WorldData& world_data,
                                         const jactorio::game::Chunk::ChunkPair& world_coords,
                                         const std::shared_ptr<jactorio::game::TransportSegment>& segment,
                                         const jactorio::data::TransportLine& prototype) {
	auto* tile = world_data.GetTile(world_coords);
	assert(tile);
	auto* chunk = world_data.GetChunk(world_coords);
	assert(chunk);

	auto& layer         = tile->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
	layer.prototypeData = &prototype;

	layer.MakeUniqueData<jactorio::data::TransportLineData>(segment);

	chunk->GetLogicGroup(jactorio::game::Chunk::LogicGroup::transport_line)
	     .emplace_back(&tile->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity));
}

///
/// \brief Creates a 2x2 multi tile assembly machine at coordinates
/// \return top left layer
inline jactorio::game::ChunkTileLayer& TestSetupAssemblyMachine(jactorio::game::WorldData& world_data,
                                                                const jactorio::game::WorldData::WorldPair& world_coords,
                                                                const jactorio::data::AssemblyMachine& assembly_proto) {
	auto& origin_layer = world_data.GetTile(world_coords)->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);
	origin_layer.InitMultiTileProp(2, 2);

	for (int y = 0; y < 2; ++y) {
		for (int x = 0; x < 2; ++x) {
			if (x == 0 && y == 0)
				continue;

			auto& layer = world_data.GetTile(world_coords.first + x, world_coords.second + y)
			                        ->GetLayer(jactorio::game::ChunkTile::ChunkLayer::entity);

			layer.prototypeData  = &assembly_proto;
			layer.multiTileIndex = y * 2 + x;
			layer.SetMultiTileParent(&origin_layer);
		}
	}

	origin_layer.MakeUniqueData<jactorio::data::AssemblyMachineData>();

	return origin_layer;
}


struct TestSetupRecipeReturn
{
	jactorio::data::Recipe recipe{};
	jactorio::data::Item* item1       = nullptr;
	jactorio::data::Item* item2       = nullptr;
	jactorio::data::Item* itemProduct = nullptr;
};

///
/// \brief Sets up and registers a recipe
/// 1a + 1b = 1c
J_NODISCARD inline auto TestSetupRecipe(jactorio::data::PrototypeManager& proto_manager) {
	TestSetupRecipeReturn rt{};

	rt.recipe.craftingTime = 1.f;
	rt.recipe.ingredients  = {{"@1", 1}, {"@2", 1}};
	rt.recipe.product      = {"@3", 1};

	auto i_1 = std::make_unique<jactorio::data::Item>();
	rt.item1 = i_1.get();

	auto i_2 = std::make_unique<jactorio::data::Item>();
	rt.item2 = i_2.get();

	auto item_3    = std::make_unique<jactorio::data::Item>();
	rt.itemProduct = item_3.get();

	proto_manager.DataRawAdd("@1", i_1.release());
	proto_manager.DataRawAdd("@2", i_2.release());
	proto_manager.DataRawAdd("@3", item_3.release());

	return rt;
}

#endif // JACTORIO_TEST_JACTORIOTESTS_H
