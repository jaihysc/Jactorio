// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
#define JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
#pragma once

#include <future>
#include <glm/glm.hpp>

#include "core/data_type.h"
#include "game/world/chunk_tile_layer.h"
#include "renderer/rendering/renderer_layer.h"
#include "renderer/rendering/spritemap_generator.h"

namespace jactorio::game
{
	class WorldData;
	class Chunk;
}

namespace jactorio::renderer
{
	class Renderer
	{
	public:
		static constexpr unsigned int tileWidth = 6;

		Renderer();

		// ======================================================================
		// Properties


		J_NODISCARD static unsigned int GetWindowWidth() { return windowWidth_; }
		J_NODISCARD static unsigned int GetWindowHeight() { return windowHeight_; }

		// ======================================================================
		// OpenGL calls

		static void GlClear();
		///
		/// \brief Resizes rendering buffers to new window size 
		void GlResizeWindow(unsigned int window_x, unsigned int window_y);


		J_NODISCARD size_t GetDrawThreads() const noexcept { return drawThreads_; }
		void GlSetDrawThreads(size_t threads);

		// ======================================================================
		// Rendering (Recalculated on window resize)

		/// Changes zoom 
		float tileProjectionMatrixOffset = 0;


		///
		/// \brief Renderer will lookup uv coords at the provided spritemap_coords
		void SetSpriteUvCoords(const SpriteUvCoordsT& spritemap_coords) noexcept {
			spritemapCoords_ = &spritemap_coords;
		}

		///
		/// \param world_data World to render
		/// \param player_x X Position of the player in tiles
		/// \param player_y Y Position of the player in tiles
		void RenderPlayerPosition(GameTickT game_tick,
		                          const game::WorldData& world_data,
		                          float player_x, float player_y);


		// ======================================================================
		// Rendering internals
	private:

		// Center the world at position 
		// This is achieved by offsetting the rendered chunks, for decimal numbers, the view matrix is used

		// Player movement is in tiles
		// Every chunk_width tiles, shift 1 chunk
		// Remaining tiles are offset

		// The top left of the tile at player position will be at the center of the screen

		// On a 1920 x 1080 screen:
		// 960 pixels from left
		// 540 pixels form top
		// Right and bottom varies depending on tile size
		void CalculateViewMatrix(float player_x, float player_y) noexcept;


		/// Extra chunks drawn around the border
		/// Hides the camera moving
		static constexpr int kPaddingChunks = 1;

		///
		/// \brief Number of tiles to draw to fill window dimensions
		J_NODISCARD core::Position2<int> GetTileDrawAmount() noexcept;

		///
		/// \brief All tiles drawn will have its position added to tile offset
		J_NODISCARD core::Position2<int> GetTileDrawOffset(int position_x, int position_y) noexcept;

		///
		/// \brief Top left chunk coordinates to begin drawing
		J_NODISCARD core::Position2<int> GetChunkDrawStart(int position_x, int position_y) noexcept;

		///
		/// \brief Number of chunks to draw to fill window dimensions
		J_NODISCARD core::Position2<int> GetChunkDrawAmount(int position_x, int position_y) noexcept;


		// Each chunk draw unit gets a renderer layer
		size_t drawThreads_ = 0;

		std::vector<std::future<void>> chunkDrawThreads_;

		/// Each thread gets 2 layers for rendering tiles + unique data
		std::vector<RendererLayer> renderLayers_;

		///
		/// \param row_start Chunk coordinate where the row of chunks starts
		/// \param chunk_span Number of chunks spanned
		/// \param layer_index Index to ChunkTileLayer 
		/// \param render_tile_offset_x Offset drawn tiles on screen by this tile amount
		/// \param render_pixel_offset_y Offset drawn tiles on screen by this pixel amount
		void PrepareChunkRow(RendererLayer& r_layer, const game::WorldData& world_data,
		                     core::Position2<int> row_start, int chunk_span, int layer_index,
		                     int render_tile_offset_x, int render_pixel_offset_y,
		                     GameTickT game_tick) const noexcept;

		void PrepareChunk(RendererLayer& r_layer, const game::Chunk& chunk,
		                  core::Position2<int> render_pixel_offset, int layer_index,
		                  GameTickT game_tick) const noexcept;

		static void ApplySpriteUvAdjustment(UvPositionT& uv,
											const UvPositionT& uv_offset) noexcept;

		static void ApplyMultiTileUvAdjustment(UvPositionT& uv,
											   const game::ChunkTileLayer& tile_layer) noexcept;


		///
		/// \brief Draws current data to the screen
		/// \param index_count Count of indices to draw
		static void GlDraw(uint64_t index_count) noexcept;

		///
		/// \brief Updates projection matrix and zoom level
		void GlUpdateTileProjectionMatrix();

		// ======================================================================

		/// Internal ids to spritemap positions
		const SpriteUvCoordsT* spritemapCoords_;

		static unsigned int windowWidth_;
		static unsigned int windowHeight_;
	};
};

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
