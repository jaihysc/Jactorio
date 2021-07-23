// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_TILE_RENDERER_H
#define JACTORIO_INCLUDE_RENDER_TILE_RENDERER_H
#pragma once

#include <future>
#include <glm/glm.hpp>

#include "core/data_type.h"
#include "core/orientation.h"
#include "render/opengl/shader.h"
#include "render/trender_buffer.h"

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
    struct RendererCommon;
    class Spritemap;
    class Texture;

    class TileRenderer
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

        /// GLSL slot which texture will be bound to
        static constexpr auto kTextureSlot = 0;

    public:
        static constexpr unsigned int tileWidth = 1;

        explicit TileRenderer(RendererCommon& common);

        TileRenderer(const TileRenderer& other)     = delete;
        TileRenderer(TileRenderer&& other) noexcept = delete;

        /// Sets up renderer + OpenGl settings, only need to call once on program start
        /// \exception RendererException Failed to setup
        void Init();
        /// \remark spritemap and texture must be kept alive for lifetime of Renderer
        void InitTexture(const Spritemap& spritemap, const Texture& texture) noexcept;
        /// \exception std::runtime_error Too many tex coords for shader
        void InitShader();


        static void GlClear() noexcept;
        /// Sets up required resources + states for rendering
        void GlBind() const noexcept;

        // Window

        J_NODISCARD static unsigned int GetWindowWidth() noexcept;
        J_NODISCARD static unsigned int GetWindowHeight() noexcept;

        /// Resizes rendering buffers to new window size
        void GlResizeWindow(unsigned int window_x, unsigned int window_y) noexcept;


        // Rendering

        J_NODISCARD size_t GetDrawThreads() const noexcept;
        void GlSetDrawThreads(size_t threads);

        /// Apply to tex coord id to enable/disable animations
        J_NODISCARD static SpriteTexCoordIndexT GetAnimationOffset() noexcept;

        /// \return Zoom between [0,1] 0: Furthest out; 1:Furthest in
        J_NODISCARD float GetZoom() const noexcept;

        /// \param zoom Between [0,1] 0: Furthest out; 1:Furthest in. Auto clamps if out of range
        void SetZoom(float zoom) noexcept;

        /// Used for rendering methods, set prior to drawing
        void SetPlayerPosition(const Position2<float>& player_position) noexcept;

        /// Renders player position
        /// \param world World to render
        void GlRender(const game::World& world);


        /// Allows use of Prepare methods
        void GlPrepareBegin();
        /// Renders, disallows use of Prepare methods after this call
        void GlPrepareEnd();

        /// Provided tex coord id increments along dimension.x, left to right; then dimension.y up to down
        void PrepareSprite(const WorldCoord& coord,
                           SpriteTexCoordIndexT tex_coord_id,
                           const Dimension& dimension = {1, 1});

        // Utility

        J_NODISCARD WorldCoord ScreenPosToWorldCoord(const Position2<float>& player_pos,
                                                     const Position2<int32_t>& screen_pos) const;
        /// Uses cached player position
        J_NODISCARD WorldCoord ScreenPosToWorldCoord(const Position2<int32_t>& screen_pos) const;

        /// \return On screen position of world coord, suitable for sending to buffers for rendering
        J_NODISCARD Position2<int16_t> WorldCoordToBufferPos(const Position2<float>& player_pos,
                                                             const WorldCoord& coord) const;
        /// Uses cached player position
        /// \return On screen position of world coord, suitable for sending to buffers for rendering
        J_NODISCARD Position2<int16_t> WorldCoordToBufferPos(const WorldCoord& coord) const;

    private:
        /// \exception RendererException Failed to setup
        static void GlSetupTessellation();

        /// glDrawArrays
        static void GlDraw(uint64_t count) noexcept;


        void CalculateViewMatrix(Position2<int> i_player) noexcept;

        /// Number of tiles to draw to fill window dimensions
        J_NODISCARD Position2<int> GetTileDrawAmount() const noexcept;

        /// \param row_start Chunk coordinate where the row of chunks starts
        /// \param chunk_span Number of chunks spanned
        /// \param render_tile_offset Offset drawn tiles on screen by this tile amount
        void PrepareChunkRow(TRenderBuffer& r_layer,
                             const game::World& world,
                             std::mutex& world_gen_mutex,
                             Position2<int> row_start,
                             int chunk_span,
                             Position2<int> render_tile_offset) const noexcept;

        /// Prepares 1 chunk to r_layer using data from tex_coord_ids
        /// tex_coord_ids should never be nullptr
        /// \param tile_start Only prepares tiles with x >= tile_start.x && y >= tile_start.y
        /// \param tile_amount Amount in x, y directions
        void PrepareChunk(TRenderBuffer& r_layer,
                          const SpriteTexCoordIndexT* tex_coord_ids,
                          Position2<int> render_tile_offset,
                          Position2<uint8_t> tile_start,
                          Position2<uint8_t> tile_amount) const noexcept;

        void PrepareOverlayLayers(TRenderBuffer& r_layer,
                                  const game::Chunk& chunk,
                                  Position2<int> render_tile_offset) const;


        /// Sends the tex coords for next animation frame to GPU
        void UpdateAnimationTexCoords() const noexcept;


        /// Allows layer to be drawn on
        static void GlPrepareBegin(TRenderBuffer& r_layer);
        /// Renders current layer, can no longer be drawn on
        static void GlPrepareEnd(TRenderBuffer& r_layer);

        /// Updates projection matrix and zoom level
        /// \tparam zoom Between [0, 1]. 0 furthest, 1 closest (auto clamps if out of range)
        void GlUpdateTileProjectionMatrix(float zoom) noexcept;

        RendererCommon* common_ = nullptr;

        Shader shader_;
        const Spritemap* spritemap_ = nullptr;
        const Texture* texture_     = nullptr;

        std::vector<std::future<void>> chunkDrawThreads_;

        /// Each thread gets a render layer
        std::vector<TRenderBuffer> renderLayers_;

        float zoom_ = 0.5f;

        /// Cached player's position, for convenience to avoid having to pass player around everywhere
        Position2<float> playerPosition_;

        static unsigned int windowWidth_;
        static unsigned int windowHeight_;

        static SpriteTexCoordIndexT animationOffset_;
    };
}; // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_TILE_RENDERER_H
