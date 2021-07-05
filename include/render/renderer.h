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

        // Center the world at player position
        // The top left of the tile at player position will be at the center of the screen
        // On a 1920 x 1080 screen: center is
        // 960 pixels from left
        // 540 pixels form top

        // Achieved by offsetting the rendered chunks
        // Every kChunkWidth tiles, shift 1 chunk
        // Offset remainder tiles
        // Offset decimal tile, using view matrix
    public:
        static constexpr unsigned int tileWidth = 1;

        Renderer();

        Renderer(const Renderer& other)     = delete;
        Renderer(Renderer&& other) noexcept = delete;

        /// Sets up OpenGl settings, only need to call once on program start
        /// \exception RendererException Failed to setup
        static void GlSetup();
        static void GlClear() noexcept;


        // Window

        J_NODISCARD static unsigned int GetWindowWidth() noexcept;
        J_NODISCARD static unsigned int GetWindowHeight() noexcept;

        /// Resizes rendering buffers to new window size
        void GlResizeWindow(unsigned int window_x, unsigned int window_y) noexcept;


        // Rendering

        J_NODISCARD size_t GetDrawThreads() const noexcept;
        void GlSetDrawThreads(size_t threads);

        /// \return Zoom between [0,1] 0: Furthest out; 1:Furthest in
        J_NODISCARD float GetZoom() const noexcept;

        /// \param zoom Between [0,1] 0: Furthest out; 1:Furthest in. Auto clamps if out of range
        void SetZoom(float zoom) noexcept;

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


        //

        J_NODISCARD const MvpManager& GetMvpManager() const;
        J_NODISCARD MvpManager& GetMvpManager();


        // Utility

        J_NODISCARD WorldCoord ScreenPosToWorldCoord(const Position2<float>& player_pos,
                                                     const Position2<int32_t>& screen_pos) const;

        /// \return On screen position of world coord, suitable for sending to buffers for rendering
        J_NODISCARD Position2<uint16_t> WorldCoordToBufferPos(const Position2<float>& player_pos,
                                                              const WorldCoord& coord) const;

    private:
        /// \exception RendererException Failed to setup
        static void GlSetupTessellation();

        /// glDrawArrays
        static void GlDraw(uint64_t count) noexcept;
        /// glDrawBuffers
        /// Requires index buffer bound
        static void GlDrawIndex(uint64_t index_count) noexcept;


        void CalculateViewMatrix(Position2<int> i_player) noexcept;

        /// Number of tiles to draw to fill window dimensions
        J_NODISCARD Position2<int> GetTileDrawAmount() const noexcept;

        /// \param row_start Chunk coordinate where the row of chunks starts
        /// \param chunk_span Number of chunks spanned
        /// \param render_tile_offset Offset drawn tiles on screen by this tile amount
        void PrepareChunkRow(RendererLayer& r_layer,
                             const game::World& world,
                             std::mutex& world_gen_mutex,
                             Position2<int> row_start,
                             int chunk_span,
                             Position2<int> render_tile_offset) const noexcept;

        /// Prepares 1 chunk to r_layer using data from tex_coord_ids
        /// tex_coord_ids should never be nullptr
        /// \param tile_start Only prepares tiles with x >= tile_start.x && y >= tile_start.y
        /// \param tile_end Only prepares tiles with x < tile_end.x && y < tile_end.y
        void PrepareChunk(RendererLayer& r_layer,
                          const SpriteTexCoordIndexT* tex_coord_ids,
                          Position2<int> render_tile_offset,
                          Position2<uint8_t> tile_start,
                          Position2<uint8_t> tile_end) const noexcept;

        void PrepareOverlayLayers(RendererLayer& r_layer,
                                  const game::Chunk& chunk,
                                  Position2<int> render_tile_offset) const;


        /// Adjusts uv region to a sub region provided by uv_sub
        /// ----    ****
        /// ---- -> **--
        /// ----    **--
        static void ApplySpriteUvAdjustment(TexCoord& uv, const TexCoord& uv_sub) noexcept;

        static void ApplyMultiTileUvAdjustment(TexCoord& uv, const game::ChunkTile& tile) noexcept;


        /// Allows layer to be drawn on
        static void GlPrepareBegin(RendererLayer& r_layer);
        /// Renders current layer, can no longer be drawn on
        static void GlPrepareEnd(RendererLayer& r_layer);

        /// Updates projection matrix and zoom level
        /// \tparam zoom Between [0, 1]. 0 furthest, 1 closest
        void GlUpdateTileProjectionMatrix(float zoom) noexcept;


        size_t drawThreads_ = 0;
        std::vector<std::future<void>> chunkDrawThreads_;

        /// Each thread gets a render layer
        std::vector<RendererLayer> renderLayers_;

        MvpManager mvpManager_;

        float zoom_ = 0.5f;

        /// Cached player's position, for convenience to avoid having to pass player around everywhere
        Position2<float> playerPosition_;

        static unsigned int windowWidth_;
        static unsigned int windowHeight_;
    };
}; // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_RENDERER_H
