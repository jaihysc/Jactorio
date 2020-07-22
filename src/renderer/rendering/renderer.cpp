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

	GlResizeWindow(m_viewport[2], m_viewport[3]);
}

// ======================================================================

void renderer::Renderer::GlClear() {
	DEBUG_OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

void renderer::Renderer::GlResizeWindow(const unsigned int window_x,
                                        const unsigned int window_y) {
	// glViewport is critical, changes the size of the rendering area
	glViewport(0, 0, window_x, window_y);

	// Initialize fields
	windowWidth_  = window_x;
	windowHeight_ = window_y;
	GlUpdateTileProjectionMatrix();
}

void renderer::Renderer::GlSetDrawThreads(const size_t threads) {
	assert(threads > 0);
	drawThreads_ = threads;

	chunkDrawThreads_.resize(drawThreads_);

	renderLayers_.clear();  // Opengl probably stores some internal memory addresses, so each layer must be recreated
	renderLayers_.resize(drawThreads_ * 2);
}


void renderer::Renderer::GlDraw(const unsigned int element_count) noexcept {
	DEBUG_OPENGL_CALL(
		// There are 6 indices for each tile
		glDrawElements(GL_TRIANGLES, element_count * 6, GL_UNSIGNED_INT, nullptr)
	); // Pointer not needed as buffer is already bound
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

// ======================================================================


void renderer::Renderer::RenderPlayerPosition(const GameTickT game_tick,
                                              const game::WorldData& world_data,
                                              const float player_x, const float player_y) {
	assert(spritemapCoords_);
	assert(drawThreads_ > 0);

	EXECUTION_PROFILE_SCOPE(profiler, "World draw");

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

	const auto tile_offset = GetTileDrawOffset(position_x, position_y);

	const auto chunk_start  = GetChunkDrawStart(position_x, position_y);
	const auto chunk_amount = GetChunkDrawAmount(position_x, position_y);

	CalculateViewMatrix(player_x, player_y);

	int started_threads = 0;  // Also is index for vector holding futures


	auto begin_prepare_data = [](RendererLayer& r_layer) {
		r_layer.Clear();
		r_layer.GlWriteBegin();
	};

	auto end_prepare_data = [](RendererLayer& r_layer) {
		r_layer.GlWriteEnd();
		r_layer.GlBindBuffers();
		r_layer.GlHandleBufferResize();
		GlDraw(r_layer.GetElementCount());
	};

	auto await_thread_completion = [&]() {
		for (int i = 0; i < started_threads; ++i) {
			chunkDrawThreads_[i].wait();
			auto& r_layer_tile   = renderLayers_[i];
			auto& r_layer_unique = renderLayers_[i + drawThreads_];

			end_prepare_data(r_layer_tile);
			end_prepare_data(r_layer_unique);
		}
	};


	for (int layer_index = 0; layer_index < static_cast<int>(game::ChunkTile::ChunkLayer::count_); ++layer_index) {
		for (int y = 0; y < chunk_amount.y; ++y) {

			// Wait for started threads to finish before starting new ones
			if (static_cast<size_t>(started_threads) == drawThreads_) {
				await_thread_completion();
				started_threads = 0;
			}


			const auto chunk_y = chunk_start.y + y;

			auto& r_layer_tile   = renderLayers_[started_threads];
			auto& r_layer_unique = renderLayers_[started_threads + drawThreads_];

			begin_prepare_data(r_layer_tile);
			begin_prepare_data(r_layer_unique);

			core::Position2<int> row_start = {chunk_start.x, chunk_y};

			chunkDrawThreads_[started_threads] =
				std::async(std::launch::async, &Renderer::PrepareChunkRow, this,
				           std::ref(r_layer_tile), std::ref(r_layer_unique),
				           std::ref(world_data),
				           row_start, chunk_amount.x, layer_index,
				           tile_offset.x, y * game::Chunk::kChunkWidth + tile_offset.y,
				           game_tick
				);

			++started_threads;

		}
	}

	await_thread_completion();
}

void renderer::Renderer::CalculateViewMatrix(const float player_x, const float player_y) noexcept {
	const auto position_x = static_cast<int>(player_x);
	const auto position_y = static_cast<int>(player_y);

	// Negative moves window right and down

	// Decimal is used to shift the camera
	// Invert the movement to give the illusion of moving in the correct direction
	const float camera_offset_x = (player_x - position_x) * static_cast<float>(tileWidth) * -1;
	const float camera_offset_y = (player_y - position_y) * static_cast<float>(tileWidth) * -1;

	// Remaining pixel distance not covered by tiles and chunks are covered by the view matrix to center
	const auto tile_amount = GetTileDrawAmount();

	// Divide by 2 first to truncate decimals
	const auto& view_transform = GetViewTransform();
	view_transform->x
		= static_cast<float>(static_cast<int>(GetWindowWidth() / 2 - (tile_amount.x / 2 * tileWidth)))
		+ camera_offset_x;

	view_transform->y
		= static_cast<float>(static_cast<int>(GetWindowHeight() / 2 - (tile_amount.y / 2 * tileWidth)))
		+ camera_offset_y;

	UpdateViewTransform();
	GlUpdateTileProjectionMatrix();
	UpdateShaderMvp();
}

core::Position2<int> renderer::Renderer::GetTileDrawAmount() noexcept {
	const auto matrix        = glm::vec4(1, -1, 1, 1) / GetMvpMatrix();
	const auto tile_amount_x = static_cast<int>(matrix.x / static_cast<double>(tileWidth) * 2) + 2;
	const auto tile_amount_y = static_cast<int>(matrix.y / static_cast<double>(tileWidth) * 2) + 2;

	return {tile_amount_x, tile_amount_y};
}

core::Position2<int> renderer::Renderer::GetTileDrawOffset(const int position_x, const int position_y) noexcept {
	// Player has not moved an entire chunk's width yet, offset the tiles
	// Modulus chunk_width to make it snap back to 0 after offsetting the entirety of a chunk
	// Inverted to move the tiles AWAY from the screen instead of following the screen
	auto tile_start_x = static_cast<int>(position_x % game::Chunk::kChunkWidth * -1);
	auto tile_start_y = static_cast<int>(position_y % game::Chunk::kChunkWidth * -1);

	const auto tile_amount = GetTileDrawAmount();

	// Center player position on screen 
	tile_start_x += tile_amount.x / 2 % game::Chunk::kChunkWidth;
	tile_start_y += tile_amount.y / 2 % game::Chunk::kChunkWidth;

	// Extra chunk must be hid for the top and left sides to hide black background
	tile_start_x -= (kPaddingChunks + 1) * game::Chunk::kChunkWidth;
	tile_start_y -= (kPaddingChunks + 1) * game::Chunk::kChunkWidth;

	return {tile_start_x, tile_start_y};
}

core::Position2<int> renderer::Renderer::GetChunkDrawStart(const int position_x, const int position_y) noexcept {
	auto chunk_start_x = static_cast<int>(position_x / game::Chunk::kChunkWidth);
	auto chunk_start_y = static_cast<int>(position_y / game::Chunk::kChunkWidth);

	const auto tile_amount = GetTileDrawAmount();

	// Center player position on screen 
	chunk_start_x -= tile_amount.x / 2 / game::Chunk::kChunkWidth;
	chunk_start_y -= tile_amount.y / 2 / game::Chunk::kChunkWidth;

	// Match the tile offset with start offset
	chunk_start_x -= kPaddingChunks + 1;
	chunk_start_y -= kPaddingChunks + 1;

	return {chunk_start_x, chunk_start_y};
}

core::Position2<int> renderer::Renderer::GetChunkDrawAmount(const int position_x, const int position_y) noexcept {
	const auto tile_offset = GetTileDrawOffset(position_x, position_y);
	const auto tile_amount = GetTileDrawAmount();

	int chunk_amount_x = (tile_amount.x - tile_offset.x) / game::Chunk::kChunkWidth;
	int chunk_amount_y = (tile_amount.y - tile_offset.y) / game::Chunk::kChunkWidth;

	chunk_amount_x += kPaddingChunks;
	chunk_amount_y += kPaddingChunks;

	return {chunk_amount_x, chunk_amount_y};
}

// ======================================================================

void renderer::Renderer::PrepareChunkRow(RendererLayer& r_layer_tile, RendererLayer& r_layer_unique,
                                         const game::WorldData& world_data,
                                         const core::Position2<int> row_start, const int chunk_span, const int layer_index,
                                         const int render_tile_offset_x, const int render_pixel_offset_y,
                                         const GameTickT game_tick) const noexcept {

	for (int x = 0; x < chunk_span; ++x) {
		const auto chunk_x = x + row_start.x;

		const auto* chunk = world_data.GetChunkC(chunk_x, row_start.y);

		// Queue chunk for generation if it does not exist
		if (!chunk) {
			world_data.QueueChunkGeneration(chunk_x, row_start.y);
			continue;
		}

		PrepareChunk(r_layer_tile, r_layer_unique,
		             *chunk,
		             {
			             x * game::Chunk::kChunkWidth + render_tile_offset_x,
			             render_pixel_offset_y
		             },
		             layer_index,
		             game_tick);
	}
}

void renderer::Renderer::PrepareChunk(RendererLayer& r_layer_tile, RendererLayer& r_layer_unique,
                                      const game::Chunk& chunk,
                                      const core::Position2<int> render_pixel_offset, const int layer_index,
                                      const GameTickT game_tick) const noexcept {
	// Load chunk into buffer
	game::ChunkTile* tiles = chunk.Tiles();


	// Iterate through and load tiles of a chunk into layer for rendering
	for (uint8_t tile_y = 0; tile_y < game::Chunk::kChunkWidth; ++tile_y) {
		const auto pixel_y = static_cast<float>(render_pixel_offset.y + tile_y) * static_cast<float>(tileWidth);

		for (uint8_t tile_x = 0; tile_x < game::Chunk::kChunkWidth; ++tile_x) {
			const auto pixel_x = static_cast<float>(render_pixel_offset.x + tile_x) * static_cast<float>(tileWidth);

			auto& tile       = tiles[tile_y * game::Chunk::kChunkWidth + tile_x];
			auto& tile_layer = tile.GetLayer(layer_index);


			const auto* proto = tile_layer.GetPrototypeData<data::IPrototypeRenderable>();
			if (!proto)  // Layer not initialized
				continue;

			const auto* unique_data = tile_layer.GetMultiTileTopLeft().GetUniqueData<data::PrototypeRenderableData>();

			// Unique data can be nullptr for certain layers
			const auto sprite_frame = proto->OnRGetSprite(unique_data, game_tick);
			auto uv                 = spritemapCoords_->at(sprite_frame.first->internalId);

			if (unique_data) {
				ApplySpriteUvAdjustment(uv, sprite_frame.first->GetCoords(unique_data->set, sprite_frame.second));
				unique_data->OnDrawUniqueData(r_layer_unique, *spritemapCoords_, pixel_x, pixel_y);
			}

			if (tile_layer.IsMultiTile())
				ApplyMultiTileUvAdjustment(uv, tile_layer);


			r_layer_tile.PushBack(
				RendererLayer::Element(
					{  // top left of tile, 1 tile over and down
						{pixel_x, pixel_y},
						{pixel_x + static_cast<float>(tileWidth), pixel_y + static_cast<float>(tileWidth)}
					},
					{
						uv.topLeft,
						uv.bottomRight
					}
				)
			);

		}
	}
}


void renderer::Renderer::ApplySpriteUvAdjustment(core::QuadPosition& uv, const core::QuadPosition& uv_offset) noexcept {
	const auto difference = uv.bottomRight - uv.topLeft;

	assert(difference.x >= 0);
	assert(difference.y >= 0);

	// Calculate bottom first since it needs the unmodified top_left
	uv.bottomRight = uv.topLeft + difference * uv_offset.bottomRight;
	uv.topLeft += difference * uv_offset.topLeft;
}

void renderer::Renderer::ApplyMultiTileUvAdjustment(core::QuadPosition& uv, const game::ChunkTileLayer& tile_layer) noexcept {
	game::MultiTileData& mt_data = tile_layer.GetMultiTileData();

	// Calculate the correct UV coordinates for multi-tile entities
	// Split the sprite into sections and stretch over multiple tiles if this entity is multi tile

	// Total length of the sprite, to be split among the different tiles
	const auto len_x = (uv.bottomRight.x - uv.topLeft.x) / static_cast<float>(mt_data.multiTileSpan);
	const auto len_y = (uv.bottomRight.y - uv.topLeft.y) / static_cast<float>(mt_data.multiTileHeight);

	const double x_multiplier = tile_layer.GetOffsetX();
	const double y_multiplier = tile_layer.GetOffsetY();

	// Opengl flips vertically, thus the y multiplier is inverted
	// bottom right
	uv.bottomRight.x = uv.bottomRight.x - len_x * static_cast<float>(mt_data.multiTileSpan - x_multiplier - 1);
	uv.bottomRight.y = uv.bottomRight.y - len_y * y_multiplier;

	// top left
	uv.topLeft.x = uv.topLeft.x + len_x * x_multiplier;
	uv.topLeft.y = uv.topLeft.y + len_y * static_cast<float>(mt_data.multiTileHeight - y_multiplier - 1);
}


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
