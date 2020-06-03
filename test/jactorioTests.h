// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/29/2020

#ifndef JACTORIO_TEST_JACTORIOTESTS_H
#define JACTORIO_TEST_JACTORIOTESTS_H
#pragma once

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/inserter.h"
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


#endif // JACTORIO_TEST_JACTORIOTESTS_H
