// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/input/mouse_selection.h"

#include "jactorio.h"

#include "data/prototype_manager.h"
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


void jactorio::game::MouseSelection::DrawCursorOverlay(PlayerData& player_data, const data::PrototypeManager& data_manager) {
	auto* last_tile = player_data.GetPlayerWorldData().GetTile(lastTilePos_.first,
	                                                           lastTilePos_.second);
	if (last_tile == nullptr)
		return;


	const auto cursor_position = player_data.GetMouseTileCoords();
	const auto* stack          = player_data.GetSelectedItemStack();

	if (stack)
		DrawOverlay(player_data, data_manager,
		            static_cast<data::Entity*>(stack->item->entityPrototype),
		            cursor_position.first, cursor_position.second, player_data.placementOrientation);
	else
		DrawOverlay(player_data, data_manager,
		            nullptr,
		            cursor_position.first, cursor_position.second, player_data.placementOrientation);
}

void jactorio::game::MouseSelection::DrawOverlay(PlayerData& player_data, const data::PrototypeManager& data_manager,
                                                 data::Entity* const selected_entity,
                                                 const int world_x, const int world_y,
                                                 const data::Orientation placement_orientation) {
	WorldData& world_data = player_data.GetPlayerWorldData();

	auto* last_tile = world_data.GetTile(lastTilePos_.first,
	                                     lastTilePos_.second);
	auto* tile = world_data.GetTile(world_x, world_y);


	// TODO reimplement
	// Clear last overlay
	if (!last_tile)
		return;
	// PlaceSpriteAtCoords(
	// 	world_data,
	// 	ChunkTile::ChunkLayer::overlay,
	// 	nullptr,
	// 	lastTileDimensions_.first, lastTileDimensions_.second,
	// 	lastTilePos_.first, lastTilePos_.second);


	// Draw new overlay
	if (!tile)
		return;
	lastTilePos_ = {world_x, world_y};

	if (selected_entity && selected_entity->placeable) {
		// Has item selected
		// PlaceSpriteAtCoords(world_data, ChunkTile::ChunkLayer::overlay, selected_entity->sprite,
		//                     selected_entity->tileWidth, selected_entity->tileHeight, world_x,
		//                     world_y);

		// Rotatable entities
		if (selected_entity->rotatable) {
			// Create unique data at tile to indicate set / frame to render
			const auto set = selected_entity->OnRGetSet(placement_orientation,
			                                            world_data,
			                                            {world_x, world_y});

			// ChunkTileLayer& target_layer = tile->GetLayer(ChunkTile::ChunkLayer::overlay);
			//
			// target_layer.MakeUniqueData<data::PrototypeRenderableData>(set);
			// target_layer.prototypeData = selected_entity
			//                              ->OnRGetSprite(target_layer.GetUniqueData(),
			//                                             player_data.GetPlayerLogicData().GameTick()).first;
		}

		lastTileDimensions_ = {selected_entity->tileWidth, selected_entity->tileHeight};
	}
	else {
		// No item selected
		if (tile->GetLayer(ChunkTile::ChunkLayer::entity).prototypeData ||
			tile->GetLayer(ChunkTile::ChunkLayer::resource).prototypeData) {

			// Is hovering over entity	
			const auto* sprite_ptr =
				data_manager.DataRawGet<data::Sprite>(player_data.MouseSelectedTileInRange()
					                                      ? "__core__/cursor-select"
					                                      : "__core__/cursor-invalid");
			assert(sprite_ptr != nullptr);

			// tile->SetSpritePrototype(ChunkTile::ChunkLayer::overlay, sprite_ptr);
			//
			// auto& layer = tile->GetLayer(ChunkTile::ChunkLayer::overlay);
			// if (layer.IsMultiTile())
			// 	layer.GetMultiTileData().multiTileSpan = 1;
		}

		lastTileDimensions_ = {1, 1};
	}
}
