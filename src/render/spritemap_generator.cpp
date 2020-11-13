// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/spritemap_generator.h"

#include <algorithm>
#include <map>

#include "core/convert.h"
#include "core/logger.h"
#include "core/resource_guard.h"

#include "data/prototype_manager.h"

using namespace jactorio;

void render::RendererSprites::ClearSpritemaps() {
    for (auto& map : textures_) {
        delete map.second;
    }
    textures_.clear();
    // The pointer which this contains is already cleared by spritemaps
    spritemapDatas_.clear();
}

void render::RendererSprites::GInitializeSpritemap(const data::PrototypeManager& data_manager,
                                                   proto::Sprite::SpriteGroup group,
                                                   const bool invert_sprites) {
    const auto spritemap_data = CreateSpritemap(data_manager, group, invert_sprites);

    textures_[static_cast<int>(group)] =
        new Texture(spritemap_data.spriteBuffer, spritemap_data.width, spritemap_data.height);
    spritemapDatas_[static_cast<int>(group)] = spritemap_data;
}

render::RendererSprites::SpritemapData render::RendererSprites::CreateSpritemap(
    const data::PrototypeManager& data_manager, proto::Sprite::SpriteGroup group, const bool invert_sprites) const {

    auto sprites = data_manager.DataRawGetAll<const proto::Sprite>(proto::Category::sprite);

    // Filter to group only
    sprites.erase(
        std::remove_if(
            sprites.begin(),
            sprites.end(),
            [group](auto* ptr) {
                // Return false to NOT remove
                // Category of none is never removed
                if (ptr->group.empty()) {
                    LOG_MESSAGE_F(
                        warning,
                        "Sprite prototype '%s' does not have a category, and will be added to all sprite groups."
                        " Consider giving it a category",
                        ptr->name.c_str());
                    return false;
                }

                return !ptr->IsInGroup(group);
            }),
        sprites.end());

    return GenSpritemap(sprites, invert_sprites);
}


const render::RendererSprites::SpritemapData& render::RendererSprites::GetSpritemap(proto::Sprite::SpriteGroup group) {
    return spritemapDatas_[static_cast<int>(group)];
}

const render::Texture* render::RendererSprites::GetTexture(proto::Sprite::SpriteGroup group) {
    return textures_[static_cast<int>(group)];
}


// ======================================================================
// Spritemap generation functions

render::RendererSprites::SpritemapData render::RendererSprites::GenSpritemap(
    const std::vector<const proto::Sprite*>& sprites, const bool invert_sprites) const {

    LOG_MESSAGE_F(
        info, "Generating spritemap with %lld sprites, %s", sprites.size(), invert_sprites ? "Inverted" : "Upright");

    // At least 1 sprite is needed
    if (sprites.empty()) {
        auto data   = SpritemapData();
        data.width  = 0;
        data.height = 0;
        return data;
    }

    // ======================================================================

    SpritemapDimensionT spritemap_x;
    SpritemapDimensionT spritemap_y;

    std::vector<GeneratorNode*> node_buffer;
    core::CapturingGuard<void()> guard([&]() {
        for (auto* node : node_buffer) {
            delete node;
        }
    });

    {
        auto sorted_sprites = sprites;
        SortInputSprites(sorted_sprites);


        // Generate "nodes" of each sprite and its neighbors
        constexpr SpritemapDimensionT max_width = 99999;

        // sorted_sprites is empty after generating nodes
        spritemap_y = GetSpriteHeight(sorted_sprites[0]);


        GeneratorNode parent_node{nullptr};
        GenerateSpritemapNodes(sorted_sprites, node_buffer, parent_node, max_width, spritemap_y);


        assert(parent_node.above != nullptr);
        spritemap_x = GetSpritemapWidth(*parent_node.above);
    }


    // ======================================================================
    // Convert nodes into image output


    const auto spritemap_buffer_size = core::SafeCast<uint64_t>(spritemap_x) * spritemap_y * 4;

    std::shared_ptr<Texture::SpriteBufferT> spritemap_buffer(new Texture::SpriteBufferT[spritemap_buffer_size],
                                                             [](const Texture::SpriteBufferT* p) { delete[] p; });

    SpriteUvCoordsT image_positions;

    GenerateSpritemapOutput(spritemap_buffer, spritemap_x, *node_buffer[0], invert_sprites, image_positions, 0, 0);


    // Normalize positions based on image size to value between 0 - 1
    for (auto& image : image_positions) {
        auto& position = image.second;

        position.topLeft.x /= core::SafeCast<float>(spritemap_x);
        position.topLeft.y /= core::SafeCast<float>(spritemap_y);

        position.bottomRight.x /= core::SafeCast<float>(spritemap_x);
        position.bottomRight.y /= core::SafeCast<float>(spritemap_y);
    }


    SpritemapData spritemap_data;
    spritemap_data.spriteBuffer = std::move(spritemap_buffer);
    spritemap_data.width        = spritemap_x;
    spritemap_data.height       = spritemap_y;

    spritemap_data.spritePositions = image_positions;

    return spritemap_data;
}


proto::Sprite::SpriteDimension render::RendererSprites::GetSpriteWidth(const proto::Sprite* sprite) {
    return sprite->GetWidth() + 2 * sprite_border;
}

proto::Sprite::SpriteDimension render::RendererSprites::GetSpriteHeight(const proto::Sprite* sprite) {
    return sprite->GetHeight() + 2 * sprite_border;
}

void render::RendererSprites::SortInputSprites(std::vector<const proto::Sprite*>& sprites) {
    std::sort(sprites.begin(), sprites.end(), [](auto* first, auto* second) {
        const auto first_h  = GetSpriteHeight(first);
        const auto second_h = GetSpriteHeight(second);

        // Sort in descending order, by height then by width
        if (first_h == second_h)
            return GetSpriteWidth(first) > GetSpriteWidth(second);

        return first_h > second_h;
    });
}

void render::RendererSprites::GenerateSpritemapNodes(std::vector<const proto::Sprite*>& sprites,
                                                     std::vector<GeneratorNode*>& node_buffer,
                                                     GeneratorNode& parent_node,
                                                     SpritemapDimensionT max_width,
                                                     const SpritemapDimensionT max_height) {
    GeneratorNode* current_node = &parent_node;

    while (!sprites.empty()) {
        bool found_sprite           = false;
        const proto::Sprite* sprite = nullptr;

        for (decltype(sprites.size()) i = 0; i < sprites.size(); ++i) {
            const auto* i_sprite = sprites[i];

            if (GetSpriteWidth(i_sprite) <= max_width && GetSpriteHeight(i_sprite) <= max_height) {
                sprite = i_sprite;
                // Erase since it was used
                sprites.erase(sprites.begin() + i);

                found_sprite = true;
                break;
            }
        }
        if (!found_sprite)
            return;


        max_width -= GetSpriteWidth(sprite);
        const auto remaining_height = max_height - GetSpriteHeight(sprite);


        auto* node = node_buffer.emplace_back(new GeneratorNode(sprite));

        // The only time where above is set is the first iteration above parent node
        if (current_node == &parent_node)
            current_node->above = node;
        else
            current_node->right = node;


        // Try to create node above
        if (remaining_height != 0) {
            GenerateSpritemapNodes(sprites, node_buffer, *node, GetSpriteWidth(sprite), remaining_height);
        }

        current_node = node;
    }
}

render::RendererSprites::SpritemapDimensionT render::RendererSprites::GetSpritemapWidth(GeneratorNode& base_node) {
    // Added to width of each sprite
    SpritemapDimensionT width   = 0;
    GeneratorNode* current_node = &base_node;

    while (true) {
        width += GetSpriteWidth(current_node->sprite);

        if (current_node->right != nullptr)
            current_node = current_node->right;
        else
            break;
    }

    return width;
}

// ======================================================================

uint64_t GetSpriteIndex(const bool invert_sprites,
                        const proto::Sprite::SpriteDimension sprite_width,
                        const proto::Sprite::SpriteDimension sprite_height,
                        const proto::Sprite::SpriteDimension sprite_x,
                        const proto::Sprite::SpriteDimension sprite_y,
                        const uint8_t color_offset) {
    if (invert_sprites)
        return (core::SafeCast<uint64_t>(sprite_width) * (sprite_height - 1 - sprite_y) + sprite_x) * 4 + color_offset;

    return (core::SafeCast<uint64_t>(sprite_width) * sprite_y + sprite_x) * 4 + color_offset;
}

uint64_t GetBufferIndex(const render::RendererSprites::SpritemapDimensionT spritemap_width,
                        const render::RendererSprites::SpritemapDimensionT spritemap_x_offset,
                        const render::RendererSprites::SpritemapDimensionT spritemap_y_offset,
                        const proto::Sprite::SpriteDimension pixel_x,
                        const proto::Sprite::SpriteDimension pixel_y,
                        const uint8_t color_offset) {
    uint64_t buffer_index = spritemap_width * (pixel_y + spritemap_y_offset);
    buffer_index += pixel_x + spritemap_x_offset;
    buffer_index = buffer_index * 4 + color_offset;
    return buffer_index;
}

void render::RendererSprites::SetSpritemapPixel(std::shared_ptr<Texture::SpriteBufferT>& spritemap_buffer,
                                                const SpritemapDimensionT spritemap_width,
                                                const bool invert_sprites,
                                                const SpritemapDimensionT spritemap_x_offset,
                                                const SpritemapDimensionT spritemap_y_offset,
                                                const unsigned char* sprite_data,
                                                const proto::Sprite::SpriteDimension sprite_width,
                                                const proto::Sprite::SpriteDimension sprite_height,
                                                const unsigned pixel_x,
                                                const unsigned pixel_y) {
    for (uint8_t color_offset = 0; color_offset < 4; ++color_offset) {
        const auto sprite_index =
            GetSpriteIndex(invert_sprites, sprite_width, sprite_height, pixel_x, pixel_y, color_offset);

        const uint64_t buffer_index =
            GetBufferIndex(spritemap_width, spritemap_x_offset, spritemap_y_offset, pixel_x, pixel_y, color_offset);

        spritemap_buffer.get()[buffer_index] = sprite_data[sprite_index];
    }
}

void render::RendererSprites::GenerateSpritemapOutput(std::shared_ptr<Texture::SpriteBufferT>& spritemap_buffer,
                                                      const SpritemapDimensionT spritemap_width,
                                                      GeneratorNode& base_node,
                                                      const bool invert_sprites,
                                                      SpriteUvCoordsT& image_positions,
                                                      SpritemapDimensionT x_offset,
                                                      const SpritemapDimensionT y_offset) {
    auto adjusted_x_offset = x_offset;
    auto adjusted_y_offset = y_offset;

    adjusted_y_offset += sprite_border;

    GeneratorNode* current_node = &base_node;

    while (true) {
        adjusted_x_offset += sprite_border;

        const auto* sprite               = current_node->sprite;
        const unsigned char* sprite_data = sprite->GetSpritePtr();

        const auto sprite_width  = sprite->GetWidth();
        const auto sprite_height = sprite->GetHeight();

        // Extend edge pixels into sprite_border
        // NOTE: This does not handle the 4 corner pixels
        for (int i = 0; i < sprite_border; ++i) {
            // Top and bottom
            for (unsigned int pixel_x = 0; pixel_x < sprite_width; ++pixel_x) {
                SetSpritemapPixel(spritemap_buffer,
                                  spritemap_width,
                                  invert_sprites,
                                  adjusted_x_offset,
                                  y_offset,
                                  sprite_data,
                                  sprite_width,
                                  sprite_height,
                                  pixel_x,
                                  i);

                SetSpritemapPixel(spritemap_buffer,
                                  spritemap_width,
                                  invert_sprites,
                                  adjusted_x_offset,
                                  adjusted_y_offset + i + 1,
                                  sprite_data,
                                  sprite_width,
                                  sprite_height,
                                  pixel_x,
                                  sprite_height - (i + 1));
            }

            // Left and right
            for (unsigned int pixel_y = 0; pixel_y < sprite_height; ++pixel_y) {
                SetSpritemapPixel(spritemap_buffer,
                                  spritemap_width,
                                  invert_sprites,
                                  x_offset,
                                  adjusted_y_offset,
                                  sprite_data,
                                  sprite_width,
                                  sprite_height,
                                  i,
                                  pixel_y);

                SetSpritemapPixel(spritemap_buffer,
                                  spritemap_width,
                                  invert_sprites,
                                  adjusted_x_offset + i + 1,
                                  adjusted_y_offset,
                                  sprite_data,
                                  sprite_width,
                                  sprite_height,
                                  sprite_width - (i + 1),
                                  pixel_y);
            }
        }

        // Copy data onto spritemap
        for (unsigned int pixel_y = 0; pixel_y < sprite_height; ++pixel_y) {
            for (unsigned int pixel_x = 0; pixel_x < sprite_width; ++pixel_x) {
                SetSpritemapPixel(spritemap_buffer,
                                  spritemap_width,
                                  invert_sprites,
                                  adjusted_x_offset,
                                  adjusted_y_offset,
                                  sprite_data,
                                  sprite_width,
                                  sprite_height,
                                  pixel_x,
                                  pixel_y);
            }
        }

        // Keep track of image positions within the spritemap
        {
            auto& image_position = image_positions[sprite->internalId];

            image_position.topLeft =
                core::Position2{core::SafeCast<float>(adjusted_x_offset), core::SafeCast<float>(adjusted_y_offset)};

            image_position.bottomRight = core::Position2{core::SafeCast<float>(adjusted_x_offset + sprite_width),
                                                         core::SafeCast<float>(adjusted_y_offset + sprite_height)};
        }

        if (current_node->above != nullptr) {
            GenerateSpritemapOutput(spritemap_buffer,
                                    spritemap_width,
                                    *current_node->above,
                                    invert_sprites,
                                    image_positions,
                                    x_offset,
                                    adjusted_y_offset + sprite_height + sprite_border);
        }

        adjusted_x_offset += sprite_width;
        adjusted_x_offset += sprite_border;

        // For the next sprite, x offset is with adjustments to line up
        x_offset = adjusted_x_offset;


        if (current_node->right != nullptr)
            current_node = current_node->right;
        else
            break;
    }
}
