// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include "renderer/rendering/renderer.h"

#include <future>
#include <glm/gtc/matrix_transform.hpp>

#include "jactorio.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/interface/renderable.h"
#include "data/prototype/tile/tile.h"
#include "game/world/world_data.h"
#include "renderer/opengl/error.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/rendering/mvp_manager.h"

using namespace jactorio;

unsigned int renderer::Renderer::windowWidth_  = 0;
unsigned int renderer::Renderer::windowHeight_ = 0;

renderer::Renderer::Renderer() {
	// Initialize model matrix
	const glm::mat4 model_matrix = translate(glm::mat4(1.f), glm::vec3(0, 0, 0));
	SetgModelMatrix(model_matrix);
	UpdateShaderMvp();

	// Get window size
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	// TODO must be changeable
	for (auto& render_layer : renderLayers_) {
		render_layer.GInitBuffer();
	}

	GlResizeBuffers(m_viewport[2], m_viewport[3]);
}


// vvvvvvvvvvvvvvvvvvvvvvvvvvvvv

std::unordered_map<unsigned int, core::QuadPosition> renderer::Renderer::spritemapCoords_{};

void renderer::Renderer::SetSpritemapCoords(
	const std::unordered_map<unsigned, core::QuadPosition>& spritemap_coords) {
	spritemapCoords_ = spritemap_coords;
}

core::QuadPosition renderer::Renderer::GetSpritemapCoords(const unsigned internal_id) {
	return spritemapCoords_.at(internal_id);
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// constexpr auto kChunkWidth = game::WorldData::kChunkWidth;
//
// struct TileDrawFuncParams
// {
// 	TileDrawFuncParams(const game::ChunkTileLayer& tile_layer,
// 	                   const GameTickT game_tick)
// 		: tileLayer(tile_layer),
// 		  gameTick(game_tick) {
// 	}
//
// 	const game::ChunkTileLayer& tileLayer;
// 	GameTickT gameTick;
// };
//
// using TileDrawFuncReturn = std::pair<core::QuadPosition, const data::RenderableData*>;
// using TileDrawFunc = TileDrawFuncReturn (*)(const TileDrawFuncParams&);
//
// using ObjectDrawFunc = unsigned int (*)(const game::ChunkObjectLayer&);
//
// const TileDrawFuncReturn no_draw{
// 	{{-1.f, -1.f}, {-1.f, -1.f}},
// 	nullptr
// };
//
// void ApplyUvOffset(core::QuadPosition& uv, const core::QuadPosition& uv_offset) {
// 	const auto difference = uv.bottomRight - uv.topLeft;
//
// 	assert(difference.x >= 0);
// 	assert(difference.y >= 0);
//
// 	// Calculate bottom first since it needs the unmodified top_left
// 	uv.bottomRight = uv.topLeft + difference * uv_offset.bottomRight;
// 	uv.topLeft += difference * uv_offset.topLeft;
// }
//
//
// /// \brief Functions for drawing each layer, they are accessed by layer_index
// /// \return uv coords, uniqueData to draw, nullptr if no unique data to draw
// /// \remark return top_left.x -1 to skip
// constexpr TileDrawFunc tile_draw_func[]{
// 	[](const TileDrawFuncParams& params) {
// 		// Sprites + tiles are guaranteed not nullptr
// 		const auto* t           = static_cast<const data::Tile*>(params.tileLayer.prototypeData);
// 		const auto* unique_data = params.tileLayer.GetUniqueData<data::RenderableData>();
//
//
// 		const auto sprite_frame = t->OnRGetSprite(unique_data, params.gameTick);
// 		auto uv                 = renderer::Renderer::GetSpritemapCoords(sprite_frame.first->internalId);
// 		if (unique_data)
// 			ApplyUvOffset(uv, t->sprite->GetCoords(unique_data->set, sprite_frame.second));
//
// 		return TileDrawFuncReturn{uv, nullptr};
// 	},
//
// 	[](const TileDrawFuncParams& params) {
// 		const auto* t = static_cast<const data::Entity*>(params.tileLayer.prototypeData);
// 		if (t == nullptr)
// 			return no_draw;
//
// 		const auto* unique_data = params.tileLayer.GetUniqueData<data::RenderableData>();
//
// 		const auto sprite_frame = t->OnRGetSprite(unique_data, params.gameTick);
// 		auto uv                 = renderer::Renderer::GetSpritemapCoords(sprite_frame.first->internalId);
//
// 		if (unique_data)  // Unique data may not be initialized by the time this is drawn due to concurrency
// 			ApplyUvOffset(uv, t->sprite->GetCoords(unique_data->set, sprite_frame.second));
//
// 		return TileDrawFuncReturn{uv, nullptr};
// 	},
// 	[](const TileDrawFuncParams& params) {
// 		const auto* t = static_cast<const data::Entity*>(params.tileLayer.prototypeData);
// 		if (t == nullptr)
// 			return no_draw;
//
// 		const auto* unique_data = params.tileLayer.GetUniqueData<data::RenderableData>();
//
// 		const auto sprite_frame = t->OnRGetSprite(unique_data, params.gameTick);
// 		auto uv                 = renderer::Renderer::GetSpritemapCoords(sprite_frame.first->internalId);
//
// 		if (unique_data) {
// 			ApplyUvOffset(uv, t->sprite->GetCoords(unique_data->set, sprite_frame.second));
// 			return TileDrawFuncReturn{uv, unique_data};
// 		}
//
// 		return TileDrawFuncReturn{uv, nullptr};
// 	},
//
// 	[](const TileDrawFuncParams& params) {
// 		const auto* t = static_cast<const data::Sprite*>(params.tileLayer.prototypeData);
// 		if (t == nullptr)
// 			return no_draw;
//
// 		const auto* unique_data = params.tileLayer.GetUniqueData<data::RenderableData>();
//
// 		auto uv = renderer::Renderer::GetSpritemapCoords(t->internalId);
//
// 		if (unique_data)  // Unique data may not be initialized by the time this is drawn due to concurrency
// 			ApplyUvOffset(uv, t->GetCoords(unique_data->set, 0));
//
// 		return TileDrawFuncReturn{uv, nullptr};
// 	}
// };
//
// ObjectDrawFunc object_layer_get_sprite_id_func[]{
// 	// Debug overlay
// 	[](const game::ChunkObjectLayer& layer) {
// 		const auto* sprite = static_cast<const data::Sprite*>(layer.prototypeData);
// 		return sprite->internalId;
// 	},
// };

// template <uint8_t Amount,
//           typename Function, typename ... Args>
// void DrawLayers(renderer::RendererLayer& layer_1,
//                 renderer::RendererLayer& layer_2,
//                 renderer::RendererLayer& top_layer,
//                 Function* function, const Args& ... args) {
// #define J_LAYER_BEGIN_PREPARE(name_)\
// 	(name_).Clear();\
// 	(name_).GWriteBegin()
//
// #define J_LAYER_END_PREPARE(name_)\
// 	(name_).GWriteEnd();\
// 	(name_).GUpdateData();\
// 	(name_).GBufferBind();\
// 	jactorio::renderer::Renderer::GlDraw((name_).GetElementCount())
//
// 	// !Very important! Remember to clear the layers or else it will keep trying to append into it
// 	J_LAYER_BEGIN_PREPARE(layer_2);
// 	J_LAYER_BEGIN_PREPARE(top_layer);
//
// 	std::future<void> preparing_thread1;
// 	std::future<void> preparing_thread2 = std::async(std::launch::async,
// 	                                                 function, args ...,
// 	                                                 0,
// 	                                                 std::ref(layer_2), std::ref(top_layer));
//
// 	// Begin at index 1, since index 0 is handled above
// 	for (unsigned int layer_index = 1; layer_index < Amount; ++layer_index) {
// 		// Prepare 1
// 		if (layer_index % 2 != 0) {
// 			J_LAYER_BEGIN_PREPARE(layer_1);
// 			preparing_thread1 =
// 				std::async(std::launch::async,
// 				           function, args ...,
// 				           layer_index,
// 				           std::ref(layer_1), std::ref(top_layer));
//
// 			preparing_thread2.wait();
//
// 			J_LAYER_END_PREPARE(layer_2);
// 		}
// 			// Prepare 2
// 		else {
// 			J_LAYER_BEGIN_PREPARE(layer_2);
// 			preparing_thread2 =
// 				std::async(std::launch::async,
// 				           function, args ...,
// 				           layer_index,
// 				           std::ref(layer_2), std::ref(top_layer));
//
// 			preparing_thread1.wait();
//
// 			J_LAYER_END_PREPARE(layer_1);
// 		}
// 	}
//
// 	// Wait for the final layer to draw
// 	if constexpr (Amount % 2 != 0) {
// 		preparing_thread2.wait();
//
// 		J_LAYER_END_PREPARE(layer_2);
// 	}
// 	else {
// 		preparing_thread1.wait();
//
// 		J_LAYER_END_PREPARE(layer_1);
// 	}
//
// 	J_LAYER_END_PREPARE(top_layer);
// }

void renderer::Renderer::RenderPlayerPosition(const GameTickT game_tick,
                                              const game::WorldData& world_data,
                                              const float player_x, const float player_y) {
	EXECUTION_PROFILE_SCOPE(profiler, "World draw");

	CalculateViewMatrix(player_x, player_y);

	// Moves the world to match player_position_
	// This is achieved by offsetting the rendered chunks, for decimal numbers, the view matrix is used

	// Player movement is in tiles
	// Every chunk_width tiles, shift 1 chunk
	// Remaining tiles are offset

	// The top left of the tile at player position will be at the center of the screen

	// On a 1920 x 1080 screen:
	// 960 pixels from left
	// 540 pixels form top
	// Right and bottom varies depending on tile size

	// Player position with decimal removed


	// How many chunks to offset based on player's position
	// auto chunk_start_x = static_cast<int>(position_x / game::WorldData::kChunkWidth);
	// auto chunk_start_y = static_cast<int>(position_y / game::WorldData::kChunkWidth);

	// Player has not moved an entire chunk's width yet, offset the tiles
	// Modulus chunk_width to make it snap back to 0 after offsetting the entirety of a chunk
	// Inverted to move the tiles AWAY from the screen instead of following the screen
	// auto tile_start_x = static_cast<int>(position_x % game::WorldData::kChunkWidth * -1);
	// auto tile_start_y = static_cast<int>(position_y % game::WorldData::kChunkWidth * -1);


	// const auto matrix        = glm::vec4(1, -1, 1, 1) / GetMvpMatrix();
	// const auto tile_amount_x = static_cast<int>(matrix.x / static_cast<double>(Renderer::tileWidth) * 2) + 2;
	// const auto tile_amount_y = static_cast<int>(matrix.y / static_cast<double>(Renderer::tileWidth) * 2) + 2;


	// Render the player position in the center of the screen
	// chunk_start_x -= tile_amount_x / 2 / game::WorldData::kChunkWidth;
	// tile_start_x += tile_amount_x / 2 % game::WorldData::kChunkWidth;

	// chunk_start_y -= tile_amount_y / 2 / game::WorldData::kChunkWidth;
	// tile_start_y += tile_amount_y / 2 % game::WorldData::kChunkWidth;


	// // Rendering layers utilizes the following pattern looped
	// // Prepare 1 - ASYNC
	// // Wait 2
	// // Update 2
	// // Draw 2
	// // -------------------
	// // Prepare 2 - ASYNC
	// // Wait 1
	// // Update 1
	// // Draw 1

	// -64 to hide the 2 extra chunk around the outside screen
	// const auto window_start_x = tile_start_x - 64;
	// const auto window_start_y = tile_start_y - 64;

	// Match the tile offset with start offset
	// chunk_start_x -= 2;
	// chunk_start_y -= 2;

	// Calculate the maximum number of chunks which can be rendered
	// const int amount_x = (tile_amount_x - window_start_x) / game::WorldData::kChunkWidth + 1;  // Render 1 extra chunk on the edge
	// const int amount_y = (tile_amount_y - window_start_y) / game::WorldData::kChunkWidth + 1;

	// PrepareProperties props{
	// 	game_tick,
	// 	world_data,
	// 	window_start_x, window_start_y,
	// 	chunk_start_x, chunk_start_y,
	// 	amount_x, amount_y
	// };
	//
	// auto& layer_1   = renderLayers_[0];
	// auto& layer_2   = renderLayers_[1];
	// auto& top_layer = renderLayers_[2];
	//
	// DrawLayers<game::ChunkTile::kTileLayerCount>(layer_1, layer_2, top_layer,
	//                                              &PrepareChunkData,
	//                                              std::ref(props));
	//
	// DrawLayers<game::Chunk::kObjectLayerCount>(layer_1, layer_2, top_layer,
	//                                            &PrepareChunkData,
	//                                            std::ref(props));
}

void renderer::Renderer::CalculateViewMatrix(const float player_x, const float player_y) {
	const auto position_x = static_cast<int>(player_x);
	const auto position_y = static_cast<int>(player_y);

	// Negative moves window right and down

	// Decimal is used to shift the camera
	// Invert the movement to give the illusion of moving in the correct direction
	const float camera_offset_x = (player_x - position_x) * static_cast<float>(tileWidth) * -1;
	const float camera_offset_y = (player_y - position_y) * static_cast<float>(tileWidth) * -1;

	// Remaining pixel distance not covered by tiles and chunks are covered by the view matrix
	// to center pixel (For centering specification, see top of function)
	const auto window_width  = GetWindowWidth();
	const auto window_height = GetWindowHeight();

	const auto tile_amount = GetTileDrawAmount();
	// Divide by 2 first to truncate decimals
	const auto& view_transform = GetViewTransform();
	view_transform->x
		= static_cast<float>(static_cast<int>(window_width / 2 - (tile_amount.x / 2 * tileWidth)))
		+ camera_offset_x;

	view_transform->y
		= static_cast<float>(static_cast<int>(window_height / 2 - (tile_amount.y / 2 * tileWidth)))
		+ camera_offset_y;

	// Set view matrix
	UpdateViewTransform();
	// Set projection matrix
	GlUpdateTileProjectionMatrix();
	UpdateShaderMvp();
}

// ======================================================================

core::Position2<int> renderer::Renderer::GetTileDrawAmount() noexcept {
	const auto matrix = glm::vec4(1, -1, 1, 1) / GetMvpMatrix();

	const auto tile_amount_x = static_cast<int>(matrix.x / static_cast<double>(tileWidth) * 2) + 2;
	const auto tile_amount_y = static_cast<int>(matrix.y / static_cast<double>(tileWidth) * 2) + 2;

	return {tile_amount_x, tile_amount_y};
}

core::Position2<int> renderer::Renderer::GetChunkDrawStart(const int position_x, const int position_y) noexcept {

	// How many chunks to offset based on player's position
	auto chunk_start_x = static_cast<int>(position_x / game::WorldData::kChunkWidth);
	auto chunk_start_y = static_cast<int>(position_y / game::WorldData::kChunkWidth);

	const auto tile_amount = GetTileDrawAmount();

	// Render the player position in the center of the screen
	chunk_start_x -= tile_amount.x / 2 / game::WorldData::kChunkWidth;
	chunk_start_y -= tile_amount.y / 2 / game::WorldData::kChunkWidth;

	// Match the tile offset with start offset
	chunk_start_x -= 2;
	chunk_start_y -= 2;

	return {chunk_start_x, chunk_start_y};
}

core::Position2<int> renderer::Renderer::GetChunkDrawAmount(const int position_x, const int position_y) noexcept {
	// Player has not moved an entire chunk's width yet, offset the tiles
	// Modulus chunk_width to make it snap back to 0 after offsetting the entirety of a chunk
	// Inverted to move the tiles AWAY from the screen instead of following the screen
	auto tile_start_x = static_cast<int>(position_x % game::WorldData::kChunkWidth * -1);
	auto tile_start_y = static_cast<int>(position_y % game::WorldData::kChunkWidth * -1);

	const auto tile_amount = GetTileDrawAmount();

	// Render the player position in the center of the screen
	tile_start_x += tile_amount.x / 2 % game::WorldData::kChunkWidth;
	tile_start_y += tile_amount.y / 2 % game::WorldData::kChunkWidth;

	// -64 to hide the 2 extra chunk around the outside screen
	const auto window_start_x = tile_start_x - 64;
	const auto window_start_y = tile_start_y - 64;

	// Calculate the maximum number of chunks which can be rendered
	const int amount_x = (tile_amount.x - window_start_x) / game::WorldData::kChunkWidth + 1;  // Render 1 extra chunk on the edge
	const int amount_y = (tile_amount.y - window_start_y) / game::WorldData::kChunkWidth + 1;

	return {amount_x, amount_y};
}

void renderer::Renderer::ChunkDrawUnit() {
}

// void renderer::Renderer::PrepareChunkData(const PrepareProperties& props,
//                                           const int layer_index, RendererLayer& layer,
//                                           RendererLayer& top_layer) {
// 	for (int chunk_y = 0; chunk_y < props.chunkAmountY; ++chunk_y) {
// 		const int chunk_y_offset = chunk_y * kChunkWidth + props.renderOffsetY;
//
// 		for (int chunk_x = 0; chunk_x < props.chunkAmountX; ++chunk_x) {
// 			const int chunk_x_offset = chunk_x * kChunkWidth + props.renderOffsetX;
//
// 			const game::Chunk* chunk = props.worldData.GetChunkC(props.chunkStartX + chunk_x,
// 			                                                     props.chunkStartY + chunk_y);
// 			// Generate chunk if non existent
// 			if (chunk == nullptr) {
// 				props.worldData.QueueChunkGeneration(
// 					props.chunkStartX + chunk_x,
// 					props.chunkStartY + chunk_y);
// 				continue;
// 			}
//
// 			if (props.isTileLayer) {
// 				PrepareTileData(props.gameTick,
// 				                layer_index,
// 				                layer, top_layer,
// 				                static_cast<float>(chunk_y_offset), static_cast<float>(chunk_x_offset),
// 				                chunk);
// 			}
// 			else {
// 				PrepareObjectData(layer_index, layer,
// 				                  static_cast<float>(chunk_y_offset), static_cast<float>(chunk_x_offset),
// 				                  chunk);
// 			}
//
// 		}
// 	}
// }
//
// void renderer::Renderer::PrepareTileData(const GameTickT game_tick,
//                                          const unsigned layer_index, RendererLayer& layer,
//                                          RendererLayer& top_layer,
//                                          const float chunk_y_offset, const float chunk_x_offset,
//                                          const game::Chunk* const chunk) {
// 	// Load chunk into buffer
// 	game::ChunkTile* tiles = chunk->Tiles();
//
//
// 	// Iterate through and load tiles of a chunk into layer for rendering
// 	for (uint8_t tile_y = 0; tile_y < kChunkWidth; ++tile_y) {
// 		const float y = (chunk_y_offset + tile_y) * static_cast<float>(tileWidth);
//
// 		for (uint8_t tile_x = 0; tile_x < kChunkWidth; ++tile_x) {
// 			const game::ChunkTile& tile      = tiles[tile_y * kChunkWidth + tile_x];
// 			game::ChunkTileLayer& layer_tile = tile.GetLayer(layer_index);
//
// 			TileDrawFuncReturn draw_func_return;
// 			core::QuadPosition uv;
//
// 			if (layer_tile.IsMultiTile()) {
// 				// Unique data for multi tiles is stored in the top left tile
// 				draw_func_return = tile_draw_func[layer_index](
// 					{
// 						layer_tile.GetMultiTileTopLeft(), game_tick
// 					});
// 				uv = draw_func_return.first;
//
// 				game::MultiTileData& mt_data = layer_tile.GetMultiTileData();
//
// 				// Calculate the correct UV coordinates for multi-tile entities
// 				// Split the sprite into sections and stretch over multiple tiles if this entity is multi tile
//
// 				// Total length of the sprite, to be split among the different tiles
// 				const auto len_x = (uv.bottomRight.x - uv.topLeft.x) / static_cast<float>(mt_data.multiTileSpan);
// 				const auto len_y = (uv.bottomRight.y - uv.topLeft.y) / static_cast<float>(mt_data.multiTileHeight);
//
// 				const double x_multiplier = layer_tile.GetOffsetX();
// 				const double y_multiplier = layer_tile.GetOffsetY();
//
// 				// Opengl flips vertically, thus the y multiplier is inverted
// 				// bottom right
// 				uv.bottomRight.x = uv.bottomRight.x - len_x * static_cast<float>(mt_data.multiTileSpan - x_multiplier - 1);
// 				uv.bottomRight.y = uv.bottomRight.y - len_y * y_multiplier;
//
// 				// top left
// 				uv.topLeft.x = uv.topLeft.x + len_x * x_multiplier;
// 				uv.topLeft.y = uv.topLeft.y + len_y * static_cast<float>(mt_data.multiTileHeight - y_multiplier - 1);
// 			}
// 			else {
// 				draw_func_return = tile_draw_func[layer_index](
// 					{
// 						tile.GetLayer(layer_index),
// 						game_tick
// 					});
// 				uv = draw_func_return.first;
// 			}
//
// 			// ======================================================================
//
// 			// uv top left.x = -1.f means draw no tile
// 			if (uv.topLeft.x == -1.f)
// 				continue;
//
// 			// Calculate screen coordinates
// 			const float x = (chunk_x_offset + tile_x) * static_cast<float>(renderer::Renderer::tileWidth);
//
// 			layer.PushBack(
// 				renderer::RendererLayer::Element(
// 					{
// 						{
// 							x,
// 							y
// 						},
// 						// One tile right and down
// 						{
// 							x + static_cast<float>(renderer::Renderer::tileWidth),
// 							y + static_cast<float>(renderer::Renderer::tileWidth)
// 						}
// 					},
// 					{uv.topLeft, uv.bottomRight}
// 				)
// 			);
//
// 			// Has unique data to draw
// 			if (draw_func_return.second) {
// 				draw_func_return.second->OnDrawUniqueData(top_layer, x, y);
// 			}
// 		}
// 	}
// }
//
// void renderer::Renderer::PrepareObjectData(const unsigned layer_index, RendererLayer& layer,
//                                            const float chunk_y_offset, const float chunk_x_offset,
//                                            const game::Chunk* const chunk) {
//
// 	const auto& objects = chunk->objects[layer_index];
// 	for (const auto& object_layer : objects) {
// 		const unsigned int internal_id = object_layer_get_sprite_id_func[layer_index](object_layer);
//
// 		// Internal id of 0 indicates no tile
// 		if (internal_id == 0)
// 			continue;
//
// 		const auto& uv_pos = GetSpritemapCoords(internal_id);
//
// 		layer.PushBack(RendererLayer::Element(
// 			{
// 				{
// 					(chunk_x_offset + object_layer.positionX)
// 					* static_cast<float>(tileWidth),
//
// 					(chunk_y_offset + object_layer.positionY)
// 					* static_cast<float>(tileWidth)
// 				},
// 				{
// 					(chunk_x_offset + object_layer.positionX + object_layer.sizeX)
// 					* static_cast<float>(tileWidth),
//
// 					(chunk_y_offset + object_layer.positionY + object_layer.sizeY)
// 					* static_cast<float>(tileWidth)
// 				}
// 			},
// 			{uv_pos.topLeft, uv_pos.bottomRight}
// 		));
// 	}
// }


// ======================================================================
// openGL methods

void renderer::Renderer::GlDraw(const unsigned int element_count) {
	DEBUG_OPENGL_CALL(
		// There are 6 indices for each tile
		glDrawElements(GL_TRIANGLES, element_count * 6, GL_UNSIGNED_INT, nullptr)
	); // Pointer not needed as buffer is already bound
}

void renderer::Renderer::GlClear() {
	DEBUG_OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}


void renderer::Renderer::GlResizeBuffers(const unsigned int window_x,
                                         const unsigned int window_y) {
	// Initialize fields
	windowWidth_  = window_x;
	windowHeight_ = window_y;
	GlUpdateTileProjectionMatrix();

	// Raise the bottom and right by tile_width so the last tile has enough space to render out
	tileCountX_ = windowWidth_ / tileWidth + 1;
	tileCountY_ = windowHeight_ / tileWidth + 1;

	gridElementsCount_ = tileCountX_ * tileCountY_;

	// Render layer (More may be reserved as needed by the renderer)
	for (auto& render_layer : renderLayers_) {
		render_layer.Reserve(gridElementsCount_);
		render_layer.GUpdateData();
	}
}

void renderer::Renderer::GlUpdateTileProjectionMatrix() {
	const auto max_tile_width = static_cast<float>(tileWidth * 2);

	if (tileProjectionMatrixOffset < max_tile_width)
		// Prevent zooming out too far
		tileProjectionMatrixOffset = max_tile_width;
	else {
		// Prevent zooming too far in
		unsigned int smallest_axis;
		if (windowWidth_ > windowHeight_) {
			smallest_axis = windowHeight_;
		}
		else {
			smallest_axis = windowWidth_;
		}

		// Maximum zoom is 30 from center
		const int max_zoom_offset = 30;
		if (tileProjectionMatrixOffset > static_cast<float>(smallest_axis) / 2 - max_zoom_offset) {
			tileProjectionMatrixOffset = static_cast<float>(smallest_axis) / 2 - max_zoom_offset;
		}
	}

	SetgProjectionMatrix(ToProjMatrix(windowWidth_, windowHeight_, tileProjectionMatrixOffset));
}
