// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include "render/renderer.h"

#include <algorithm>
#include <cmath>
#include <future>
#include <glm/gtc/matrix_transform.hpp>

#include "core/execution_timer.h"
#include "game/world/world.h"
#include "proto/sprite.h"
#include "render/opengl/error.h"
#include "render/renderer_exception.h"

using namespace jactorio;

unsigned int render::Renderer::windowWidth_  = 0;
unsigned int render::Renderer::windowHeight_ = 0;

render::Renderer::Renderer() {
    // This does not need to change as everything is already prepared in world space
    const glm::mat4 model_matrix = translate(glm::mat4(1.f), glm::vec3(0, 0, 0));
    mvpManager_.GlSetModelMatrix(model_matrix);

    // Get window size
    GLint m_viewport[4];
    glGetIntegerv(GL_VIEWPORT, m_viewport);

    GlResizeWindow(m_viewport[2], m_viewport[3]);
}

void render::Renderer::GlSetup() {
    // Enables transparency in textures
    DEBUG_OPENGL_CALL(glEnable(GL_BLEND));
    DEBUG_OPENGL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // Depth buffer
    DEBUG_OPENGL_CALL(glEnable(GL_DEPTH_TEST));
    DEBUG_OPENGL_CALL(glDepthFunc(GL_LEQUAL));

    GlSetupTessellation();
}

void render::Renderer::GlClear() noexcept {
    DEBUG_OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}


unsigned render::Renderer::GetWindowWidth() noexcept {
    return windowWidth_;
}

unsigned render::Renderer::GetWindowHeight() noexcept {
    return windowHeight_;
}

void render::Renderer::GlResizeWindow(const unsigned int window_x, const unsigned int window_y) noexcept {
    // glViewport is critical, changes the size of the rendering area
    DEBUG_OPENGL_CALL(glViewport(0, 0, window_x, window_y));

    // Initialize fields
    windowWidth_  = window_x;
    windowHeight_ = window_y;
    GlUpdateTileProjectionMatrix(zoom_);

    for (auto& render_layer : renderLayers_) {
        render_layer.ResizeDefault();
    }
}


size_t render::Renderer::GetDrawThreads() const noexcept {
    return drawThreads_;
}

void render::Renderer::GlSetDrawThreads(const size_t threads) {
    assert(threads > 0);
    drawThreads_ = threads;

    chunkDrawThreads_.resize(drawThreads_);

    renderLayers_.clear(); // Opengl probably stores some internal memory addresses, so each layer must be recreated
    renderLayers_.resize(drawThreads_);

    assert(chunkDrawThreads_.size() == drawThreads_);
    assert(renderLayers_.size() == drawThreads_);
}

float render::Renderer::GetZoom() const noexcept {
    return zoom_;
}

void render::Renderer::SetZoom(float zoom) noexcept {
    if (zoom < 0.f) {
        zoom = 0.f;
    }
    else if (zoom > 1.f) {
        zoom = 1.f;
    }

    zoom_ = zoom;
}

void render::Renderer::SetPlayerPosition(const Position2<float>& player_position) noexcept {
    playerPosition_ = player_position;
}

void render::Renderer::GlRenderPlayerPosition(const GameTickT game_tick, const game::World& world) {
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

    // Negative moves window right and down

    // Truncated player position
    const auto i_player = Position2{LossyCast<int>(playerPosition_.x), LossyCast<int>(playerPosition_.y)};

    GlUpdateTileProjectionMatrix(zoom_ - 0.01f);
    CalculateViewMatrix(i_player);
    mvpManager_.CalculateMvpMatrix();

    // View matrix depends on mvp matrix to calculate correct camera offset
    // Thus we must calculate the mvp matrix twice
    // Once so we can properly calculate the view matrix, then a second time for final mvp
    CalculateViewMatrix(i_player);

    // Player has not moved an entire chunk's width yet, offset the tiles
    // Modulus chunkWidth to make it snap back to 0 after offsetting the entirety of a chunk
    // Inverted to move the tiles AWAY from the screen instead of following the screen
    auto tile_offset = Position2{i_player.x % game::Chunk::kChunkWidth * -1, //
                                 i_player.y % game::Chunk::kChunkWidth * -1};

    auto chunk_start = Position2{i_player.x / game::Chunk::kChunkWidth, //
                                 i_player.y / game::Chunk::kChunkWidth};

    const auto tile_amount = GetTileDrawAmount();

    // Center player position on screen
    chunk_start.x -= tile_amount.x / 2 / game::Chunk::kChunkWidth;
    chunk_start.y -= tile_amount.y / 2 / game::Chunk::kChunkWidth;

    tile_offset.x += tile_amount.x / 2 % game::Chunk::kChunkWidth; // Remaining tile distance after div
    tile_offset.y += tile_amount.y / 2 % game::Chunk::kChunkWidth;


    // Render extra chunks since camera moves
    tile_offset.x -= 2 * game::Chunk::kChunkWidth;
    tile_offset.y -= 2 * game::Chunk::kChunkWidth;
    chunk_start.x -= 2;
    chunk_start.y -= 2;

    auto chunk_amount = Position2{(tile_amount.x - tile_offset.x) / game::Chunk::kChunkWidth,
                                  (tile_amount.y - tile_offset.y) / game::Chunk::kChunkWidth};
    // Add 1 back since division may truncate decimals
    chunk_amount.x += 1;
    chunk_amount.y += 1;

    // Zoom in more to hide the black edges from camera movement
    GlUpdateTileProjectionMatrix(zoom_);
    mvpManager_.CalculateMvpMatrix();
    mvpManager_.UpdateShaderMvp();

    // ======================================================================


    std::mutex world_gen_mutex;

    // Thread order: (Helps reduce stalls as some threads are always active)
    // - Start all
    // - Wait for thread 1, draw, continue thread 1
    // - Wait for thread 2, draw, continue thread 2
    // ... Repeat until done

    const auto needed_threads   = chunk_amount.y;
    const auto threads_to_start = std::min(LossyCast<int>(drawThreads_), needed_threads);

    // Start all threads
    int i = 0;
    for (; i < threads_to_start; ++i) {
        auto& r_layer = renderLayers_[i];
        GlPrepareBegin(r_layer);


        Position2 row_start{chunk_start.x, chunk_start.y + i};
        Position2 render_tile_offset{tile_offset.x, i * game::Chunk::kChunkWidth + tile_offset.y};

        chunkDrawThreads_[i] = std::async(std::launch::async,
                                          &Renderer::PrepareChunkRow,
                                          this,
                                          std::ref(r_layer),
                                          std::ref(world),
                                          std::ref(world_gen_mutex),
                                          row_start,
                                          chunk_amount.x,
                                          render_tile_offset);
    }

    // Wait for thread n, draw n, ...

    std::size_t thread_n = 0; // The thread currently waiting for
    for (; i < needed_threads; ++i) {
        chunkDrawThreads_[thread_n].wait();

        auto& r_layer = renderLayers_[thread_n];
        GlPrepareEnd(r_layer);

        GlPrepareBegin(r_layer);

        Position2 row_start{chunk_start.x, chunk_start.y + i};
        Position2 render_tile_offset{tile_offset.x, i * game::Chunk::kChunkWidth + tile_offset.y};

        chunkDrawThreads_[thread_n] = std::async(std::launch::async,
                                                 &Renderer::PrepareChunkRow,
                                                 this,
                                                 std::ref(r_layer),
                                                 std::ref(world),
                                                 std::ref(world_gen_mutex),
                                                 row_start,
                                                 chunk_amount.x,
                                                 render_tile_offset);

        thread_n++;
        if (thread_n >= drawThreads_)
            thread_n = 0;
    }

    // Continue off from prior loop, but only waiting for threads and drawing
    for (int j = 0; j < threads_to_start; ++j) {
        chunkDrawThreads_[thread_n].wait();

        auto& r_layer = renderLayers_[thread_n];
        GlPrepareEnd(r_layer);

        thread_n++;
        if (thread_n >= drawThreads_)
            thread_n = 0;
    }
}

void render::Renderer::GlPrepareBegin() {
    GlPrepareBegin(renderLayers_[0]);
}
void render::Renderer::GlPrepareEnd() {
    GlPrepareEnd(renderLayers_[0]);
}

void render::Renderer::PrepareSprite(const WorldCoord& coord,
                                     const proto::Sprite& sprite,
                                     const SpriteSetT set,
                                     const Position2<float>& dimension) {
    auto& r_layer = renderLayers_[0];

    const auto screen_pos = WorldCoordToBufferPos(playerPosition_, coord);

    // Only capable of render 1 x 1 with current renderer
    assert(dimension.x == 1);
    assert(dimension.y == 1);

    r_layer.PushBack({{screen_pos.x, screen_pos.y, 5}, sprite.texCoordId});
}


const render::MvpManager& render::Renderer::GetMvpManager() const {
    return mvpManager_;
}

render::MvpManager& render::Renderer::GetMvpManager() {
    return mvpManager_;
}


WorldCoord render::Renderer::ScreenPosToWorldCoord(const Position2<float>& player_pos,
                                                   const Position2<int32_t>& screen_pos) const {
    const auto truncated_player_pos_x = SafeCast<float>(LossyCast<int>(player_pos.x));
    const auto truncated_player_pos_y = SafeCast<float>(LossyCast<int>(player_pos.y));

    const auto norm_screen_pos_x = 2 * (SafeCast<float>(screen_pos.x) / SafeCast<float>(GetWindowWidth())) - 1;
    const auto norm_screen_pos_y = 2 * (SafeCast<float>(screen_pos.y) / SafeCast<float>(GetWindowHeight())) - 1;

    const glm::vec4 adjusted_screen_pos =
        mvpManager_.GetMvpMatrix() / glm::vec4(norm_screen_pos_x, norm_screen_pos_y, 1, 1);

    // Normalize window center between -1 and 1, since is window center, simplifies to 0
    auto screen_center_pos = mvpManager_.GetMvpMatrix() / glm::vec4(0, 0, 1.f, 1.f);

    // Players can be partially on a tile, adjust the center accordingly to the correct location
    screen_center_pos.x -= SafeCast<float>(tileWidth) * (player_pos.x - truncated_player_pos_x);
    screen_center_pos.y += SafeCast<float>(tileWidth) * (player_pos.y - truncated_player_pos_y);

    // WorldCoord sign direction
    const auto pixels_from_center_x = adjusted_screen_pos.x - screen_center_pos.x;
    const auto pixels_from_center_y = screen_center_pos.y - adjusted_screen_pos.y;

    auto tile_x = truncated_player_pos_x + pixels_from_center_x / LossyCast<float>(tileWidth);
    auto tile_y = truncated_player_pos_y + pixels_from_center_y / LossyCast<float>(tileWidth);

    // Subtract extra tile if negative because no tile exists at -0, -0
    if (tile_x < 0)
        tile_x -= 1;
    if (tile_y < 0)
        tile_y -= 1;

    return {LossyCast<WorldCoordAxis>(tile_x), LossyCast<WorldCoordAxis>(tile_y)};
}

Position2<uint16_t> render::Renderer::WorldCoordToBufferPos(const Position2<float>& player_pos,
                                                            const WorldCoord& coord) const {
    const auto truncated_player_pos_x = SafeCast<float>(LossyCast<int>(player_pos.x));
    const auto truncated_player_pos_y = SafeCast<float>(LossyCast<int>(player_pos.y));

    // From center, without matrix adjustments (WorldCoord sign direction)
    const auto pixels_from_center_x = (coord.x - truncated_player_pos_x) * tileWidth;
    const auto pixels_from_center_y = (coord.y - truncated_player_pos_y) * tileWidth;

    // From top left of screen (Top left 0, 0)
    const auto buffer_center_pos = mvpManager_.GetMvpMatrix() / glm::vec4(0, 0, 1.f, 1.f);
    auto buffer_pos_x            = pixels_from_center_x + buffer_center_pos.x;
    auto buffer_pos_y            = pixels_from_center_y + buffer_center_pos.y;

    // Players can be partially on a tile, remove partial distance for top left of a tile
    buffer_pos_x -= SafeCast<float>(tileWidth) * (player_pos.x - truncated_player_pos_x);
    buffer_pos_y -= SafeCast<float>(tileWidth) * (player_pos.y - truncated_player_pos_y);

    // Sometimes has float 8.99999, which is wrongly truncated to 8, thus must round first
    return {LossyCast<uint16_t>(std::round(buffer_pos_x)), LossyCast<uint16_t>(std::round(buffer_pos_y))};
}

// ======================================================================

void render::Renderer::GlSetupTessellation() {
    // Setup tessellation
    constexpr auto input_patch_vertices  = 1;
    constexpr auto output_patch_vertices = 4;

    GLint max_patch_vertices;
    DEBUG_OPENGL_CALL(glGetIntegerv(GL_MAX_PATCH_VERTICES, &max_patch_vertices));
    if (max_patch_vertices < input_patch_vertices) {
        throw RendererException("Requested input patch vertices not supported: " +
                                std::to_string(input_patch_vertices));
    }
    if (max_patch_vertices < output_patch_vertices) {
        throw RendererException("Requested output patch vertices not supported: " +
                                std::to_string(output_patch_vertices));
    }

    DEBUG_OPENGL_CALL(glPatchParameteri(GL_PATCH_VERTICES, input_patch_vertices));


    const GLfloat inner_level[]{1, 1};
    const GLfloat outer_level[]{1, 1, 1, 1};

    DEBUG_OPENGL_CALL(glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, inner_level));
    DEBUG_OPENGL_CALL(glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, outer_level));
}

void render::Renderer::GlDraw(const uint64_t count) noexcept {
    DEBUG_OPENGL_CALL(glDrawArrays(GL_PATCHES, 0, SafeCast<GLsizei>(count)));
}

void render::Renderer::GlDrawIndex(const uint64_t index_count) noexcept {
    DEBUG_OPENGL_CALL(glDrawElements(GL_PATCHES,
                                     SafeCast<GLsizei>(index_count),
                                     GL_UNSIGNED_INT,
                                     nullptr)); // Pointer not needed as index buffer is already bound
}

void render::Renderer::CalculateViewMatrix(const Position2<int> i_player) noexcept {
    // Decimal is used to shift the camera
    // Invert the movement to give the illusion of moving in the correct direction
    const auto camera_offset =
        Position2{(playerPosition_.x - SafeCast<float>(i_player.x)) * SafeCast<float>(tileWidth) * -1,
                  (playerPosition_.y - SafeCast<float>(i_player.y)) * SafeCast<float>(tileWidth) * -1};

    // Remaining pixel distance not covered by tiles and chunks are covered by the view matrix to center
    const auto tile_amount = GetTileDrawAmount();

    const auto& view_transform = mvpManager_.GetViewTransform();

    // Operations inside lossy cast centers of player position at the center of screen
    // Use truncating division, such that for odd # of tiles (e.g 5), the top left of third tile is at center of screen
    // instead of the center of the tile
    // A LossyCast is used as during startup, tile_amount is invalid since the mvp matrix is invalid
    view_transform->x =
        LossyCast<float>(windowWidth_) / 2 - LossyCast<float>(tile_amount.x / 2 * tileWidth) + camera_offset.x;
    view_transform->y =
        LossyCast<float>(windowHeight_) / 2 - LossyCast<float>(tile_amount.y / 2 * tileWidth) + camera_offset.y;

    mvpManager_.UpdateViewTransform();
}

Position2<int> render::Renderer::GetTileDrawAmount() const noexcept {
    const auto matrix = glm::vec4(1, -1, 1, 1) / mvpManager_.GetMvpMatrix();
    return Position2{LossyCast<int>(matrix.x / LossyCast<double>(tileWidth) * 2) + 2,
                     LossyCast<int>(matrix.y / LossyCast<double>(tileWidth) * 2) + 2};
}

void render::Renderer::PrepareChunkRow(RendererLayer& r_layer,
                                       const game::World& world,
                                       std::mutex& world_gen_mutex,
                                       const Position2<int> row_start,
                                       const int chunk_span,
                                       const Position2<int> render_tile_offset) const noexcept {
    auto [tex_ids, readable_chunks] = world.GetChunkTexCoordIds(row_start);

    if (readable_chunks < chunk_span) {
        std::lock_guard gen_guard{world_gen_mutex};

        for (int x = readable_chunks; x < chunk_span; ++x) {
            const auto chunk_x = x + row_start.x;
            world.QueueChunkGeneration({chunk_x, row_start.y});
        }
    }

    // Allocate for the maximum possible tile layers to render
    const auto required_r_layer_capacity =
        SafeCast<uint32_t>(chunk_span * game::Chunk::kChunkArea * game::kTileLayerCount);
    if (required_r_layer_capacity >= r_layer.Capacity()) {
        r_layer.Reserve(required_r_layer_capacity);
        return;
    }

    // Prevents PrepareChunk from having a pixel x/y < 0
    auto skip_tiles_top = -render_tile_offset.y / SafeCast<int>(tileWidth);
    if (skip_tiles_top < 0) { // No tiles to skip
        skip_tiles_top = 0;
    }

    auto tiles_prepare_y = (windowHeight_ - render_tile_offset.y) / SafeCast<int>(tileWidth);
    if (tiles_prepare_y > game::Chunk::kChunkWidth) {
        tiles_prepare_y = game::Chunk::kChunkWidth;
    }

    for (int x = 0; x < std::min(readable_chunks, chunk_span); ++x) {
        const auto chunk_render_tile_offset_x = x * game::Chunk::kChunkWidth + render_tile_offset.x;

        auto skip_tiles_left = -chunk_render_tile_offset_x / SafeCast<int>(tileWidth);
        if (skip_tiles_left < 0) {
            skip_tiles_left = 0;
        }

        auto tiles_prepare_x = (windowWidth_ - render_tile_offset.x) / SafeCast<int>(tileWidth);
        if (tiles_prepare_x > game::Chunk::kChunkWidth) {
            tiles_prepare_x = game::Chunk::kChunkWidth;
        }

        // PrepareOverlayLayers(r_layer, chunk, render_tile_offset); // Unused
        PrepareChunk(r_layer,
                     tex_ids,
                     {chunk_render_tile_offset_x, render_tile_offset.y},
                     {SafeCast<uint8_t>(skip_tiles_left), SafeCast<uint8_t>(skip_tiles_top)},
                     {SafeCast<uint8_t>(tiles_prepare_x), SafeCast<uint8_t>(tiles_prepare_y)});
        tex_ids += game::Chunk::kChunkArea * game::kTileLayerCount;
    }
}

FORCEINLINE void render::Renderer::PrepareChunk(RendererLayer& r_layer,
                                                const SpriteTexCoordIndexT* tex_coord_ids,
                                                const Position2<int> render_tile_offset,
                                                const Position2<uint8_t> tile_start,
                                                const Position2<uint8_t> tile_end) const noexcept {
    tex_coord_ids += tile_start.y * game::Chunk::kChunkWidth * game::kTileLayerCount;


    for (ChunkTileCoordAxis tile_y = tile_start.y; tile_y < tile_end.y; ++tile_y) {
        const auto pixel_y = (render_tile_offset.y + tile_y) * SafeCast<int>(tileWidth);

        tex_coord_ids += tile_start.x * game::kTileLayerCount;

        for (ChunkTileCoordAxis tile_x = tile_start.x; tile_x < tile_end.x; ++tile_x) {
            const auto pixel_x = (render_tile_offset.x + tile_x) * SafeCast<int>(tileWidth);

            // Manually unrolled 3 times
            static_assert(3 == game::kTileLayerCount);

            // 1
            if (tex_coord_ids[0] != 0) {
                r_layer.UncheckedPushBack(
                    {{SafeCast<uint16_t>(pixel_x), SafeCast<uint16_t>(pixel_y), 0}, tex_coord_ids[0]});
            }

            // 2
            if (tex_coord_ids[1] != 0) {
                r_layer.UncheckedPushBack(
                    {{SafeCast<uint16_t>(pixel_x), SafeCast<uint16_t>(pixel_y), 1}, tex_coord_ids[1]});
            }

            // 3
            // if (tex_coord_ids[2] != 0) {
            //     r_layer.UncheckedPushBack(
            //         {{SafeCast<uint16_t>(pixel_x), SafeCast<uint16_t>(pixel_y), 2}, tex_coord_ids[2]});
            // }

            tex_coord_ids += 3;
        }

        tex_coord_ids += (game::Chunk::kChunkWidth - tile_end.x) * game::kTileLayerCount;
    }
}

FORCEINLINE void render::Renderer::PrepareOverlayLayers(RendererLayer& r_layer,
                                                        const game::Chunk& chunk,
                                                        const Position2<int> render_tile_offset) const {
    assert(false); // PushBack usage needs to be updated

    for (int layer_index = 0; layer_index < game::kOverlayLayerCount; ++layer_index) {
        const auto& overlay_container = chunk.overlays[layer_index];

        for (const auto& overlay : overlay_container) {
            // Only capable of render 1 x 1 with current renderer
            assert(overlay.size.x == 1);
            assert(overlay.size.y == 1);

            // Fix when this method is used
            // r_layer.PushBack({{(render_tile_offset.x + overlay.position.x) * SafeCast<float>(tileWidth),
            //                    (render_tile_offset.y + overlay.position.y) * SafeCast<float>(tileWidth),
            //                    overlay.position.z},
            // 0});
        }
    }
}

void render::Renderer::ApplySpriteUvAdjustment(TexCoord& uv, const TexCoord& uv_sub) noexcept {
    const auto diff_x = uv.bottomRight.x - uv.topLeft.x;
    const auto diff_y = uv.bottomRight.y - uv.topLeft.y;

    assert(diff_x >= 0);
    assert(diff_y >= 0);

    // Calculate bottom first since it needs the unmodified top_left
    uv.bottomRight.x = uv.topLeft.x + diff_x * uv_sub.bottomRight.x;
    uv.bottomRight.y = uv.topLeft.y + diff_y * uv_sub.bottomRight.y;

    uv.topLeft.x += diff_x * uv_sub.topLeft.x;
    uv.topLeft.y += diff_y * uv_sub.topLeft.y;
}

void render::Renderer::ApplyMultiTileUvAdjustment(TexCoord& uv, const game::ChunkTile& tile) noexcept {
    const auto& mt_data = tile.GetDimension();

    // Calculate the correct UV coordinates for multi-tile entities
    // Split the sprite into sections and stretch over multiple tiles if this entity is multi tile

    // Total length of the sprite, to be split among the different tiles
    const auto len_x = (uv.bottomRight.x - uv.topLeft.x) / SafeCast<float>(mt_data.x);
    const auto len_y = (uv.bottomRight.y - uv.topLeft.y) / SafeCast<float>(mt_data.y);

    const double x_multiplier = tile.GetOffsetX();
    const double y_multiplier = tile.GetOffsetY();

    // Opengl flips vertically, thus the y multiplier is inverted
    uv.bottomRight.x = uv.topLeft.x + LossyCast<TexCoord::PositionT::ValueT>(len_x * (x_multiplier + 1));
    uv.bottomRight.y = uv.bottomRight.y - LossyCast<TexCoord::PositionT::ValueT>(len_y * y_multiplier);

    uv.topLeft.x = uv.bottomRight.x - len_x;
    uv.topLeft.y = uv.bottomRight.y - len_y;
}

void render::Renderer::GlPrepareBegin(RendererLayer& r_layer) {
    r_layer.Clear();
    r_layer.GlWriteBegin();
}

void render::Renderer::GlPrepareEnd(RendererLayer& r_layer) {
    r_layer.GlWriteEnd();
    r_layer.GlBindBuffers();
    r_layer.GlHandleBufferResize();
    GlDraw(r_layer.Size());
}

void render::Renderer::GlUpdateTileProjectionMatrix(const float zoom) noexcept {
    // Must subtract some because zooming to EXACTLY half leaves no pixels remaining, making everything invisible
    const auto max_pixel_zoom     = std::min(windowWidth_, windowHeight_) / 2.f - 1;
    constexpr auto min_pixel_zoom = 1.f;

    auto pixel_zoom = zoom * max_pixel_zoom;

    if (pixel_zoom < min_pixel_zoom) {
        pixel_zoom = min_pixel_zoom;
    }
    else if (pixel_zoom > max_pixel_zoom) {
        pixel_zoom = max_pixel_zoom;
    }

    assert(pixel_zoom > 0);
    mvpManager_.GlSetProjectionMatrix(MvpManager::ToProjMatrix(windowWidth_, windowHeight_, pixel_zoom));
}
