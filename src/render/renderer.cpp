// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include "render/renderer.h"

#include <cmath>
#include <future>
#include <glm/gtc/matrix_transform.hpp>

#include "core/execution_timer.h"
#include "game/world/world.h"
#include "proto/sprite.h"
#include "render/opengl/error.h"

using namespace jactorio;

unsigned int render::Renderer::windowWidth_  = 0;
unsigned int render::Renderer::windowHeight_ = 0;

render::Renderer::Renderer() {
    // Initialize model matrix
    const glm::mat4 model_matrix = translate(glm::mat4(1.f), glm::vec3(0, 0, 0));
    mvpManager_.GlSetModelMatrix(model_matrix);

    // Get window size
    GLint m_viewport[4];
    glGetIntegerv(GL_VIEWPORT, m_viewport);

    GlResizeWindow(m_viewport[2], m_viewport[3]);
}

// ======================================================================

void render::Renderer::GlSetup() noexcept {
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

void render::Renderer::GlClear() noexcept {
    DEBUG_OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void render::Renderer::GlResizeWindow(const unsigned int window_x, const unsigned int window_y) noexcept {
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

void render::Renderer::GlSetDrawThreads(const size_t threads) {
    assert(threads > 0);
    drawThreads_ = threads;

    chunkDrawThreads_.resize(drawThreads_);

    renderLayers_.clear(); // Opengl probably stores some internal memory addresses, so each layer must be recreated
    renderLayers_.resize(drawThreads_);

    assert(chunkDrawThreads_.size() == drawThreads_);
    assert(renderLayers_.size() == drawThreads_);
}


// ======================================================================


const SpriteUvCoordsT::mapped_type& render::Renderer::GetSpriteUvCoords(const SpriteUvCoordsT& map,
                                                                        const SpriteUvCoordsT::key_type key) noexcept {
    try {
        return const_cast<SpriteUvCoordsT&>(map)[key];
    }
    catch (std::exception&) {
        assert(false); // Should not throw
        std::terminate();
    }
}

void render::Renderer::SetPlayerPosition(const Position2<float>& player_position) noexcept {
    playerPosition_ = player_position;
}

void render::Renderer::GlRenderPlayerPosition(const GameTickT game_tick, const game::World& world) {
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
    const auto position_x = LossyCast<int>(playerPosition_.x);
    const auto position_y = LossyCast<int>(playerPosition_.y);


    const auto tile_offset = GetTileDrawOffset(position_x, position_y);

    const auto chunk_start  = GetChunkDrawStart(position_x, position_y);
    const auto chunk_amount = GetChunkDrawAmount(position_x, position_y);


    // Must be calculated after tile_offset, chunk_start and chunk_amount. Otherwise, zooming becomes jagged
    CalculateViewMatrix(playerPosition_.x, playerPosition_.y);
    GlUpdateTileProjectionMatrix();
    mvpManager_.CalculateMvpMatrix();
    mvpManager_.UpdateShaderMvp();

    std::size_t started_threads = 0; // Also is index for vector holding futures


    auto await_thread_completion = [&]() {
        for (int i = 0; i < started_threads; ++i) {
            chunkDrawThreads_[i].wait();
            auto& r_layer_tile = renderLayers_[i];

            GlPrepareEnd(r_layer_tile);
        }
    };

    std::mutex world_gen_mutex;

    for (int y = 0; y < chunk_amount.y; ++y) {

        // Wait for started threads to finish before starting new ones
        if (started_threads == drawThreads_) {
            await_thread_completion();
            started_threads = 0;
        }


        const auto chunk_y = chunk_start.y + y;

        auto& r_layer_tile = renderLayers_[started_threads];
        GlPrepareBegin(r_layer_tile);

        Position2<int> row_start{chunk_start.x, chunk_y};
        Position2<int> render_tile_offset{tile_offset.x, y * game::Chunk::kChunkWidth + tile_offset.y};

        chunkDrawThreads_[started_threads] = std::async(std::launch::async,
                                                        &Renderer::PrepareChunkRow,
                                                        this,
                                                        std::ref(r_layer_tile),
                                                        std::ref(world),
                                                        std::ref(world_gen_mutex),
                                                        row_start,
                                                        chunk_amount.x,
                                                        render_tile_offset,
                                                        game_tick);

        ++started_threads;
    }

    await_thread_completion();
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

    auto uv = GetSpriteUvCoords(sprite.internalId);
    ApplySpriteUvAdjustment(uv, sprite.GetCoords(set, 0));

    r_layer.PushBack({{

                          {SafeCast<float>(screen_pos.x), SafeCast<float>(screen_pos.y)},

                          {SafeCast<float>(screen_pos.x) + dimension.x * SafeCast<float>(tileWidth),
                           SafeCast<float>(screen_pos.y) + dimension.y * SafeCast<float>(tileWidth)}},
                      {uv.topLeft, uv.bottomRight}},
                     0.5f);
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

Position2<int32_t> render::Renderer::WorldCoordToBufferPos(const Position2<float>& player_pos,
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
    return {LossyCast<int32_t>(std::round(buffer_pos_x)), LossyCast<int32_t>(std::round(buffer_pos_y))};
}

void render::Renderer::CalculateViewMatrix(const float player_x, const float player_y) noexcept {
    const auto position_x = LossyCast<int>(player_x);
    const auto position_y = LossyCast<int>(player_y);

    // Negative moves window right and down

    // Decimal is used to shift the camera
    // Invert the movement to give the illusion of moving in the correct direction
    const float camera_offset_x = (player_x - SafeCast<float>(position_x)) * SafeCast<float>(tileWidth) * -1;
    const float camera_offset_y = (player_y - SafeCast<float>(position_y)) * SafeCast<float>(tileWidth) * -1;

    // Remaining pixel distance not covered by tiles and chunks are covered by the view matrix to center
    const auto tile_amount = GetTileDrawAmount();

    const auto& view_transform = mvpManager_.GetViewTransform();

    // Divide by 2 first to truncate decimals
    // A LossyCast is used as during startup, tile_amount is invalid since the mvp matrix is invalid
    view_transform->x = LossyCast<float>(GetWindowWidth() / 2 - (tile_amount.x / 2 * tileWidth)) + camera_offset_x;

    view_transform->y = LossyCast<float>(GetWindowHeight() / 2 - (tile_amount.y / 2 * tileWidth)) + camera_offset_y;

    mvpManager_.UpdateViewTransform();
}

Position2<int> render::Renderer::GetTileDrawAmount() const noexcept {
    const auto matrix        = glm::vec4(1, -1, 1, 1) / mvpManager_.GetMvpMatrix();
    const auto tile_amount_x = LossyCast<int>(matrix.x / LossyCast<double>(tileWidth) * 2) + 2;
    const auto tile_amount_y = LossyCast<int>(matrix.y / LossyCast<double>(tileWidth) * 2) + 2;

    return {tile_amount_x, tile_amount_y};
}

Position2<int> render::Renderer::GetTileDrawOffset(const int position_x, const int position_y) const noexcept {
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

Position2<int> render::Renderer::GetChunkDrawStart(const int position_x, const int position_y) const noexcept {
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

Position2<int> render::Renderer::GetChunkDrawAmount(const int position_x, const int position_y) const noexcept {
    const auto tile_offset = GetTileDrawOffset(position_x, position_y);
    const auto tile_amount = GetTileDrawAmount();

    int chunk_amount_x = (tile_amount.x - tile_offset.x) / game::Chunk::kChunkWidth;
    int chunk_amount_y = (tile_amount.y - tile_offset.y) / game::Chunk::kChunkWidth;

    chunk_amount_x += kPaddingChunks;
    chunk_amount_y += kPaddingChunks;

    return {chunk_amount_x, chunk_amount_y};
}

void render::Renderer::PrepareChunkRow(RendererLayer& r_layer,
                                       const game::World& world,
                                       std::mutex& world_gen_mutex,
                                       const Position2<int> row_start,
                                       const int chunk_span,
                                       const Position2<int> render_tile_offset,
                                       const GameTickT game_tick) const noexcept {

    for (int x = 0; x < chunk_span; ++x) {
        const auto chunk_x = x + row_start.x;

        const auto* chunk = world.GetChunkC({chunk_x, row_start.y});

        // Queue chunk for generation if it does not exist
        if (chunk == nullptr) {
            std::lock_guard<std::mutex> gen_guard{world_gen_mutex};
            world.QueueChunkGeneration({chunk_x, row_start.y});
            continue;
        }

        PrepareChunk(
            r_layer, *chunk, {x * game::Chunk::kChunkWidth + render_tile_offset.x, render_tile_offset.y}, game_tick);
    }
}

void render::Renderer::PrepareChunk(RendererLayer& r_layer,
                                    const game::Chunk& chunk,
                                    const Position2<int> render_tile_offset,
                                    const GameTickT game_tick) const noexcept {
    // Iterate through and load tiles of a chunk into layer for rendering
    for (ChunkTileCoordAxis tile_y = 0; tile_y < game::Chunk::kChunkWidth; ++tile_y) {
        const auto pixel_y = SafeCast<float>(render_tile_offset.y + tile_y) * SafeCast<float>(tileWidth);

        for (ChunkTileCoordAxis tile_x = 0; tile_x < game::Chunk::kChunkWidth; ++tile_x) {
            const auto pixel_x = SafeCast<float>(render_tile_offset.x + tile_x) * SafeCast<float>(tileWidth);

            PrepareTileLayers(r_layer, chunk, {tile_x, tile_y}, {pixel_x, pixel_y}, game_tick);
        }
    }

    PrepareOverlayLayers(r_layer, chunk, render_tile_offset);
}

void render::Renderer::PrepareTileLayers(RendererLayer& r_layer,
                                         const game::Chunk& chunk,
                                         const ChunkTileCoord ct_coord,
                                         const Position2<float>& pixel_pos,
                                         const GameTickT game_tick) const noexcept {
    for (int layer_index = 0; layer_index < game::kTileLayerCount; ++layer_index) {
        const auto& tile = chunk.GetCTile(ct_coord, static_cast<game::TileLayer>(layer_index));

        const auto* proto = tile.GetPrototype();
        if (proto == nullptr) // Layer not initialized
            continue;

        const auto* unique_data = tile.GetUniqueData();

        // Unique data can be nullptr for certain layers

        SpriteUvCoordsT::mapped_type uv;

        if (unique_data != nullptr) {
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

        if (tile.IsMultiTile())
            ApplyMultiTileUvAdjustment(uv, tile);

        const float pixel_z = 0.f + LossyCast<float>(0.01 * layer_index);

        // TODO do not draw those out of view
        r_layer.PushBack({{// top left of tile, 1 tile over and down
                           {pixel_pos.x, pixel_pos.y},
                           {pixel_pos.x + SafeCast<float>(tileWidth), pixel_pos.y + SafeCast<float>(tileWidth)}},
                          {uv.topLeft, uv.bottomRight}},
                         pixel_z);
    }
}

void render::Renderer::PrepareOverlayLayers(RendererLayer& r_layer,
                                            const game::Chunk& chunk,
                                            const Position2<int> render_tile_offset) const {

    for (int layer_index = 0; layer_index < game::kOverlayLayerCount; ++layer_index) {
        const auto& overlay_container = chunk.overlays[layer_index];

        for (const auto& overlay : overlay_container) {
            auto uv = GetSpriteUvCoords(overlay.sprite->internalId);

            ApplySpriteUvAdjustment(uv, overlay.sprite->GetCoords(overlay.spriteSet, 0));

            r_layer.PushBack(
                {{

                     {(render_tile_offset.x + overlay.position.x) * SafeCast<float>(tileWidth),

                      (render_tile_offset.y + overlay.position.y) * SafeCast<float>(tileWidth)},
                     {(render_tile_offset.x + overlay.position.x + overlay.size.x) * SafeCast<float>(tileWidth),

                      (render_tile_offset.y + overlay.position.y + overlay.size.y) * SafeCast<float>(tileWidth)}},
                 {uv.topLeft, uv.bottomRight}},
                overlay.position.z);
        }
    }
}

void render::Renderer::ApplySpriteUvAdjustment(UvPositionT& uv, const UvPositionT& uv_sub) noexcept {
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

void render::Renderer::ApplyMultiTileUvAdjustment(UvPositionT& uv, const game::ChunkTile& tile) noexcept {
    const auto& mt_data = tile.GetDimension();

    // Calculate the correct UV coordinates for multi-tile entities
    // Split the sprite into sections and stretch over multiple tiles if this entity is multi tile

    // Total length of the sprite, to be split among the different tiles
    const auto len_x = (uv.bottomRight.x - uv.topLeft.x) / SafeCast<float>(mt_data.x);
    const auto len_y = (uv.bottomRight.y - uv.topLeft.y) / SafeCast<float>(mt_data.y);

    const double x_multiplier = tile.GetOffsetX();
    const double y_multiplier = tile.GetOffsetY();

    // Opengl flips vertically, thus the y multiplier is inverted
    uv.bottomRight.x = uv.topLeft.x + LossyCast<UvPositionT::PositionT::ValueT>(len_x * (x_multiplier + 1));
    uv.bottomRight.y = uv.bottomRight.y - LossyCast<UvPositionT::PositionT::ValueT>(len_y * y_multiplier);

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
    GlDraw(r_layer.GetIndicesCount());
}

void render::Renderer::GlDraw(const uint64_t index_count) noexcept {
    DEBUG_OPENGL_CALL(glDrawElements(GL_TRIANGLES,
                                     SafeCast<GLsizei>(index_count),
                                     GL_UNSIGNED_INT,
                                     nullptr)); // Pointer not needed as buffer is already bound
}

void render::Renderer::GlUpdateTileProjectionMatrix() noexcept {
    const auto max_tile_width = SafeCast<float>(tileWidth * 2);

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
        if (tileProjectionMatrixOffset > SafeCast<float>(smallest_axis) / 2 - max_zoom_offset) {
            tileProjectionMatrixOffset = SafeCast<float>(smallest_axis) / 2 - max_zoom_offset;
        }
    }

    mvpManager_.GlSetProjectionMatrix(
        MvpManager::ToProjMatrix(windowWidth_, windowHeight_, tileProjectionMatrixOffset));
}
