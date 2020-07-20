// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
#define JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
#pragma once

#include <glm/glm.hpp>

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

		static void SetSpritemapCoords(const std::unordered_map<unsigned, core::QuadPosition>& spritemap_coords);

		///
		/// \param internal_id internal id of sprite prototype
		static core::QuadPosition GetSpritemapCoords(unsigned int internal_id);

		J_NODISCARD static unsigned int GetWindowWidth() { return windowWidth_; }
		J_NODISCARD static unsigned int GetWindowHeight() { return windowHeight_; }

		// ======================================================================
		// Rendering (Recalculated on window resize)

		/// Changes zoom 
		float tileProjectionMatrixOffset = 0;

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
		void CalculateViewMatrix(float player_x, float player_y);


		///
		/// \brief Number of tiles to draw to fill window dimensions
		J_NODISCARD core::Position2<int> GetTileDrawAmount() noexcept;

		///
		/// \brief Top left chunk coordinates to begin drawing
		J_NODISCARD core::Position2<int> GetChunkDrawStart(int position_x, int position_y) noexcept;
		///
		/// \brief Number of chunks to draw to fill window dimensions
		J_NODISCARD core::Position2<int> GetChunkDrawAmount(int position_x, int position_y) noexcept;


		// Each chunk draw unit gets a renderer layer
		std::vector<RendererLayer> renderLayers_;
		
		///
		/// \brief Draws chunks to screen
		/// Create multiple units in threads to draw multiple chunks at once
		void ChunkDrawUnit();


		// ======================================================================
		// Gl methods must be called from an OpenGL context!
	public:
		static void GlClear();

		///
		/// \brief Resizes opengl buffers used for rendering,
		void GlResizeBuffers(unsigned int window_x, unsigned int window_y);

	private:
		///
		/// \brief Draws current data to the screen
		/// \param element_count Count of elements to draw (1 element = 6 indices)
		static void GlDraw(unsigned int element_count);

		///
		/// \brief Updates projection matrix and zoom level
		void GlUpdateTileProjectionMatrix();


		// ======================================================================

		// Internal ids to spritemap positions
		static std::unordered_map<unsigned int, core::QuadPosition> spritemapCoords_;

		static unsigned int windowWidth_;
		static unsigned int windowHeight_;


		unsigned int gridElementsCount_ = 0;

		unsigned int tileCountX_ = 0;
		unsigned int tileCountY_ = 0;
	};
};

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
