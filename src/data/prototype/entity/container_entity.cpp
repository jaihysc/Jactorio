// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include "data/prototype/entity/container_entity.h"

#include "renderer/gui/gui_menus.h"

jactorio::data::UniqueDataBase* jactorio::data::ContainerEntity::CopyUniqueData(UniqueDataBase* ptr) const {
	auto* data = new ContainerEntityData(inventorySize);

	// Copy inventory contents over
	for (int i = 0; i < inventorySize; ++i) {
		data->inventory[i] = static_cast<ContainerEntityData*>(ptr)->inventory[i];
	}

	return data;
}


void jactorio::data::ContainerEntity::OnBuild(game::WorldData&,
                                              const game::WorldData::WorldPair&,
                                              game::ChunkTileLayer& tile_layer,
                                              Orientation) const {
	tile_layer.uniqueData = new ContainerEntityData(inventorySize);
}

void jactorio::data::ContainerEntity::OnRShowGui(game::PlayerData& player_data,
                                                 game::ChunkTileLayer* tile_layer) const {
	renderer::ContainerEntity(player_data, static_cast<ContainerEntityData*>(tile_layer->uniqueData));
}
