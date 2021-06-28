// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/spritemap_generator.h"

#include <algorithm>
#include <map>

#include "core/convert.h"
#include "core/logger.h"
#include "core/resource_guard.h"

#include "data/prototype_manager.h"

using namespace jactorio;

void render::RendererSprites::Clear() {
    for (auto& map : textures_) {
        delete map.second;
    }
    textures_.clear();
    // The pointer which this contains is already cleared by spritemaps
    spritemapDatas_.clear();
}

void render::RendererSprites::GlInitializeSpritemap(const data::PrototypeManager& proto,
                                                    proto::Sprite::SpriteGroup group,
                                                    const bool invert_sprites) {
    const auto spritemap_data = CreateSpritemap(proto, group, invert_sprites);

    textures_[static_cast<int>(group)] =
        new Texture(spritemap_data.spriteBuffer, spritemap_data.width, spritemap_data.height);
    spritemapDatas_[static_cast<int>(group)] = spritemap_data;
}

render::RendererSprites::SpritemapData render::RendererSprites::CreateSpritemap(const data::PrototypeManager& proto,
                                                                                proto::Sprite::SpriteGroup group,
                                                                                const bool invert_sprites) {

    auto sprites = proto.GetAll<proto::Sprite>();

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

render::RendererSprites::SpritemapData render::RendererSprites::GenSpritemap(std::vector<proto::Sprite*> sprites,
                                                                             const bool invert_sprites) {

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
    CapturingGuard<void()> guard([&]() {
        for (auto* node : node_buffer) {
            delete node;
        }
    });

    SpriteTexCoords tex_coords;
    tex_coords.reserve(sprites.size() + 1);
    tex_coords.push_back({{}, {}}); // Index 0 is unused

    {
        SortInputSprites(sprites);

        spritemap_y = TotalSpriteHeight(sprites[0]);

        GeneratorNode parent_node{nullptr};
        GenerateSpritemapNodes(sprites, node_buffer, parent_node, kMaxSpritemapWidth, spritemap_y);
        assert(sprites.empty());


        assert(parent_node.above != nullptr);
        spritemap_x = GetSpritemapWidth(*parent_node.above);
    }


    // ======================================================================
    // Convert nodes into image output


    const auto spritemap_buffer_size = SafeCast<uint64_t>(spritemap_x) * spritemap_y * 4;
    std::shared_ptr<Texture::SpriteBufferT> spritemap_buffer(new Texture::SpriteBufferT[spritemap_buffer_size],
                                                             [](const Texture::SpriteBufferT* p) { delete[] p; });

    SpriteTexCoordIndexT tex_coord_id = 1;
    GeneratorContext context{spritemap_buffer.get(), spritemap_x, tex_coords, invert_sprites, &tex_coord_id};

    GenerateSpritemapOutput(context, *node_buffer[0], {0, 0});


    // Normalize positions based on image size to value between 0 - 1
    for (auto& coord : tex_coords) {
        coord.topLeft.x /= SafeCast<float>(spritemap_x);
        coord.topLeft.y /= SafeCast<float>(spritemap_y);

        coord.bottomRight.x /= SafeCast<float>(spritemap_x);
        coord.bottomRight.y /= SafeCast<float>(spritemap_y);
    }


    SpritemapData spritemap_data;
    spritemap_data.spriteBuffer = std::move(spritemap_buffer);
    spritemap_data.width        = spritemap_x;
    spritemap_data.height       = spritemap_y;

    spritemap_data.spritePositions = std::move(tex_coords);

    return spritemap_data;
}


proto::Sprite::SpriteDimension render::RendererSprites::TotalSpriteWidth(const proto::Sprite* sprite) noexcept {
    return sprite->GetImage().width + 2 * kSpriteBorder;
}

proto::Sprite::SpriteDimension render::RendererSprites::TotalSpriteHeight(const proto::Sprite* sprite) noexcept {
    return sprite->GetImage().height + 2 * kSpriteBorder;
}

void render::RendererSprites::SortInputSprites(std::vector<proto::Sprite*>& sprites) {
    std::sort(sprites.begin(), sprites.end(), [](auto* first, auto* second) {
        const auto first_h  = TotalSpriteHeight(first);
        const auto second_h = TotalSpriteHeight(second);

        // Sort in descending order, by height then by width
        if (first_h == second_h)
            return TotalSpriteWidth(first) > TotalSpriteWidth(second);

        return first_h > second_h;
    });
}

void render::RendererSprites::GenerateSpritemapNodes(std::vector<proto::Sprite*>& sprites,
                                                     std::vector<GeneratorNode*>& node_buffer,
                                                     GeneratorNode& parent_node,
                                                     SpritemapDimensionT max_width,
                                                     const SpritemapDimensionT max_height) {
    GeneratorNode* current_node = &parent_node;

    while (!sprites.empty()) {
        bool found_sprite     = false;
        proto::Sprite* sprite = nullptr;

        for (std::size_t i = 0; i < sprites.size(); ++i) {
            auto* i_sprite = sprites[i];

            if (TotalSpriteWidth(i_sprite) <= max_width && TotalSpriteHeight(i_sprite) <= max_height) {
                sprite = i_sprite;
                // Erase since it was used
                sprites.erase(sprites.begin() + i);

                found_sprite = true;
                break;
            }
        }
        if (!found_sprite)
            return;


        max_width -= TotalSpriteWidth(sprite);
        const auto remaining_height = max_height - TotalSpriteHeight(sprite);


        auto* node = node_buffer.emplace_back(new GeneratorNode(sprite));

        // The only time where above is set is the first iteration above parent node
        if (current_node == &parent_node)
            current_node->above = node;
        else
            current_node->right = node;


        // Try to create node above
        if (remaining_height != 0) {
            GenerateSpritemapNodes(sprites, node_buffer, *node, TotalSpriteWidth(sprite), remaining_height);
        }

        current_node = node;
    }
}

render::RendererSprites::SpritemapDimensionT render::RendererSprites::GetSpritemapWidth(GeneratorNode& base_node) {
    // Added to width of each sprite
    SpritemapDimensionT width   = 0;
    GeneratorNode* current_node = &base_node;

    while (true) {
        width += TotalSpriteWidth(current_node->sprite);

        if (current_node->right != nullptr)
            current_node = current_node->right;
        else
            break;
    }

    return width;
}

void render::RendererSprites::SetSpritemapPixel(GeneratorContext& context,
                                                Position2<SpritemapDimensionT> offset,
                                                const proto::ImageContainer& image,
                                                Position2<unsigned> pixel_pos) {
    assert(image.buffer != nullptr);

    /// Calculates index into image buffer
    auto calc_image_index = [&](const uint8_t color_offset) {
        auto base = SafeCast<uint64_t>(image.width);

        if (context.invertSprites) {
            base *= image.height - 1 - pixel_pos.y;
        }
        else {
            base *= pixel_pos.y;
        }
        base += pixel_pos.x;

        // Obtain R, G, B or A
        base = base * 4 + color_offset;
        return base;
    };

    /// Calculates index into spritemap buffer
    auto calc_spritemap_index = [&](const uint8_t color_offset) {
        auto buffer_index = SafeCast<uint64_t>(context.spritemapWidth) * offset.y;
        buffer_index += offset.x;

        // Obtain R, G, B or A
        buffer_index = buffer_index * 4 + color_offset;
        return buffer_index;
    };


    for (uint8_t color_offset = 0; color_offset < 4; ++color_offset) {
        const auto image_index     = calc_image_index(color_offset);
        const auto spritemap_index = calc_spritemap_index(color_offset);

        context.spritemapBuffer[spritemap_index] = image.buffer[image_index];
    }
}

void render::RendererSprites::SetImageBorder(GeneratorContext& context,
                                             const proto::ImageContainer& image,
                                             const Position2<SpritemapDimensionT> offset) {
    for (unsigned border_i = 0; border_i < kSpriteBorder; ++border_i) {
        // Top and bottom
        for (unsigned i = 0; i < image.width; ++i) {
            SetSpritemapPixel(context, //
                              {offset.x + kSpriteBorder + i, offset.y + border_i},
                              image,
                              {i, 0});
            SetSpritemapPixel(context, //
                              {offset.x + kSpriteBorder + i, offset.y + kSpriteBorder + image.height + border_i},
                              image,
                              {i, SafeCast<unsigned>(image.height - 1)});
        }

        // Left and right
        for (unsigned i = 0; i < image.height; ++i) {
            SetSpritemapPixel(context, //
                              {offset.x + border_i, offset.y + kSpriteBorder + i},
                              image,
                              {0, i});
            SetSpritemapPixel(context, //
                              {offset.x + kSpriteBorder + image.width + border_i, offset.y + kSpriteBorder + i},
                              image,
                              {SafeCast<unsigned>(image.width - 1), i});
        }
    }
}

void render::RendererSprites::GenerateSpritemapOutput(GeneratorContext& context,
                                                      GeneratorNode& base_node,
                                                      Position2<SpritemapDimensionT> offset) {
    auto* current_node = &base_node;

    while (true) {
        auto* sprite      = current_node->sprite;
        const auto& image = sprite->GetImage();

        SetImageBorder(context, image, offset);

        // Copy image onto spritemap
        for (unsigned y = 0; y < image.height; ++y) {
            for (unsigned x = 0; x < image.width; ++x) {
                SetSpritemapPixel(context, //
                                  {offset.x + kSpriteBorder + x, offset.y + kSpriteBorder + y},
                                  image,
                                  {x, y});
            }
        }

        context.texCoords.push_back({{SafeCast<float>(offset.x + kSpriteBorder), //
                                      SafeCast<float>(offset.y + kSpriteBorder)},
                                     {SafeCast<float>(offset.x + kSpriteBorder + image.width),
                                      SafeCast<float>(offset.y + kSpriteBorder + image.height)}});
        sprite->texCoordId = (*context.texCoordIdCounter)++;

        if (current_node->above != nullptr) {
            GenerateSpritemapOutput(context, *current_node->above, {offset.x, offset.y + TotalSpriteHeight(sprite)});
        }

        offset.x += TotalSpriteWidth(sprite);

        if (current_node->right != nullptr)
            current_node = current_node->right;
        else
            break;
    }
}
