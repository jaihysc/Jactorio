// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/input/mouse_selection.h"

#include "jactorio.h"

#include "data/prototype_manager.h"
#include "data/prototype/entity/entity.h"
#include "game/player/player_data.h"
#include "game/world/chunk_tile.h"
#include "game/world/world_data.h"
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
	const auto cursor_position = player_data.GetMouseTileCoords();
	const auto* stack          = player_data.GetSelectedItemStack();

	if (stack)
		DrawOverlay(player_data, data_manager,
		            static_cast<data::Entity*>(stack->item->entityPrototype),
		            cursor_position.x, cursor_position.y, player_data.placementOrientation);
	else
		DrawOverlay(player_data, data_manager,
		            nullptr,
		            cursor_position.x, cursor_position.y, player_data.placementOrientation);
}

void jactorio::game::MouseSelection::DrawOverlay(PlayerData& player_data, const data::PrototypeManager& data_manager,
                                                 const data::Entity* const selected_entity,
                                                 const int world_x, const int world_y,
                                                 const data::Orientation placement_orientation) {
	WorldData& world_data = player_data.GetPlayerWorldData();

	// Clear last overlay
	if (lastOverlayElementIndex_ != UINT64_MAX) {
		auto* last_chunk = world_data.GetChunkC(lastChunkPos_);
		assert(last_chunk);

		auto& overlay_layer = last_chunk->GetOverlay(kCursorOverlayLayer);
		overlay_layer.erase(overlay_layer.begin() + lastOverlayElementIndex_);

		lastOverlayElementIndex_ = UINT64_MAX;
	}

	// Draw new overlay
	auto* chunk = world_data.GetChunkW(world_x, world_y);
	if (!chunk)
		return;

	auto& overlay_layer = chunk->GetOverlay(kCursorOverlayLayer);

	auto* tile = world_data.GetTile(world_x, world_y);
	if (!tile)
		return;

	// Saves such that can be found and removed in the future
	auto save_overlay_info = [&]() {
		lastOverlayElementIndex_ = overlay_layer.size() - 1;
		lastChunkPos_            = {WorldData::ToChunkCoord(world_x), WorldData::ToChunkCoord(world_y)};  // TODO
	};


	if (selected_entity && selected_entity->placeable) {
		// Has item selected
		// TODO separate virtual function for get sprite only
		OverlayElement element{
			*selected_entity->sprite, // OnRGetSprite(nullptr, player_data.GetPlayerLogicData().GameTick()).first, 

			{WorldData::ToOverlayCoord(world_x), WorldData::ToOverlayCoord(world_y)},
			{static_cast<float>(selected_entity->tileWidth), static_cast<float>(selected_entity->tileHeight)},
			kCursorOverlayLayer
		};

		// Rotatable entities
		if (selected_entity->rotatable) {
			//  indicate set to render
			element.spriteSet = selected_entity->OnRGetSet(placement_orientation, world_data, {world_x, world_y});
		}

		overlay_layer.push_back(element);
		save_overlay_info();
	}
	else if (tile->GetLayer(ChunkTile::ChunkLayer::entity).prototypeData ||
		tile->GetLayer(ChunkTile::ChunkLayer::resource).prototypeData) {

		// Is hovering over entity	
		const auto* sprite =
			data_manager.DataRawGet<data::Sprite>(player_data.MouseSelectedTileInRange()
				                                      ? "__core__/cursor-select"
				                                      : "__core__/cursor-invalid");
		assert(sprite);

		overlay_layer.push_back({
			*sprite,
			{WorldData::ToOverlayCoord(world_x), WorldData::ToOverlayCoord(world_y)},
			{1, 1},
			kCursorOverlayLayer
		});
		save_overlay_info();
	}
}
