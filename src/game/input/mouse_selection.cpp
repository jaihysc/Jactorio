// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/input/mouse_selection.h"

#include "jactorio.h"

#include "data/prototype_manager.h"
#include "data/prototype/abstract_proto/entity.h"
#include "game/player/player_data.h"
#include "game/world/chunk_tile.h"
#include "game/world/world_data.h"
#include "renderer/rendering/renderer.h"

using namespace jactorio;

double x_position = 0.;
double y_position = 0.;

void game::SetCursorPosition(const double x_pos, const double y_pos) {
	x_position = x_pos;
	y_position = y_pos;
}


double game::MouseSelection::GetCursorX() {
	return x_position;
}

double game::MouseSelection::GetCursorY() {
	return y_position;
}


void game::MouseSelection::DrawCursorOverlay(PlayerData& player_data, const data::PrototypeManager& data_manager) {
	const auto cursor_position = player_data.GetMouseTileCoords();
	const auto* stack          = player_data.GetSelectedItemStack();

	if (stack)
		DrawOverlay(player_data, data_manager,
		            static_cast<data::Entity*>(stack->item->entityPrototype),
		            {cursor_position.x, cursor_position.y}, player_data.placementOrientation);
	else
		DrawOverlay(player_data, data_manager,
		            nullptr,
		            {cursor_position.x, cursor_position.y}, player_data.placementOrientation);
}

void game::MouseSelection::DrawOverlay(PlayerData& player_data, const data::PrototypeManager& data_manager,
                                       const data::Entity* const selected_entity,
                                       const WorldCoord& coord,
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
	auto* chunk = world_data.GetChunkW(coord.x, coord.y);
	if (!chunk)
		return;

	auto& overlay_layer = chunk->GetOverlay(kCursorOverlayLayer);

	auto* tile = world_data.GetTile(coord.x, coord.y);
	if (!tile)
		return;

	// Saves such that can be found and removed in the future
	auto save_overlay_info = [&]() {
		lastOverlayElementIndex_ = overlay_layer.size() - 1;
		lastChunkPos_            = {WorldData::WorldCToChunkC(coord.x), WorldData::WorldCToChunkC(coord.y)};
	};


	if (selected_entity && selected_entity->placeable) {
		// Has item selected
		const auto set = selected_entity->OnRGetSpriteSet(placement_orientation,
		                                                  world_data,
		                                                  {coord.x, coord.y});

		OverlayElement element{
			*selected_entity->OnRGetSprite(set),
			{WorldData::WorldCToOverlayC(coord.x), WorldData::WorldCToOverlayC(coord.y)},
			{core::SafeCast<float>(selected_entity->tileWidth), core::SafeCast<float>(selected_entity->tileHeight)},
			kCursorOverlayLayer
		};

		element.spriteSet = set;

		overlay_layer.push_back(element);
		save_overlay_info();
	}
	else if (tile->GetLayer(TileLayer::entity).prototypeData.Get() ||
		tile->GetLayer(TileLayer::resource).prototypeData.Get()) {

		// Is hovering over entity	
		const auto* sprite =
			data_manager.DataRawGet<data::Sprite>(player_data.MouseSelectedTileInRange()
				                                      ? "__core__/cursor-select"
				                                      : "__core__/cursor-invalid");
		assert(sprite);

		overlay_layer.push_back({
			*sprite,
			{WorldData::WorldCToOverlayC(coord.x), WorldData::WorldCToOverlayC(coord.y)},
			{1, 1},
			kCursorOverlayLayer
		});
		save_overlay_info();
	}
}

void game::MouseSelection::SkipErasingLastOverlay() noexcept {
	lastOverlayElementIndex_ = UINT64_MAX;
}
