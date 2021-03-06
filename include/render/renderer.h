// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_RENDERER_H
#define JACTORIO_INCLUDE_RENDER_RENDERER_H
#pragma once

#include <future>
#include <glm/glm.hpp>

#include "core/data_type.h"
#include "core/orientation.h"
#include "render/opengl/mvp_manager.h"
#include "render/renderer_layer.h"

namespace jactorio::proto
{
    class Sprite;
}

namespace jactorio::game
{
    class Chunk;
    class ChunkTile;
    class World;
} // namespace jactorio::game

namespace jactorio::render
{
    class Renderer
    {
        // Rendering specifications
        //
        // The visible area to render to is a grid of pixels
        // - Top left is (0, 0); Bottom right is (window width, window height)
        // - No need to worry about matrices, pretend it does not exist while preparing coordinates
    public:
        static constexpr unsigned int tileWidth = 6;

        static constexpr double kDepthBufferNearMax = 1.;
        static constexpr double kDepthBufferFarMax  = -1.;

        Renderer();

        Renderer(const Renderer& other)     = delete;
        Renderer(Renderer&& other) noexcept = delete;

        // ======================================================================
        // Properties


        J_NODISCARD static unsigned int GetWindowWidth() noexcept {
            return windowWidth_;
        }
        J_NODISCARD static unsigned int GetWindowHeight() noexcept {
            return windowHeight_;
        }

        // ======================================================================
        // OpenGL calls

        /// Sets up OpenGl settings, only need to call once on program start
        static void GlSetup() noexcept;

        static void GlClear() noexcept;

        /// Resizes rendering buffers to new window size
        void GlResizeWindow(unsigned int window_x, unsigned int window_y) noexcept;


        J_NODISCARD size_t GetDrawThreads() const noexcept {
            return drawThreads_;
        }
        void GlSetDrawThreads(size_t threads);

        // ======================================================================
        // Rendering (Recalculated on window resize)

        J_NODISCARD const MvpManager& GetMvpManager() const {
            return mvpManager_;
        }
        J_NODISCARD MvpManager& GetMvpManager() {
            return mvpManager_;
        }


        /// Renderer will lookup uv coords at the provided spritemap_coords
        void SetSpriteUvCoords(const SpriteUvCoordsT& spritemap_coords) noexcept {
            spritemapCoords_ = &spritemap_coords;
        }

        /// Faster non range checked get into spritemapCoords_
        /// \remark Ensure key always exists
        J_NODISCARD static const SpriteUvCoordsT::mapped_type& GetSpriteUvCoords(
            const SpriteUvCoordsT& map, SpriteUvCoordsT::key_type key) noexcept;

        J_NODISCARD const SpriteUvCoordsT::mapped_type& GetSpriteUvCoords(
            const SpriteUvCoordsT::key_type key) const noexcept {
            return GetSpriteUvCoords(*spritemapCoords_, key);
        }


        /// Used for rendering methods, set prior to drawing
        void SetPlayerPosition(const Position2<float>& player_position) noexcept;

        /// \param world World to render
        void GlRenderPlayerPosition(GameTickT game_tick, const game::World& world);


        /// Allows use of Prepare methods
        void GlPrepareBegin();
        /// Renders, disallows use of Prepare methods after this call
        void GlPrepareEnd();

        void PrepareSprite(const WorldCoord& coord,
                           const proto::Sprite& sprite,
                           SpriteSetT set                    = 0,
                           const Position2<float>& dimension = {1, 1});

        // ======================================================================
        // Utility
        J_NODISCARD WorldCoord ScreenPosToWorldCoord(const Position2<float>& player_pos,
                                                     const Position2<int32_t>& screen_pos) const;

        /// \return On screen position of world coord, suitable for sending to buffers for rendering
        J_NODISCARD Position2<int32_t> WorldCoordToBufferPos(const Position2<float>& player_pos,
                                                             const WorldCoord& coord) const;

        /// Changes zoom
        float tileProjectionMatrixOffset = 0;

    private:
        // Rendering internals

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

        /// Number of tiles to draw to fill window dimensions
        J_NODISCARD Position2<int> GetTileDrawAmount() const noexcept;

        /// All tiles drawn will have its position added to tile offset
        J_NODISCARD Position2<int> GetTileDrawOffset(int position_x, int position_y) const noexcept;

        /// Top left chunk coordinates to begin drawing
        J_NODISCARD Position2<int> GetChunkDrawStart(int position_x, int position_y) const noexcept;

        /// Number of chunks to draw to fill window dimensions
        J_NODISCARD Position2<int> GetChunkDrawAmount(int position_x, int position_y) const noexcept;


        // Each chunk draw unit gets a render layer
        size_t drawThreads_ = 0;

        std::vector<std::future<void>> chunkDrawThreads_;

        /// Each thread gets 2 layers for rendering tiles + unique data
        std::vector<RendererLayer> renderLayers_;

        /// \param row_start Chunk coordinate where the row of chunks starts
        /// \param chunk_span Number of chunks spanned
        /// \param render_tile_offset Offset drawn tiles on screen by this tile amount
        void PrepareChunkRow(RendererLayer& r_layer,
                             const game::World& world,
                             std::mutex& world_gen_mutex,
                             Position2<int> row_start,
                             int chunk_span,
                             Position2<int> render_tile_offset,
                             GameTickT game_tick) const noexcept;

        void PrepareChunk(RendererLayer& r_layer,
                          const game::Chunk& chunk,
                          Position2<int> render_tile_offset,
                          GameTickT game_tick) const noexcept;

        void PrepareTileLayers(RendererLayer& r_layer,
                               const game::Chunk& chunk,
                               ChunkTileCoord ct_coord,
                               const Position2<float>& pixel_pos,
                               GameTickT game_tick) const noexcept;

        void PrepareOverlayLayers(RendererLayer& r_layer,
                                  const game::Chunk& chunk,
                                  Position2<int> render_tile_offset) const;


        /// Adjusts uv region to a sub region provided by uv_sub
        /// ----    ****
        /// ---- -> **--
        /// ----    **--
        static void ApplySpriteUvAdjustment(UvPositionT& uv, const UvPositionT& uv_sub) noexcept;

        static void ApplyMultiTileUvAdjustment(UvPositionT& uv, const game::ChunkTile& tile) noexcept;


        /// Allows layer to be drawn on
        static void GlPrepareBegin(RendererLayer& r_layer);
        /// Renders current layer, can no longer be drawn on
        static void GlPrepareEnd(RendererLayer& r_layer);

        /// Draws current data to the screen
        /// \param index_count Count of indices to draw
        static void GlDraw(uint64_t index_count) noexcept;

        /// Updates projection matrix and zoom level
        void GlUpdateTileProjectionMatrix() noexcept;


        MvpManager mvpManager_;

        /// Internal ids to spritemap positions
        const SpriteUvCoordsT* spritemapCoords_ = nullptr;

        /// Cached player's position, for convenience to avoid having to pass player around everywhere
        Position2<float> playerPosition_;

        static unsigned int windowWidth_;
        static unsigned int windowHeight_;
    };
}; // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_RENDERER_H
