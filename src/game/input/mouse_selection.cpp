// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/21/2019

#include "game/input/mouse_selection.h"

#include "jactorio.h"

#include "data/data_manager.h"
#include "data/prototype/entity/entity.h"
#include "game/logic/placement_controller.h"
#include "game/player/player_data.h"
#include "game/world/chunk_tile.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/rendering/renderer.h"

double x_position = 0.f;
double y_position = 0.f;

void jactorio::game::SetCursorPosition(const double x_pos, const double y_pos) {
	x_position = x_pos;
	y_position = y_pos;
}


double jactorio::game::MouseSelection::GetCursorX() {
	return x_position;
}

double jactorio::game::MouseSelection::GetCursorY() {
	return y_position;
}


void jactorio::game::MouseSelection::DrawCursorOverlay(PlayerData& player_data) {
	auto* last_tile = player_data.GetPlayerWorld().GetTile(lastTilePos_.first,
	                                                       lastTilePos_.second);
	if (last_tile == nullptr)
		return;


	const auto cursor_position = player_data.GetMouseTileCoords();
	const data::ItemStack* ptr;
	if ((ptr = player_data.GetSelectedItem()) != nullptr)
		DrawOverlay(player_data, static_cast<data::Entity*>(ptr->first->entityPrototype),
		            cursor_position.first, cursor_position.second, player_data.placementOrientation);
	else
		DrawOverlay(player_data, nullptr,
		            cursor_position.first, cursor_position.second, player_data.placementOrientation);
}

void jactorio::game::MouseSelection::DrawOverlay(PlayerData& player_data, data::Entity* const selected_entity,
                                                 const int world_x, const int world_y,
                                                 const data::Orientation placement_orientation) {
	WorldData& world_data = player_data.GetPlayerWorld();

	auto* last_tile = world_data.GetTile(lastTilePos_.first,
	                                     lastTilePos_.second);
	auto* tile = world_data.GetTile(world_x, world_y);


	// Clear last overlay
	if (!last_tile)
		return;
	PlaceSpriteAtCoords(
		world_data,
		ChunkTile::ChunkLayer::overlay,
		nullptr,
		lastTileDimensions_.first, lastTileDimensions_.second,
		lastTilePos_.first, lastTilePos_.second);


	// Draw new overlay
	if (!tile)
		return;
	lastTilePos_ = {world_x, world_y};

	if (selected_entity && selected_entity->placeable) {
		// Has item selected
		PlaceSpriteAtCoords(world_data, ChunkTile::ChunkLayer::overlay, selected_entity->sprite,
		                    selected_entity->tileWidth, selected_entity->tileHeight, world_x,
		                    world_y);

		// Rotatable entities
		if (selected_entity->rotatable) {
			// Create unique data at tile to indicate set / frame to render
			const auto set = selected_entity->MapPlacementOrientation(placement_orientation,
			                                                             world_data,
			                                                             {world_x, world_y});

			ChunkTileLayer& target_layer = tile->GetLayer(ChunkTile::ChunkLayer::overlay);

			target_layer.MakeUniqueData<data::RenderableData>(set);
			target_layer.prototypeData = selected_entity
			                             ->OnRGetSprite(target_layer.GetUniqueData(),
			                                            player_data.GetPlayerWorld().GameTick()).first;
		}

		lastTileDimensions_ = {selected_entity->tileWidth, selected_entity->tileHeight};
	}
	else {
		// No item selected
		if (tile->GetLayer(ChunkTile::ChunkLayer::entity).prototypeData ||
			tile->GetLayer(ChunkTile::ChunkLayer::resource).prototypeData) {

			// Is hovering over entity	
			const auto* sprite_ptr = data::DataRawGet<data::Sprite>(
				data::DataCategory::sprite,
				player_data.MouseSelectedTileInRange() ? "__core__/cursor-select" : "__core__/cursor-invalid");
			assert(sprite_ptr != nullptr);

			tile->SetSpritePrototype(ChunkTile::ChunkLayer::overlay, sprite_ptr);

			auto& layer = tile->GetLayer(ChunkTile::ChunkLayer::overlay);
			if (layer.IsMultiTile())
				layer.GetMultiTileData().multiTileSpan = 1;
		}

		lastTileDimensions_ = {1, 1};
	}
}
