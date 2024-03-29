// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include "render/tile_renderer.h"

#include <algorithm>
#include <cmath>
#include <future>
#include <glm/gtc/matrix_transform.hpp>

#include "core/execution_timer.h"
#include "game/world/world.h"
#include "proto/sprite.h"
#include "render/opengl/error.h"
#include "render/renderer_common.h"
#include "render/renderer_exception.h"
#include "render/spritemap_generator.h"

using namespace jactorio;

unsigned int render::TileRenderer::windowWidth_  = 0;
unsigned int render::TileRenderer::windowHeight_ = 0;

SpriteTexCoordIndexT render::TileRenderer::animationOffset_ = 0;

render::TileRenderer::TileRenderer(RendererCommon& common) : common_(&common) {}

void render::TileRenderer::Init() {
    // This does not need to change as everything is already prepared in world space
    const glm::mat4 model_matrix = translate(glm::mat4(1.f), glm::vec3(0, 0, 0));
    common_->mvpManager.GlSetModelMatrix(model_matrix);

    // Get window size
    GLint m_viewport[4];
    glGetIntegerv(GL_VIEWPORT, m_viewport);

    GlResizeWindow(m_viewport[2], m_viewport[3]);

    GlSetupTessellation();
}

void render::TileRenderer::InitTexture(const Spritemap& spritemap, const Texture& texture) noexcept {
    spritemap_ = &spritemap;
    texture_   = &texture;
}

void render::TileRenderer::InitShader() {
    assert(spritemap_ != nullptr);
    auto [terrain_tex_coords, terrain_tex_coord_size] = spritemap_->GenCurrentFrame();
    LOG_MESSAGE_F(info, "%d tex coords for tesselation renderer", terrain_tex_coord_size);

    GLint max_uniform_component;
    DEBUG_OPENGL_CALL(glGetIntegerv(GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS, &max_uniform_component));

    // Each tex coord is 4 floats
    // Pause-able animation is achieved by
    // doubling the tex coords, one set is static, another animated versions of entities
    const auto max_tex_coords = max_uniform_component / 4 / 2;
    if (terrain_tex_coord_size > max_tex_coords) {
        throw std::runtime_error(std::string("Max tex coords exceeded: ") + std::to_string(max_tex_coords));
    }

    shader_.Init({{"data/core/shaders/vs.vert", GL_VERTEX_SHADER},
                  {"data/core/shaders/fs.frag", GL_FRAGMENT_SHADER},
                  {"data/core/shaders/te.tese", GL_TESS_EVALUATION_SHADER}},
                 {{"__terrain_tex_coords_size", std::to_string(terrain_tex_coord_size * 2)}});
    shader_.Bind();
    common_->mvpManager.SetMvpUniformLocation(shader_.GetUniformLocation("u_model_view_projection_matrix"));

    // Texture will be bound to specified slot, tell this to shader
    DEBUG_OPENGL_CALL(glUniform1i(shader_.GetUniformLocation("u_texture"), kTextureSlot));

    // Upload the both sets of tex coords (animated and static)
    std::vector<TexCoord> all_tex_coords;
    all_tex_coords.reserve(terrain_tex_coord_size * 2);
    for (int j = 0; j < 2; ++j) {
        for (int i = 0; i < terrain_tex_coord_size; ++i) {
            all_tex_coords.push_back(terrain_tex_coords[i]);
        }
    }
    DEBUG_OPENGL_CALL(glUniform4fv(shader_.GetUniformLocation("u_tex_coords"), //
                                   terrain_tex_coord_size * 2,
                                   reinterpret_cast<const GLfloat*>(all_tex_coords.data())));
    animationOffset_ = terrain_tex_coord_size;
}

void render::TileRenderer::GlClear() noexcept {
    DEBUG_OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

void render::TileRenderer::GlBind() const noexcept {
    texture_->Bind(kTextureSlot);
    shader_.Bind();
}


unsigned render::TileRenderer::GetWindowWidth() noexcept {
    return windowWidth_;
}

unsigned render::TileRenderer::GetWindowHeight() noexcept {
    return windowHeight_;
}

void render::TileRenderer::GlResizeWindow(const unsigned int window_x, const unsigned int window_y) noexcept {
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


size_t render::TileRenderer::GetDrawThreads() const noexcept {
    return chunkDrawThreads_.size();
}

void render::TileRenderer::GlSetDrawThreads(const size_t threads) {
    assert(threads > 0);

    chunkDrawThreads_.resize(threads);

    renderLayers_.clear(); // Opengl probably stores some internal memory addresses, so each layer must be recreated
    renderLayers_.resize(threads);

    assert(chunkDrawThreads_.size() == threads);
    assert(renderLayers_.size() == threads);
}

SpriteTexCoordIndexT render::TileRenderer::GetAnimationOffset() noexcept {
    return animationOffset_;
}

float render::TileRenderer::GetZoom() const noexcept {
    return zoom_;
}

void render::TileRenderer::SetZoom(float zoom) noexcept {
    if (zoom < 0.f) {
        zoom = 0.f;
    }
    else if (zoom > 1.f) {
        zoom = 1.f;
    }

    zoom_ = zoom;
}

void render::TileRenderer::SetPlayerPosition(const Position2<float>& player_position) noexcept {
    playerPosition_ = player_position;
}

void render::TileRenderer::GlRender(const game::World& world) {
    assert(GetDrawThreads() > 0);
    assert(renderLayers_.size() == chunkDrawThreads_.size());

    EXECUTION_PROFILE_SCOPE(profiler, "World draw");

    UpdateAnimationTexCoords();

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
    common_->mvpManager.CalculateMvpMatrix();

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
    common_->mvpManager.CalculateMvpMatrix();
    common_->mvpManager.UpdateShaderMvp();

    // ======================================================================


    std::mutex world_gen_mutex;

    // Thread order: (Helps reduce stalls as some threads are always active)
    // - Start all
    // - Wait for thread 1, draw, continue thread 1
    // - Wait for thread 2, draw, continue thread 2
    // ... Repeat until done

    const auto needed_threads   = chunk_amount.y;
    const auto threads_to_start = std::min(LossyCast<int>(GetDrawThreads()), needed_threads);

    // Start all threads
    int i = 0;
    for (; i < threads_to_start; ++i) {
        auto& r_layer = renderLayers_[i];
        GlPrepareBegin(r_layer);


        Position2 row_start{chunk_start.x, chunk_start.y + i};
        Position2 render_tile_offset{tile_offset.x, i * game::Chunk::kChunkWidth + tile_offset.y};

        chunkDrawThreads_[i] = std::async(std::launch::async,
                                          &TileRenderer::PrepareChunkRow,
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
                                                 &TileRenderer::PrepareChunkRow,
                                                 this,
                                                 std::ref(r_layer),
                                                 std::ref(world),
                                                 std::ref(world_gen_mutex),
                                                 row_start,
                                                 chunk_amount.x,
                                                 render_tile_offset);

        thread_n++;
        if (thread_n >= GetDrawThreads())
            thread_n = 0;
    }

    // Continue off from prior loop, but only waiting for threads and drawing
    for (int j = 0; j < threads_to_start; ++j) {
        chunkDrawThreads_[thread_n].wait();

        auto& r_layer = renderLayers_[thread_n];
        GlPrepareEnd(r_layer);

        thread_n++;
        if (thread_n >= GetDrawThreads())
            thread_n = 0;
    }
}

void render::TileRenderer::GlPrepareBegin() {
    GlPrepareBegin(renderLayers_[0]);
}
void render::TileRenderer::GlPrepareEnd() {
    GlPrepareEnd(renderLayers_[0]);
}

void render::TileRenderer::PrepareSprite(const WorldCoord& coord,
                                         const SpriteTexCoordIndexT tex_coord_id,
                                         const Dimension& dimension) {
    auto& r_layer = renderLayers_[0];

    const auto screen_pos = WorldCoordToBufferPos(playerPosition_, coord);

    for (DimensionAxis y = 0; y < dimension.y; ++y) {
        const auto id_offset_y = y * dimension.x;
        for (DimensionAxis x = 0; x < dimension.x; ++x) {
            const auto id = SafeCast<SpriteTexCoordIndexT>(tex_coord_id + id_offset_y + x);
            r_layer.PushBack({{SafeCast<uint16_t>(screen_pos.x + x), SafeCast<uint16_t>(screen_pos.y + y), 0}, id});
        }
    }
}

WorldCoord render::TileRenderer::ScreenPosToWorldCoord(const Position2<float>& player_pos,
                                                       const Position2<int32_t>& screen_pos) const {
    const auto truncated_player_pos_x = SafeCast<float>(LossyCast<int>(player_pos.x));
    const auto truncated_player_pos_y = SafeCast<float>(LossyCast<int>(player_pos.y));

    const auto norm_screen_pos_x = 2 * (SafeCast<float>(screen_pos.x) / SafeCast<float>(GetWindowWidth())) - 1;
    const auto norm_screen_pos_y = 2 * (SafeCast<float>(screen_pos.y) / SafeCast<float>(GetWindowHeight())) - 1;

    const glm::vec4 adjusted_screen_pos =
        common_->mvpManager.GetMvpMatrix() / glm::vec4(norm_screen_pos_x, norm_screen_pos_y, 1, 1);

    // Normalize window center between -1 and 1, since is window center, simplifies to 0
    const auto screen_center_pos = common_->mvpManager.GetMvpMatrix() / glm::vec4(0, 0, 1.f, 1.f);

    // WorldCoord sign direction
    // Pixels from top left of tile player standing on
    const auto pixels_from_tl_x = adjusted_screen_pos.x - screen_center_pos.x +
        SafeCast<float>(tileWidth) * (player_pos.x - truncated_player_pos_x);

    // Opengl Y increases going up, WorldCoord increases going down
    const auto pixels_from_tl_y = screen_center_pos.y - adjusted_screen_pos.y +
        SafeCast<float>(tileWidth) * (player_pos.y - truncated_player_pos_y);

    auto tile_x = truncated_player_pos_x + pixels_from_tl_x / LossyCast<float>(tileWidth);
    auto tile_y = truncated_player_pos_y + pixels_from_tl_y / LossyCast<float>(tileWidth);

    // Subtract extra tile if negative because no tile exists at -0, -0
    if (tile_x < 0)
        tile_x -= 1;
    if (tile_y < 0)
        tile_y -= 1;

    return {LossyCast<WorldCoordAxis>(tile_x), LossyCast<WorldCoordAxis>(tile_y)};
}

WorldCoord render::TileRenderer::ScreenPosToWorldCoord(const Position2<int32_t>& screen_pos) const {
    return ScreenPosToWorldCoord(playerPosition_, screen_pos);
}

Position2<int16_t> render::TileRenderer::WorldCoordToBufferPos(const Position2<float>& player_pos,
                                                               const WorldCoord& coord) const {
    const auto truncated_player_pos_x = SafeCast<float>(LossyCast<int>(player_pos.x));
    const auto truncated_player_pos_y = SafeCast<float>(LossyCast<int>(player_pos.y));

    // From top left, without matrix adjustments (WorldCoord sign direction)
    const auto pixels_from_tl_x = (coord.x - truncated_player_pos_x) * tileWidth;
    const auto pixels_from_tl_y = (coord.y - truncated_player_pos_y) * tileWidth;

    // From top left of screen (Top left 0, 0)
    const auto buffer_center_pos = common_->mvpManager.GetMvpMatrix() / glm::vec4(0, 0, 1.f, 1.f);
    const auto buffer_pos_x =
        pixels_from_tl_x + buffer_center_pos.x - SafeCast<float>(tileWidth) * (player_pos.x - truncated_player_pos_x);
    const auto buffer_pos_y =
        pixels_from_tl_y + buffer_center_pos.y - SafeCast<float>(tileWidth) * (player_pos.y - truncated_player_pos_y);

    // Sometimes has float 8.99999, which is wrongly truncated to 8, thus must round first
    return {LossyCast<int16_t>(std::round(buffer_pos_x)), LossyCast<int16_t>(std::round(buffer_pos_y))};
}

Position2<int16_t> render::TileRenderer::WorldCoordToBufferPos(const WorldCoord& coord) const {
    return WorldCoordToBufferPos(playerPosition_, coord);
}

// ======================================================================

void render::TileRenderer::GlSetupTessellation() {
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

void render::TileRenderer::GlDraw(const uint64_t count) noexcept {
    DEBUG_OPENGL_CALL(glDrawArrays(GL_PATCHES, 0, SafeCast<GLsizei>(count)));
}

void render::TileRenderer::CalculateViewMatrix(const Position2<int> i_player) noexcept {
    // Decimal is used to shift the camera
    // Invert the movement to give the illusion of moving in the correct direction
    const auto camera_offset =
        Position2{(playerPosition_.x - SafeCast<float>(i_player.x)) * SafeCast<float>(tileWidth) * -1,
                  (playerPosition_.y - SafeCast<float>(i_player.y)) * SafeCast<float>(tileWidth) * -1};

    // Remaining pixel distance not covered by tiles and chunks are covered by the view matrix to center
    const auto tile_amount = GetTileDrawAmount();

    const auto& view_transform = common_->mvpManager.GetViewTransform();

    // Operations inside lossy cast centers of player position at the center of screen
    // Use truncating division, such that for odd # of tiles (e.g 5), the top left of third tile is at center of screen
    // instead of the center of the tile
    // A LossyCast is used as during startup, tile_amount is invalid since the mvp matrix is invalid
    view_transform->x =
        LossyCast<float>(windowWidth_) / 2 - LossyCast<float>(tile_amount.x / 2 * tileWidth) + camera_offset.x;
    view_transform->y =
        LossyCast<float>(windowHeight_) / 2 - LossyCast<float>(tile_amount.y / 2 * tileWidth) + camera_offset.y;

    common_->mvpManager.UpdateViewTransform();
}

Position2<int> render::TileRenderer::GetTileDrawAmount() const noexcept {
    // Units are in pixels
    const auto bottom_right = glm::vec4(1, -1, 1, 1) / common_->mvpManager.GetMvpMatrix();

    // Double because 0 is at the center
    // + 2 for some extra tiles
    return Position2{LossyCast<int>(bottom_right.x / LossyCast<double>(tileWidth) * 2) + 2,
                     LossyCast<int>(bottom_right.y / LossyCast<double>(tileWidth) * 2) + 2};
}

void render::TileRenderer::PrepareChunkRow(TRenderBuffer& r_layer,
                                           const game::World& world,
                                           std::mutex& world_gen_mutex,
                                           Position2<int> row_start,
                                           const int chunk_span,
                                           Position2<int> render_tile_offset) const noexcept {
    auto [tex_ids, readable_chunks] = world.GetChunkTexCoordIds(row_start);

    if (readable_chunks < chunk_span) {
        std::lock_guard gen_guard{world_gen_mutex};

        for (int x = readable_chunks; x < chunk_span; ++x) {
            const auto chunk_x = x + row_start.x;
            world.QueueChunkGeneration({chunk_x, row_start.y});
        }
    }

    // If the leftmost chunk is not readable, try other ones in row
    // avoids annoying black screens when moving left
    if (readable_chunks == 0) {
        int i = 1; // Not 0, since we already know current chunk has no readable chunks
        while (readable_chunks == 0 && i < chunk_span) {
            std::tie(tex_ids, readable_chunks) = world.GetChunkTexCoordIds({row_start.x + i, row_start.y});
            render_tile_offset.x += game::Chunk::kChunkWidth;
            ++i;
        }
        row_start.x += i - 1; // i gets increment extra time at end of while
    }

    // Despite being readable, the chunk may not be generated
    auto tex_ids_2 = tex_ids; // Cannot move the original pointer
    for (int i = 0; i < readable_chunks; ++i) {
        if (tex_ids_2[0] == 0) { // First tile, bottom layer of chunk
            std::lock_guard gen_guard{world_gen_mutex};
            world.QueueChunkGeneration({row_start.x + i, row_start.y});
        }
        tex_ids_2 += game::Chunk::kChunkArea * game::kTileLayerCount;
    }

    // Allocate for the maximum possible tile layers to render
    const auto required_r_layer_capacity =
        SafeCast<uint32_t>(chunk_span * game::Chunk::kChunkArea * game::kTileLayerCount);
    if (required_r_layer_capacity > r_layer.Capacity()) {
        r_layer.Reserve(required_r_layer_capacity);
        return;
    }

    // Prevents PrepareChunk from having a pixel x/y < 0
    auto skip_tiles_top = -render_tile_offset.y / SafeCast<int>(tileWidth);
    if (skip_tiles_top < 0) { // No tiles to skip
        skip_tiles_top = 0;
    }

    // Tiles needed to reach the window height
    auto tiles_prepare_y = (windowHeight_ - render_tile_offset.y * SafeCast<int>(tileWidth)) / SafeCast<int>(tileWidth);
    if (tiles_prepare_y > game::Chunk::kChunkWidth) {
        tiles_prepare_y = game::Chunk::kChunkWidth;
    }

    for (int x = 0; x < std::min(readable_chunks, chunk_span); ++x) {
        const auto chunk_render_tile_offset_x = x * game::Chunk::kChunkWidth + render_tile_offset.x;

        auto skip_tiles_left = -chunk_render_tile_offset_x / SafeCast<int>(tileWidth);
        if (skip_tiles_left < 0) {
            skip_tiles_left = 0;
        }

        // Tiles needed to reach window width
        auto tiles_prepare_x =
            (windowWidth_ - render_tile_offset.x * SafeCast<int>(tileWidth)) / SafeCast<int>(tileWidth);
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

FORCEINLINE void render::TileRenderer::PrepareChunk(TRenderBuffer& r_layer,
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
                    {{SafeCast<uint16_t>(pixel_x), SafeCast<uint16_t>(pixel_y), 0}, tex_coord_ids[1]});
            }

            // 3
            if (tex_coord_ids[2] != 0) {
                r_layer.UncheckedPushBack(
                    {{SafeCast<uint16_t>(pixel_x), SafeCast<uint16_t>(pixel_y), 0}, tex_coord_ids[2]});
            }

            tex_coord_ids += 3;
        }

        tex_coord_ids += (game::Chunk::kChunkWidth - tile_end.x) * game::kTileLayerCount;
    }
}

FORCEINLINE void render::TileRenderer::PrepareOverlayLayers(TRenderBuffer& r_layer,
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

void render::TileRenderer::UpdateAnimationTexCoords() const noexcept {
    auto [tex_coords, size] = spritemap_->GenNextFrame();

    // Update only the animated set of tex coord
    static_assert(std::is_same_v<GLfloat, TexCoord::PositionT::ValueT>);
    DEBUG_OPENGL_CALL(glUniform4fv(shader_.GetUniformLocation("u_tex_coords"), //
                                   size,
                                   reinterpret_cast<const GLfloat*>(tex_coords)));
}

void render::TileRenderer::GlPrepareBegin(TRenderBuffer& r_layer) {
    r_layer.GlBindBuffers();
    r_layer.GlWriteBegin();
}

void render::TileRenderer::GlPrepareEnd(TRenderBuffer& r_layer) {
    r_layer.GlBindBuffers();
    r_layer.GlWriteEnd();
    r_layer.GlHandleBufferResize();
    GlDraw(r_layer.Size());
}

void render::TileRenderer::GlUpdateTileProjectionMatrix(const float zoom) noexcept {
    // Must subtract some because zooming to EXACTLY half leaves no pixels remaining, making everything invisible
    const auto max_pixel_zoom     = std::min(windowWidth_, windowHeight_) / 2.f - 1;
    constexpr auto min_pixel_zoom = 1.f; // MvpManager requests pixel zoom never 0

    auto pixel_zoom = zoom * max_pixel_zoom;

    if (pixel_zoom < min_pixel_zoom) {
        pixel_zoom = min_pixel_zoom;
    }
    else if (pixel_zoom > max_pixel_zoom) {
        pixel_zoom = max_pixel_zoom;
    }

    assert(pixel_zoom > 0);
    common_->mvpManager.GlSetProjectionMatrix(MvpManager::ToProjMatrix(windowWidth_, windowHeight_, pixel_zoom));
}
