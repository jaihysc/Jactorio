// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/29/2020

#ifndef JACTORIO_TEST_JACTORIOTESTS_H
#define JACTORIO_TEST_JACTORIOTESTS_H
#pragma once

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/inserter.h"
#include "data/prototype/entity/transport/transport_line.h"
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
                                                         const jactorio::game::WorldData::WorldPair& world_coords,
                                                         const jactorio::data::Inserter& inserter_proto,
                                                         const jactorio::data::Orientation orientation) {
	using namespace jactorio;

	auto& layer = world_data.GetTile(world_coords)->GetLayer(game::ChunkTile::ChunkLayer::entity);

	layer.prototypeData = &inserter_proto;
	inserter_proto.OnBuild(world_data, world_coords, layer, orientation);

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

#endif // JACTORIO_TEST_JACTORIOTESTS_H
