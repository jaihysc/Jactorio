// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include "renderer/rendering/renderer.h"

#include <future>
#include <glm/gtc/matrix_transform.hpp>

#include "jactorio.h"
#include "data/prototype/abstract_proto/entity.h"
#include "data/prototype/interface/renderable.h"
#include "data/prototype/tile.h"
#include "game/world/world_data.h"
#include "renderer/opengl/error.h"
#include "renderer/opengl/mvp_manager.h"

using namespace jactorio;

unsigned int renderer::Renderer::windowWidth_  = 0;
unsigned int renderer::Renderer::windowHeight_ = 0;

renderer::Renderer::Renderer() {
	// Initialize model matrix
	const glm::mat4 model_matrix = translate(glm::mat4(1.f), glm::vec3(0, 0, 0));
	mvpManager_.GlSetModelMatrix(model_matrix);

	// Get window size
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	GlResizeWindow(m_viewport[2], m_viewport[3]);
}

// ======================================================================

void renderer::Renderer::GlSetup() noexcept {
	// Enables transparency in textures
	DEBUG_OPENGL_CALL(glEnable(GL_BLEND));
	DEBUG_OPENGL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	// Depth buffer
	DEBUG_OPENGL_CALL(glEnable(GL_DEPTH_TEST));
	DEBUG_OPENGL_CALL(glDepthFunc(GL_LEQUAL));

	// Fixes depth buffer transparency issues
	// Do not write to depth buffer if fully transparent
	DEBUG_OPENGL_CALL(glEnable(GL_ALPHA_TEST));
	DEBUG_OPENGL_CALL(glAlphaFunc(GL_GREATER, 0));
}

void renderer::Renderer::GlClear() noexcept {
	DEBUG_OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT));
}

void renderer::Renderer::GlResizeWindow(const unsigned int window_x,
                                        const unsigned int window_y) noexcept {
	// glViewport is critical, changes the size of the rendering area
	DEBUG_OPENGL_CALL(glViewport(0, 0, window_x, window_y));

	// Initialize fields
	windowWidth_  = window_x;
	windowHeight_ = window_y;
	GlUpdateTileProjectionMatrix();

	for (auto& render_layer : renderLayers_) {
		render_layer.ResizeDefault();
	}
}

void renderer::Renderer::GlSetDrawThreads(const size_t threads) {
	assert(threads > 0);
	drawThreads_ = threads;

	chunkDrawThreads_.resize(drawThreads_);

	renderLayers_.clear();  // Opengl probably stores some internal memory addresses, so each layer must be recreated
	renderLayers_.resize(drawThreads_);

	assert(chunkDrawThreads_.size() == drawThreads_);
	assert(renderLayers_.size() == drawThreads_);
}


// ======================================================================


const SpriteUvCoordsT::mapped_type& renderer::Renderer::GetSpriteUvCoords(const SpriteUvCoordsT& map,
                                                                          const SpriteUvCoordsT::key_type key) noexcept {
	try {
		return const_cast<SpriteUvCoordsT&>(map)[key];
	}
	catch (std::exception&) {
		assert(false);  // Should not throw
		std::terminate();
	}
}

void renderer::Renderer::GlRenderPlayerPosition(const GameTickT game_tick,
                                                const game::WorldData& world_data,
                                                const float player_x, const float player_y) {
	assert(spritemapCoords_);
	assert(drawThreads_ > 0);
	assert(renderLayers_.size() == drawThreads_);

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
	const auto position_x = core::LossyCast<int>(player_x);
	const auto position_y = core::LossyCast<int>(player_y);


	const auto tile_offset = GetTileDrawOffset(position_x, position_y);

	const auto chunk_start  = GetChunkDrawStart(position_x, position_y);
	const auto chunk_amount = GetChunkDrawAmount(position_x, position_y);


	// Must be calculated after tile_offset, chunk_start and chunk_amount. Otherwise, zooming becomes jagged
	CalculateViewMatrix(player_x, player_y);
	GlUpdateTileProjectionMatrix();
	mvpManager_.CalculateMvpMatrix();
	mvpManager_.UpdateShaderMvp();

	int started_threads = 0;  // Also is index for vector holding futures


	auto begin_prepare_data = [](RendererLayer& r_layer) {
		r_layer.Clear();
		r_layer.GlWriteBegin();
	};

	auto end_prepare_data = [](RendererLayer& r_layer) {
		r_layer.GlWriteEnd();
		r_layer.GlBindBuffers();
		r_layer.GlHandleBufferResize();
		GlDraw(r_layer.GetIndicesCount());
	};

	auto await_thread_completion = [&]() {
		for (int i = 0; i < started_threads; ++i) {
			chunkDrawThreads_[i].wait();
			auto& r_layer_tile = renderLayers_[i];

			end_prepare_data(r_layer_tile);
		}
	};


	for (int y = 0; y < chunk_amount.y; ++y) {

		// Wait for started threads to finish before starting new ones
		if (core::SafeCast<size_t>(started_threads) == drawThreads_) {
			await_thread_completion();
			started_threads = 0;
		}


		const auto chunk_y = chunk_start.y + y;

		auto& r_layer_tile = renderLayers_[started_threads];
		begin_prepare_data(r_layer_tile);

		core::Position2<int> row_start{chunk_start.x, chunk_y};
		core::Position2<int> render_tile_offset{tile_offset.x, y * game::Chunk::kChunkWidth + tile_offset.y};

		chunkDrawThreads_[started_threads] =
			std::async(std::launch::async, &Renderer::PrepareChunkRow, this,
			           std::ref(r_layer_tile), std::ref(world_data),
			           row_start, chunk_amount.x,
			           render_tile_offset,
			           game_tick
			);

		++started_threads;

	}

	await_thread_completion();
}

void renderer::Renderer::CalculateViewMatrix(const float player_x, const float player_y) noexcept {
	const auto position_x = core::LossyCast<int>(player_x);
	const auto position_y = core::LossyCast<int>(player_y);

	// Negative moves window right and down

	// Decimal is used to shift the camera
	// Invert the movement to give the illusion of moving in the correct direction
	const float camera_offset_x = (player_x - position_x) * core::SafeCast<float>(tileWidth) * -1;
	const float camera_offset_y = (player_y - position_y) * core::SafeCast<float>(tileWidth) * -1;

	// Remaining pixel distance not covered by tiles and chunks are covered by the view matrix to center
	const auto tile_amount = GetTileDrawAmount();

	const auto& view_transform = mvpManager_.GetViewTransform();

	// Divide by 2 first to truncate decimals
	// A LossyCast is used as during startup, tile_amount is invalid since the mvp matrix is invalid
	view_transform->x
		= core::LossyCast<float>(GetWindowWidth() / 2 - (tile_amount.x / 2 * tileWidth))
		+ camera_offset_x;

	view_transform->y
		= core::LossyCast<float>(GetWindowHeight() / 2 - (tile_amount.y / 2 * tileWidth))
		+ camera_offset_y;

	mvpManager_.UpdateViewTransform();
}

core::Position2<int> renderer::Renderer::GetTileDrawAmount() const noexcept {
	const auto matrix        = glm::vec4(1, -1, 1, 1) / mvpManager_.GetMvpMatrix();
	const auto tile_amount_x = core::LossyCast<int>(matrix.x / core::LossyCast<double>(tileWidth) * 2) + 2;
	const auto tile_amount_y = core::LossyCast<int>(matrix.y / core::LossyCast<double>(tileWidth) * 2) + 2;

	return {tile_amount_x, tile_amount_y};
}

core::Position2<int> renderer::Renderer::GetTileDrawOffset(const int position_x, const int position_y) const noexcept {
	// Player has not moved an entire chunk's width yet, offset the tiles
	// Modulus chunk_width to make it snap back to 0 after offsetting the entirety of a chunk
	// Inverted to move the tiles AWAY from the screen instead of following the screen
	auto tile_start_x = position_x % game::Chunk::kChunkWidth * -1;
	auto tile_start_y = position_y % game::Chunk::kChunkWidth * -1;

	const auto tile_amount = GetTileDrawAmount();

	// Center player position on screen 
	tile_start_x += tile_amount.x / 2 % game::Chunk::kChunkWidth;
	tile_start_y += tile_amount.y / 2 % game::Chunk::kChunkWidth;

	// Extra chunk must be hid for the top and left sides to hide black background
	tile_start_x -= (kPaddingChunks + 1) * game::Chunk::kChunkWidth;
	tile_start_y -= (kPaddingChunks + 1) * game::Chunk::kChunkWidth;

	return {tile_start_x, tile_start_y};
}

core::Position2<int> renderer::Renderer::GetChunkDrawStart(const int position_x, const int position_y) const noexcept {
	auto chunk_start_x = position_x / game::Chunk::kChunkWidth;
	auto chunk_start_y = position_y / game::Chunk::kChunkWidth;

	const auto tile_amount = GetTileDrawAmount();

	// Center player position on screen 
	chunk_start_x -= tile_amount.x / 2 / game::Chunk::kChunkWidth;
	chunk_start_y -= tile_amount.y / 2 / game::Chunk::kChunkWidth;

	// Match the tile offset with start offset
	chunk_start_x -= kPaddingChunks + 1;
	chunk_start_y -= kPaddingChunks + 1;

	return {chunk_start_x, chunk_start_y};
}

core::Position2<int> renderer::Renderer::GetChunkDrawAmount(const int position_x, const int position_y) const noexcept {
	const auto tile_offset = GetTileDrawOffset(position_x, position_y);
	const auto tile_amount = GetTileDrawAmount();

	int chunk_amount_x = (tile_amount.x - tile_offset.x) / game::Chunk::kChunkWidth;
	int chunk_amount_y = (tile_amount.y - tile_offset.y) / game::Chunk::kChunkWidth;

	chunk_amount_x += kPaddingChunks;
	chunk_amount_y += kPaddingChunks;

	return {chunk_amount_x, chunk_amount_y};
}

void renderer::Renderer::PrepareChunkRow(RendererLayer& r_layer, const game::WorldData& world_data,
                                         const core::Position2<int> row_start, const int chunk_span,
                                         const core::Position2<int> render_tile_offset,
                                         const GameTickT game_tick) const noexcept {

	for (int x = 0; x < chunk_span; ++x) {
		const auto chunk_x = x + row_start.x;

		const auto* chunk = world_data.GetChunkC(chunk_x, row_start.y);

		// Queue chunk for generation if it does not exist
		if (!chunk) {
			world_data.QueueChunkGeneration(chunk_x, row_start.y);
			continue;
		}

		PrepareChunk(r_layer, *chunk,
		             {
			             x * game::Chunk::kChunkWidth + render_tile_offset.x,
			             render_tile_offset.y
		             },
		             game_tick);
	}
}

void renderer::Renderer::PrepareChunk(RendererLayer& r_layer, const game::Chunk& chunk,
                                      const core::Position2<int> render_tile_offset,
                                      const GameTickT game_tick) const noexcept {
	// Load chunk into buffer
	const auto& tiles = chunk.Tiles();


	// Iterate through and load tiles of a chunk into layer for rendering
	for (uint8_t tile_y = 0; tile_y < game::Chunk::kChunkWidth; ++tile_y) {
		const auto pixel_y = core::SafeCast<float>(render_tile_offset.y + tile_y) * core::SafeCast<float>(tileWidth);

		for (uint8_t tile_x = 0; tile_x < game::Chunk::kChunkWidth; ++tile_x) {
			const auto pixel_x = core::SafeCast<float>(render_tile_offset.x + tile_x) * core::SafeCast<float>(tileWidth);

			PrepareTileLayers(r_layer, tiles[tile_y * game::Chunk::kChunkWidth + tile_x],
			                  {pixel_x, pixel_y}, game_tick
			);
		}
	}

	PrepareOverlayLayers(r_layer, chunk, render_tile_offset);
}

void renderer::Renderer::PrepareTileLayers(RendererLayer& r_layer, const game::ChunkTile& tile,
                                           const core::Position2<float>& pixel_pos,
                                           const GameTickT game_tick) const noexcept {
	for (int layer_index = 0; layer_index < game::ChunkTile::kTileLayerCount; ++layer_index) {
		const auto& tile_layer = tile.GetLayer(layer_index);


		const auto* proto = tile_layer.GetPrototypeData<data::FRenderable>();
		if (!proto)  // Layer not initialized
			continue;

		const auto* unique_data = tile_layer.GetMultiTileTopLeft().GetUniqueData<data::FRenderableData>();

		// Unique data can be nullptr for certain layers

		SpriteUvCoordsT::mapped_type uv;

		if (unique_data) {
			const auto* sprite = proto->OnRGetSprite(unique_data->set);
			uv                 = GetSpriteUvCoords(sprite->internalId);

			// Handles rendering portions of sprite
			const auto sprite_frame = proto->OnRGetSpriteFrame(*unique_data, game_tick);
			ApplySpriteUvAdjustment(uv, sprite->GetCoords(unique_data->set, sprite_frame));

			// Custom draw function
			proto->OnRDrawUniqueData(r_layer, *spritemapCoords_, {pixel_pos.x, pixel_pos.y}, unique_data);
		}
		else {
			const auto* sprite = proto->OnRGetSprite(0);
			uv                 = GetSpriteUvCoords(sprite->internalId);
		}

		if (tile_layer.IsMultiTile())
			ApplyMultiTileUvAdjustment(uv, tile_layer);

		const float pixel_z = 0.f + core::LossyCast<float>(0.01 * layer_index);

		r_layer.PushBack(
			{
				{  // top left of tile, 1 tile over and down
					{pixel_pos.x, pixel_pos.y},
					{pixel_pos.x + core::SafeCast<float>(tileWidth), pixel_pos.y + core::SafeCast<float>(tileWidth)}
				},
				{
					uv.topLeft,
					uv.bottomRight
				}
			}, pixel_z
		);

	}
}

void renderer::Renderer::PrepareOverlayLayers(RendererLayer& r_layer, const game::Chunk& chunk,
                                              const core::Position2<int> render_tile_offset) const {

	for (int layer_index = 0; layer_index < game::kOverlayLayerCount; ++layer_index) {
		const auto& overlay_container = chunk.overlays[layer_index];

		for (const auto& overlay : overlay_container) {
			auto uv = GetSpriteUvCoords(overlay.sprite->internalId);

			ApplySpriteUvAdjustment(uv, overlay.sprite->GetCoords(overlay.spriteSet, 0));

			r_layer.PushBack(
				{
					{

						{
							(render_tile_offset.x + overlay.position.x)
							* core::SafeCast<float>(tileWidth),

							(render_tile_offset.y + overlay.position.y)
							* core::SafeCast<float>(tileWidth)
						},
						{
							(render_tile_offset.x + overlay.position.x + overlay.size.x)
							* core::SafeCast<float>(tileWidth),

							(render_tile_offset.y + overlay.position.y + overlay.size.y)
							* core::SafeCast<float>(tileWidth)
						}
					},
					{
						uv.topLeft,
						uv.bottomRight
					}
				}, overlay.position.z);
		}

	}

}

void renderer::Renderer::ApplySpriteUvAdjustment(UvPositionT& uv,
                                                 const UvPositionT& uv_offset) noexcept {
	const auto diff_x = uv.bottomRight.x - uv.topLeft.x;
	const auto diff_y = uv.bottomRight.y - uv.topLeft.y;

	assert(diff_x >= 0);
	assert(diff_y >= 0);

	// Calculate bottom first since it needs the unmodified top_left
	uv.bottomRight.x = uv.topLeft.x + diff_x * uv_offset.bottomRight.x;
	uv.bottomRight.y = uv.topLeft.y + diff_y * uv_offset.bottomRight.y;

	uv.topLeft.x += diff_x * uv_offset.topLeft.x;
	uv.topLeft.y += diff_y * uv_offset.topLeft.y;
}

void renderer::Renderer::ApplyMultiTileUvAdjustment(UvPositionT& uv,
                                                    const game::ChunkTileLayer& tile_layer) noexcept {
	game::MultiTileData& mt_data = tile_layer.GetMultiTileData();

	// Calculate the correct UV coordinates for multi-tile entities
	// Split the sprite into sections and stretch over multiple tiles if this entity is multi tile

	// Total length of the sprite, to be split among the different tiles
	const auto len_x = (uv.bottomRight.x - uv.topLeft.x) / core::SafeCast<float>(mt_data.multiTileSpan);
	const auto len_y = (uv.bottomRight.y - uv.topLeft.y) / core::SafeCast<float>(mt_data.multiTileHeight);

	const double x_multiplier = tile_layer.GetOffsetX();
	const double y_multiplier = tile_layer.GetOffsetY();

	// Opengl flips vertically, thus the y multiplier is inverted
	uv.bottomRight.x = uv.topLeft.x + core::LossyCast<UvPositionT::PositionT::ValueT>(len_x * (x_multiplier + 1));
	uv.bottomRight.y = uv.bottomRight.y - core::LossyCast<UvPositionT::PositionT::ValueT>(len_y * y_multiplier);

	uv.topLeft.x = uv.bottomRight.x - len_x;
	uv.topLeft.y = uv.bottomRight.y - len_y;
}


void renderer::Renderer::GlDraw(const uint64_t index_count) noexcept {
	DEBUG_OPENGL_CALL(
		glDrawElements(GL_TRIANGLES, core::SafeCast<GLsizei>(index_count), GL_UNSIGNED_INT, nullptr)
	); // Pointer not needed as buffer is already bound
}

void renderer::Renderer::GlUpdateTileProjectionMatrix() noexcept {
	const auto max_tile_width = core::SafeCast<float>(tileWidth * 2);

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
		if (tileProjectionMatrixOffset > core::SafeCast<float>(smallest_axis) / 2 - max_zoom_offset) {
			tileProjectionMatrixOffset = core::SafeCast<float>(smallest_axis) / 2 - max_zoom_offset;
		}
	}

	mvpManager_.GlSetProjectionMatrix(MvpManager::ToProjMatrix(windowWidth_, windowHeight_, tileProjectionMatrixOffset));
}
