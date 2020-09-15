// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_RENDERING_SPRITEMAP_GENERATOR_H
#define JACTORIO_INCLUDE_RENDER_RENDERING_SPRITEMAP_GENERATOR_H
#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "core/coordinate_tuple.h"
#include "core/data_type.h"
#include "data/prototype/sprite.h"
#include "render/opengl/texture.h"

namespace jactorio::render
{
    ///
    /// Generates spritemaps on initialization with tile sprites
    /// - Concatenate sprite into spritemap
    /// - Location of a sprite within spritemap
    class RendererSprites
    {
    public:
        using SpritemapDimensionT = uint64_t;

        struct SpritemapData
        {
            // For the loaded sprite
            std::shared_ptr<Texture::SpriteBufferT> spriteBuffer;

            SpritemapDimensionT width  = 0;
            SpritemapDimensionT height = 0;

            // Image positions retrieved via the path originally given to create the spritemap
            // 0 - 1 positions of the sprite within the spritemap
            // Upper left is 0, 0 - bottom right is 1, 1
            // std::string is internal name of prototype
            SpriteUvCoordsT spritePositions;
        };

        // ======================================================================

        RendererSprites() = default;

        ~RendererSprites() {
            ClearSpritemaps();
        }

        RendererSprites(const RendererSprites& other)     = delete;
        RendererSprites(RendererSprites&& other) noexcept = delete;

        RendererSprites& operator=(RendererSprites other) {
            swap(*this, other);
            return *this;
        }

        friend void swap(RendererSprites& lhs, RendererSprites& rhs) noexcept {
            using std::swap;
            swap(lhs.textures_, rhs.textures_);
            swap(lhs.spritemapDatas_, rhs.spritemapDatas_);
        }

    private:
        std::map<unsigned int, Texture*> textures_;
        std::map<unsigned int, SpritemapData> spritemapDatas_;

    public:
        ///
        /// Frees all spritemap memory
        void ClearSpritemaps();

        ///
        /// Creates a spritemap and stores it as a render::Texture
        /// \remark Requires OpenGL context
        void GInitializeSpritemap(const data::PrototypeManager& data_manager,
                                  data::Sprite::SpriteGroup group,
                                  bool invert_sprites);

        ///
        /// Creates a spritemap
        J_NODISCARD SpritemapData CreateSpritemap(const data::PrototypeManager& data_manager,
                                                  data::Sprite::SpriteGroup group,
                                                  bool invert_sprites) const;

        ///
        /// Retrieves spritemap at specified group
        const SpritemapData& GetSpritemap(data::Sprite::SpriteGroup group);
        const Texture* GetTexture(data::Sprite::SpriteGroup group);

        ///
        /// Generates spritemap
        /// \remark Color in non specified areas of the spritemap are undefined
        /// \param sprites Collection of pointers towards sprite prototypes
        /// \param invert_sprites Whether or not to vertically invert the sprites on the spritemap. Commonly done for
        /// OpenGL
        J_NODISCARD SpritemapData GenSpritemap(const std::vector<const data::Sprite*>& sprites,
                                               bool invert_sprites) const;

    private:
        /// Additional border to each sprite, use to avoid black lines
        static constexpr int sprite_border = 1;

        ///
        /// Holds a sprite and its neighbors on the spritemap
        struct GeneratorNode
        {
            explicit GeneratorNode(const data::Sprite* sprite) : sprite(sprite) {}

            const data::Sprite* sprite;

            GeneratorNode* above = nullptr;
            GeneratorNode* right = nullptr;
        };

        ///
        /// Gets sprite width with adjustments
        static data::Sprite::SpriteDimension GetSpriteWidth(const data::Sprite* sprite);
        ///
        /// Gets sprite height with adjustments
        static data::Sprite::SpriteDimension GetSpriteHeight(const data::Sprite* sprite);


        static void SortInputSprites(std::vector<const data::Sprite*>& sprites);

        ///
        /// Recursively creates linked GeneratorNodes of sprites
        ///
        /// Will erase from sprites as each sprite is used
        static void GenerateSpritemapNodes(std::vector<const data::Sprite*>& sprites,
                                           std::vector<GeneratorNode*>& node_buffer,
                                           GeneratorNode& parent_node,
                                           SpritemapDimensionT max_width,
                                           SpritemapDimensionT max_height);


        ///
        /// Calculates width of spritemap with adjustments
        /// \param base_node node above parent node
        static SpritemapDimensionT GetSpritemapWidth(GeneratorNode& base_node);


        ///
        /// Copies pixel at sprite_x, sprite_y to spritemap buffer
        static void SetSpritemapPixel(std::shared_ptr<Texture::SpriteBufferT>& spritemap_buffer,
                                      SpritemapDimensionT spritemap_width,
                                      bool invert_sprites,
                                      SpritemapDimensionT spritemap_x_offset,
                                      SpritemapDimensionT spritemap_y_offset,
                                      const unsigned char* sprite_data,
                                      data::Sprite::SpriteDimension sprite_width,
                                      data::Sprite::SpriteDimension sprite_height,
                                      unsigned pixel_x,
                                      unsigned pixel_y);
        ///
        /// Recursively outputs GeneratorNodes into provided sprite buffer
        static void GenerateSpritemapOutput(std::shared_ptr<Texture::SpriteBufferT>& spritemap_buffer,
                                            SpritemapDimensionT spritemap_width,
                                            GeneratorNode& base_node,
                                            bool invert_sprites,
                                            SpriteUvCoordsT& image_positions,
                                            SpritemapDimensionT x_offset,
                                            SpritemapDimensionT y_offset);
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_RENDERING_SPRITEMAP_GENERATOR_H
