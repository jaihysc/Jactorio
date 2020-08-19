// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
#define JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
#pragma once

#include <exception>
#include <future>
#include <glm/glm.hpp>

#include "core/data_type.h"
#include "game/world/chunk_tile_layer.h"
#include "renderer/opengl/mvp_manager.h"
#include "renderer/rendering/renderer_layer.h"
#include "renderer/rendering/spritemap_generator.h"

namespace jactorio::game
{
	class WorldData;
	class Chunk;
	class ChunkTile;
}

namespace jactorio::renderer
{
	class Renderer
	{
	public:
		static constexpr unsigned int tileWidth = 6;

		static constexpr double kDepthBufferNearMax = 1.f;
		static constexpr double kDepthBufferFarMax  = -1.f;

		Renderer();

		// ======================================================================
		// Properties


		J_NODISCARD static unsigned int GetWindowWidth() noexcept { return windowWidth_; }
		J_NODISCARD static unsigned int GetWindowHeight() noexcept { return windowHeight_; }

		// ======================================================================
		// OpenGL calls

		///
		/// \brief Sets up OpenGl settings, only need to call once on program start
		static void GlSetup() noexcept;

		static void GlClear() noexcept;

		///
		/// \brief Resizes rendering buffers to new window size 
		void GlResizeWindow(unsigned int window_x, unsigned int window_y) noexcept;


		J_NODISCARD size_t GetDrawThreads() const noexcept { return drawThreads_; }
		void GlSetDrawThreads(size_t threads);

		// ======================================================================
		// Rendering (Recalculated on window resize)

		J_NODISCARD const MvpManager& GetMvpManager() const { return mvpManager_; }
		J_NODISCARD MvpManager& GetMvpManager() { return mvpManager_; }

		/// Changes zoom 
		float tileProjectionMatrixOffset = 0;


		///
		/// \brief Renderer will lookup uv coords at the provided spritemap_coords
		void SetSpriteUvCoords(const SpriteUvCoordsT& spritemap_coords) noexcept {
			spritemapCoords_ = &spritemap_coords;
		}

		///
		/// \brief Faster non range checked get into spritemapCoords_
		/// \remark Ensure key always exists
		J_NODISCARD static const SpriteUvCoordsT::mapped_type& GetSpriteUvCoords(const SpriteUvCoordsT& map,
		                                                                         SpriteUvCoordsT::key_type key) noexcept;

		J_NODISCARD const SpriteUvCoordsT::mapped_type& GetSpriteUvCoords(const SpriteUvCoordsT::key_type key) const noexcept {
			return GetSpriteUvCoords(*spritemapCoords_, key);
		}

		///
		/// \param world_data World to render
		/// \param player_x X Position of the player in tiles
		/// \param player_y Y Position of the player in tiles
		void GlRenderPlayerPosition(GameTickT game_tick,
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
		J_NODISCARD core::Position2<int> GetTileDrawAmount() const noexcept;

		///
		/// \brief All tiles drawn will have its position added to tile offset
		J_NODISCARD core::Position2<int> GetTileDrawOffset(int position_x, int position_y) const noexcept;

		///
		/// \brief Top left chunk coordinates to begin drawing
		J_NODISCARD core::Position2<int> GetChunkDrawStart(int position_x, int position_y) const noexcept;

		///
		/// \brief Number of chunks to draw to fill window dimensions
		J_NODISCARD core::Position2<int> GetChunkDrawAmount(int position_x, int position_y) const noexcept;


		// Each chunk draw unit gets a renderer layer
		size_t drawThreads_ = 0;

		std::vector<std::future<void>> chunkDrawThreads_;

		/// Each thread gets 2 layers for rendering tiles + unique data
		std::vector<RendererLayer> renderLayers_;

		///
		/// \param row_start Chunk coordinate where the row of chunks starts
		/// \param chunk_span Number of chunks spanned
		/// \param render_tile_offset Offset drawn tiles on screen by this tile amount
		void PrepareChunkRow(RendererLayer& r_layer, const game::WorldData& world_data, std::mutex& world_gen_mutex,
		                     core::Position2<int> row_start, int chunk_span,
		                     core::Position2<int> render_tile_offset,
		                     GameTickT game_tick) const noexcept;

		void PrepareChunk(RendererLayer& r_layer, const game::Chunk& chunk,
		                  core::Position2<int> render_tile_offset,
		                  GameTickT game_tick) const noexcept;

		void PrepareTileLayers(RendererLayer& r_layer, const game::ChunkTile& tile,
		                       const core::Position2<float>& pixel_pos,
		                       GameTickT game_tick) const noexcept;

		void PrepareOverlayLayers(RendererLayer& r_layer, const game::Chunk& chunk,
		                          core::Position2<int> render_tile_offset) const;


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
		void GlUpdateTileProjectionMatrix() noexcept;

		// ======================================================================

		MvpManager mvpManager_;

		/// Internal ids to spritemap positions
		const SpriteUvCoordsT* spritemapCoords_;

		static unsigned int windowWidth_;
		static unsigned int windowHeight_;
	};
};

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
