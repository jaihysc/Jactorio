// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/15/2019

#include "renderer/rendering/world_renderer.h"

#include <future>

#include "jactorio.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/interface/renderable.h"
#include "data/prototype/tile/tile.h"
#include "game/world/world_data.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/rendering/mvp_manager.h"

constexpr auto kChunkWidth = jactorio::game::WorldData::kChunkWidth;

struct TileDrawFuncParams
{
	TileDrawFuncParams(const jactorio::game::ChunkTileLayer& tile_layer,
	                   const jactorio::GameTickT game_tick)
		: tileLayer(tile_layer),
		  gameTick(game_tick) {
	}

	const jactorio::game::ChunkTileLayer& tileLayer;
	jactorio::GameTickT gameTick;
};

using TileDrawFuncReturn = std::pair<jactorio::core::QuadPosition, const jactorio::data::RenderableData*>;
using TileDrawFunc = TileDrawFuncReturn (*)(const TileDrawFuncParams&);

using ObjectDrawFunc = unsigned int (*)(const jactorio::game::ChunkObjectLayer&);

const TileDrawFuncReturn no_draw{
	{{-1.f, -1.f}, {-1.f, -1.f}},
	nullptr
};

void ApplyUvOffset(jactorio::core::QuadPosition& uv, const jactorio::core::QuadPosition& uv_offset) {
	const auto difference = uv.bottomRight - uv.topLeft;

	assert(difference.x >= 0);
	assert(difference.y >= 0);

	// Calculate bottom first since it needs the unmodified top_left
	uv.bottomRight = uv.topLeft + difference * uv_offset.bottomRight;
	uv.topLeft += difference * uv_offset.topLeft;
}


/// \brief Functions for drawing each layer, they are accessed by layer_index
/// \return uv coords, uniqueData to draw, nullptr if no unique data to draw
/// \remark return top_left.x -1 to skip
constexpr TileDrawFunc tile_draw_func[]{
	[](const TileDrawFuncParams& params) {
		// Sprites + tiles are guaranteed not nullptr
		const auto* t     = static_cast<const jactorio::data::Tile*>(params.tileLayer.prototypeData);
		const auto* unique_data = params.tileLayer.GetUniqueData<jactorio::data::RenderableData>();


		const auto sprite_frame = t->OnRGetSprite(unique_data, params.gameTick);
		auto uv                 = jactorio::renderer::Renderer::GetSpritemapCoords(sprite_frame.first->internalId);
		if (unique_data)
			ApplyUvOffset(uv, t->sprite->GetCoords(unique_data->set, sprite_frame.second));

		return TileDrawFuncReturn{uv, nullptr};
	},

	[](const TileDrawFuncParams& params) {
		const auto* t = static_cast<const jactorio::data::Entity*>(params.tileLayer.prototypeData);
		if (t == nullptr)
			return no_draw;

		const auto* unique_data = params.tileLayer.GetUniqueData<jactorio::data::RenderableData>();

		const auto sprite_frame = t->OnRGetSprite(unique_data, params.gameTick);
		auto uv                 = jactorio::renderer::Renderer::GetSpritemapCoords(sprite_frame.first->internalId);

		if (unique_data)  // Unique data may not be initialized by the time this is drawn due to concurrency
			ApplyUvOffset(uv, t->sprite->GetCoords(unique_data->set, sprite_frame.second));

		return TileDrawFuncReturn{uv, nullptr};
	},
	[](const TileDrawFuncParams& params) {
		const auto* t = static_cast<const jactorio::data::Entity*>(params.tileLayer.prototypeData);
		if (t == nullptr)
			return no_draw;

		const auto* unique_data = params.tileLayer.GetUniqueData<jactorio::data::RenderableData>();

		const auto sprite_frame = t->OnRGetSprite(unique_data, params.gameTick);
		auto uv                 = jactorio::renderer::Renderer::GetSpritemapCoords(sprite_frame.first->internalId);

		if (unique_data) {
			ApplyUvOffset(uv, t->sprite->GetCoords(unique_data->set, sprite_frame.second));
			return TileDrawFuncReturn{uv, unique_data};
		}

		return TileDrawFuncReturn{uv, nullptr};
	},

	[](const TileDrawFuncParams& params) {
		const auto* t = static_cast<const jactorio::data::Sprite*>(params.tileLayer.prototypeData);
		if (t == nullptr)
			return no_draw;

		const auto* unique_data = params.tileLayer.GetUniqueData<jactorio::data::RenderableData>();

		auto uv = jactorio::renderer::Renderer::GetSpritemapCoords(t->internalId);

		if (unique_data)  // Unique data may not be initialized by the time this is drawn due to concurrency
			ApplyUvOffset(uv, t->GetCoords(unique_data->set, 0));

		return TileDrawFuncReturn{uv, nullptr};
	}
};

ObjectDrawFunc object_layer_get_sprite_id_func[]{
	// Debug overlay
	[](const jactorio::game::ChunkObjectLayer& layer) {
		const auto* sprite = static_cast<const jactorio::data::Sprite*>(layer.prototypeData);
		return sprite->internalId;
	},
};


// prepare_chunk_draw_data will select either
// prepare_tile_data or prepare_object_data based on the layer it is rendering
void PrepareTileData(const jactorio::game::WorldData& world_data,
                     const unsigned layer_index,
                     jactorio::renderer::RendererLayer& layer,
                     jactorio::renderer::RendererLayer& top_layer,
                     const float chunk_y_offset, const float chunk_x_offset,
                     const jactorio::game::Chunk* const chunk) {
	// Load chunk into buffer
	jactorio::game::ChunkTile* tiles = chunk->Tiles();


	// Iterate through and load tiles of a chunk into layer for rendering
	for (uint8_t tile_y = 0; tile_y < kChunkWidth; ++tile_y) {
		const float y = (chunk_y_offset + tile_y) * static_cast<float>(jactorio::renderer::Renderer::tileWidth);

		for (uint8_t tile_x = 0; tile_x < kChunkWidth; ++tile_x) {
			const jactorio::game::ChunkTile& tile      = tiles[tile_y * kChunkWidth + tile_x];
			jactorio::game::ChunkTileLayer& layer_tile = tile.GetLayer(layer_index);

			TileDrawFuncReturn draw_func_return;
			jactorio::core::QuadPosition uv;

			if (layer_tile.IsMultiTile()) {
				// Unique data for multi tiles is stored in the top left tile
				draw_func_return = tile_draw_func[layer_index]({
					*layer_tile.GetMultiTileTopLeft(),
					world_data.GameTick()
				});
				uv = draw_func_return.first;

				jactorio::game::MultiTileData& mt_data = layer_tile.GetMultiTileData();

				// Calculate the correct UV coordinates for multi-tile entities
				// Split the sprite into sections and stretch over multiple tiles if this entity is multi tile

				// Total length of the sprite, to be split among the different tiles
				const auto len_x = (uv.bottomRight.x - uv.topLeft.x) / static_cast<float>(mt_data.multiTileSpan);
				const auto len_y = (uv.bottomRight.y - uv.topLeft.y) / static_cast<float>(mt_data.multiTileHeight);

				const double x_multiplier = layer_tile.GetOffsetX();
				const double y_multiplier = layer_tile.GetOffsetY();

				// Opengl flips vertically, thus the y multiplier is inverted
				// bottom right
				uv.bottomRight.x = uv.bottomRight.x - len_x * static_cast<float>(mt_data.multiTileSpan - x_multiplier - 1);
				uv.bottomRight.y = uv.bottomRight.y - len_y * y_multiplier;

				// top left
				uv.topLeft.x = uv.topLeft.x + len_x * x_multiplier;
				uv.topLeft.y = uv.topLeft.y + len_y * static_cast<float>(mt_data.multiTileHeight - y_multiplier - 1);
			}
			else {
				draw_func_return = tile_draw_func[layer_index]({
					tile.GetLayer(layer_index),
					world_data.GameTick()
				});
				uv = draw_func_return.first;
			}

			// ======================================================================

			// uv top left.x = -1.f means draw no tile
			if (uv.topLeft.x == -1.f)
				continue;

			// Calculate screen coordinates
			const float x = (chunk_x_offset + tile_x) * static_cast<float>(jactorio::renderer::Renderer::tileWidth);

			layer.PushBack(
				jactorio::renderer::RendererLayer::Element(
					{
						{
							x,
							y
						},
						// One tile right and down
						{
							x + static_cast<float>(jactorio::renderer::Renderer::tileWidth),
							y + static_cast<float>(jactorio::renderer::Renderer::tileWidth)
						}
					},
					{uv.topLeft, uv.bottomRight}
				)
			);

			// Has unique data to draw
			if (draw_func_return.second) {
				draw_func_return.second->OnDrawUniqueData(top_layer, x, y);
			}
		}
	}
}


void PrepareObjectData(const jactorio::game::WorldData& world_data,
                       const unsigned layer_index,
                       jactorio::renderer::RendererLayer& layer,
                       const float chunk_y_offset, const float chunk_x_offset,
                       const jactorio::game::Chunk* const chunk) {

	const auto& objects = chunk->objects[layer_index];
	for (const auto& object_layer : objects) {
		const unsigned int internal_id = object_layer_get_sprite_id_func[layer_index](object_layer);

		// Internal id of 0 indicates no tile
		if (internal_id == 0)
			continue;

		const auto& uv_pos = jactorio::renderer::Renderer::GetSpritemapCoords(internal_id);

		layer.PushBack(jactorio::renderer::RendererLayer::Element(
			{
				{
					(chunk_x_offset + object_layer.positionX)
					* static_cast<float>(jactorio::renderer::Renderer::tileWidth),

					(chunk_y_offset + object_layer.positionY)
					* static_cast<float>(jactorio::renderer::Renderer::tileWidth)
				},
				{
					(chunk_x_offset + object_layer.positionX + object_layer.sizeX)
					* static_cast<float>(jactorio::renderer::Renderer::tileWidth),

					(chunk_y_offset + object_layer.positionY + object_layer.sizeY)
					* static_cast<float>(jactorio::renderer::Renderer::tileWidth)
				}
			},
			{uv_pos.topLeft, uv_pos.bottomRight}
		));
	}
}

struct PrepareProperties
{
	const jactorio::game::WorldData& worldData;


	/// Tiles from window left to offset rendering
	const int renderOffsetX;
	/// Tiles from window top to offset rendering
	const int renderOffsetY;

	/// Chunk to begin rendering
	const int chunkStartX;
	/// Chunk to begin rendering
	const int chunkStartY;

	/// Number of chunks on X axis after chunk_start_x to draw
	const int chunkAmountX;
	/// Layer on which vertex and UV draw data will be placed
	const int chunkAmountY;
};

///
/// \brief Draws chunks to the screen
/// Attempting to draw chunks which do not exist will result in the chunk being queued for generation
/// \param layer_index Index of layer to render
template <bool IsTileLayer>
void PrepareChunkDrawData(const PrepareProperties& props,
                          const int layer_index,
                          jactorio::renderer::RendererLayer& layer,
						  jactorio::renderer::RendererLayer& top_layer) {

	for (int chunk_y = 0; chunk_y < props.chunkAmountY; ++chunk_y) {
		const int chunk_y_offset = chunk_y * kChunkWidth + props.renderOffsetY;

		for (int chunk_x = 0; chunk_x < props.chunkAmountX; ++chunk_x) {
			const int chunk_x_offset = chunk_x * kChunkWidth + props.renderOffsetX;

			std::lock_guard<std::mutex> guard{props.worldData.worldDataMutex};
			const jactorio::game::Chunk* chunk = props.worldData.GetChunkC(props.chunkStartX + chunk_x,
			                                                               props.chunkStartY + chunk_y);
			// Generate chunk if non existent
			if (chunk == nullptr) {
				props.worldData.QueueChunkGeneration(
					props.chunkStartX + chunk_x,
					props.chunkStartY + chunk_y);
				continue;
			}

			if constexpr (IsTileLayer) {
				PrepareTileData(props.worldData,
				                layer_index, 
								layer, top_layer,
				                static_cast<float>(chunk_y_offset), static_cast<float>(chunk_x_offset),
				                chunk);
			}
			else {
				PrepareObjectData(props.worldData,
				                  layer_index, layer,
				                  static_cast<float>(chunk_y_offset), static_cast<float>(chunk_x_offset),
				                  chunk);
			}

		}
	}
}

template <uint8_t Amount,
          typename Function, typename ... Args>
void DrawLayers(jactorio::renderer::RendererLayer& layer_1,
                jactorio::renderer::RendererLayer& layer_2,
                jactorio::renderer::RendererLayer& top_layer,
                Function* function, const Args& ... args) {
#define J_LAYER_BEGIN_PREPARE(name_)\
	(name_).Clear();\
	(name_).GWriteBegin()

#define J_LAYER_END_PREPARE(name_)\
	(name_).GWriteEnd();\
	(name_).GUpdateData();\
	(name_).GBufferBind();\
	jactorio::renderer::Renderer::GDraw((name_).GetElementCount())

	// !Very important! Remember to clear the layers or else it will keep trying to append into it
	J_LAYER_BEGIN_PREPARE(layer_2);
	J_LAYER_BEGIN_PREPARE(top_layer);

	std::future<void> preparing_thread1;
	std::future<void> preparing_thread2 = std::async(std::launch::async,
	                                                 function, args ...,
	                                                 0,
	                                                 std::ref(layer_2), std::ref(top_layer));

	// Begin at index 1, since index 0 is handled above
	for (unsigned int layer_index = 1; layer_index < Amount; ++layer_index) {
		// Prepare 1
		if (layer_index % 2 != 0) {
			J_LAYER_BEGIN_PREPARE(layer_1);
			preparing_thread1 =
				std::async(std::launch::async,
				           function, args ...,
				           layer_index,
				           std::ref(layer_1), std::ref(top_layer));

			preparing_thread2.wait();

			J_LAYER_END_PREPARE(layer_2);
		}
			// Prepare 2
		else {
			J_LAYER_BEGIN_PREPARE(layer_2);
			preparing_thread2 =
				std::async(std::launch::async,
				           function, args ...,
				           layer_index,
				           std::ref(layer_2), std::ref(top_layer));

			preparing_thread1.wait();

			J_LAYER_END_PREPARE(layer_1);
		}
	}

	// Wait for the final layer to draw
	if constexpr (Amount % 2 != 0) {
		preparing_thread2.wait();

		J_LAYER_END_PREPARE(layer_2);
	}
	else {
		preparing_thread1.wait();

		J_LAYER_END_PREPARE(layer_1);
	}

	J_LAYER_END_PREPARE(top_layer);
}

void jactorio::renderer::RenderPlayerPosition(const game::WorldData& world_data,
                                              Renderer* renderer,
                                              const float player_x, const float player_y) {
	// Player movement is in tiles
	// Every chunk_width tiles, shift 1 chunk
	// Remaining tiles are offset

	// The top left of the tile at player position will be at the center of the screen

	// On a 1920 x 1080 screen:
	// 960 pixels from left
	// 540 pixels form top
	// Right and bottom varies depending on tile size

	// Player position with decimal removed
	const auto position_x = static_cast<int>(player_x);
	const auto position_y = static_cast<int>(player_y);


	// How many chunks to offset based on player's position
	auto chunk_start_x = static_cast<int>(position_x / kChunkWidth);
	auto chunk_start_y = static_cast<int>(position_y / kChunkWidth);

	// Player has not moved an entire chunk's width yet, offset the tiles
	// Modulus chunk_width to make it snap back to 0 after offsetting the entirety of a chunk
	// Inverted to move the tiles AWAY from the screen instead of following the screen
	auto tile_start_x = static_cast<int>(position_x % kChunkWidth * -1);
	auto tile_start_y = static_cast<int>(position_y % kChunkWidth * -1);


	const auto tile_amount_x = renderer->GetGridSizeX();
	const auto tile_amount_y = renderer->GetGridSizeY();


	// Render the player position in the center of the screen
	chunk_start_x -= tile_amount_x / 2 / kChunkWidth;
	tile_start_x += tile_amount_x / 2 % kChunkWidth;

	chunk_start_y -= tile_amount_y / 2 / kChunkWidth;
	tile_start_y += tile_amount_y / 2 % kChunkWidth;

	{
		// ##################
		// View matrix

		// Negative moves window right and down

		// Decimal is used to shift the camera
		// Invert the movement to give the illusion of moving in the correct direction
		const float camera_offset_x =
			(player_x - position_x) * static_cast<float>(Renderer::tileWidth) * -1;
		const float camera_offset_y =
			(player_y - position_y) * static_cast<float>(Renderer::tileWidth) * -1;

		// Remaining pixel distance not covered by tiles and chunks are covered by the view matrix
		// to center pixel (For centering specification, see top of function)
		const auto window_width  = Renderer::GetWindowWidth();
		const auto window_height = Renderer::GetWindowHeight();

		// Divide by 2 first to truncate decimals
		const auto& view_transform = GetViewTransform();
		view_transform->x
			= static_cast<float>(static_cast<int>(window_width / 2 - (tile_amount_x / 2 * Renderer::tileWidth)))
			+ camera_offset_x;

		view_transform->y
			= static_cast<float>(static_cast<int>(window_height / 2 - (tile_amount_y / 2 * Renderer::tileWidth)))
			+ camera_offset_y;

		// Set view matrix
		UpdateViewTransform();
		// Set projection matrix
		renderer->UpdateTileProjectionMatrix();
		UpdateShaderMvp();
	}

	EXECUTION_PROFILE_SCOPE(profiler, "World draw");

	// Rendering layers utilizes the following pattern looped
	// Prepare 1 - ASYNC
	// Wait 2
	// Update 2
	// Draw 2
	// -------------------
	// Prepare 2 - ASYNC
	// Wait 1
	// Update 1
	// Draw 1

	// -64 to hide the 2 extra chunk around the outside screen
	const auto window_start_x = tile_start_x - 64;
	const auto window_start_y = tile_start_y - 64;

	// Match the tile offset with start offset
	chunk_start_x -= 2;
	chunk_start_y -= 2;

	// Calculate the maximum number of chunks which can be rendered
	const auto amount_x = (renderer->GetGridSizeX() - window_start_x) / kChunkWidth + 1;  // Render 1 extra chunk on the edge
	const auto amount_y = (renderer->GetGridSizeY() - window_start_y) / kChunkWidth + 1;

	PrepareProperties props{
		world_data,
		window_start_x, window_start_y,
		chunk_start_x, chunk_start_y,
		amount_x, amount_y
	};

	auto& layer_1   = renderer->renderLayers[0];
	auto& layer_2   = renderer->renderLayers[1];
	auto& top_layer = renderer->renderLayers[2];

	DrawLayers<game::ChunkTile::kTileLayerCount>(layer_1, layer_2, top_layer,
	                                             &PrepareChunkDrawData<true>,
	                                             std::ref(props));

	DrawLayers<game::Chunk::kObjectLayerCount>(layer_1, layer_2, top_layer,
	                                           &PrepareChunkDrawData<false>,
	                                           std::ref(props));
}
