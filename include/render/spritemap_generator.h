// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_SPRITEMAP_GENERATOR_H
#define JACTORIO_INCLUDE_RENDER_SPRITEMAP_GENERATOR_H
#pragma once

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/coordinate_tuple.h"
#include "core/data_type.h"
#include "proto/sprite.h"
#include "render/opengl/texture.h"

namespace jactorio::render
{
    struct Animation
    {
        /// Index into tex coords vector for first tex coord of this animation
        std::size_t texCoordIndex = 0;

        /// Total number of frames
        int frames       = 0;
        int currentFrame = 0;
        /// Tex coords per frame
        int span = 0;
    };

    class Spritemap
    {
    public:
        using DimensionT = uint64_t;

        explicit Spritemap(SpriteTexCoords tex_coords, std::vector<Animation> animation)
            : texCoords_(std::move(tex_coords)), animations_(std::move(animation)) {}


        J_NODISCARD const SpriteTexCoords& GetTexCoords() const noexcept {
            return texCoords_;
        }

        /// Generates tex coords for current frame of animation
        /// \return Pointer to tex coords, amount of tex coords
        J_NODISCARD std::pair<const TexCoord*, int> GenCurrentFrame() const;

        /// Generates tex coords for next frame of animation
        /// \return Pointer to tex coords, amount of tex coords
        J_NODISCARD std::pair<const TexCoord*, int> GenNextFrame() const;

        std::shared_ptr<Texture::SpriteBufferT> spriteBuffer;

        DimensionT width  = 0;
        DimensionT height = 0;

    private:
        /// 0 - 1 positions of the sprite within the spritemap
        /// Upper left is 0, 0 - bottom right is 1, 1
        SpriteTexCoords texCoords_;
        /// Animation information for each sprite
        std::vector<Animation> animations_;

        /// Generated tex coords is saved here first prior to copying to GPU
        mutable std::vector<TexCoord> buffer_;
    };

    /// Generates spritemaps on initialization with tile sprites
    /// - Concatenate sprite into spritemap
    /// - Location of a sprite within spritemap
    /// \remark Gl methods requires OpenGL context
    class RendererSprites
    {
        using SpritemapDimensionT = Spritemap::DimensionT;

        /// Additional border to each sprite, use to avoid black lines
        static constexpr auto kSpriteBorder                     = 1;
        static constexpr SpritemapDimensionT kMaxSpritemapWidth = 99999;

    public:
        RendererSprites() = default;

        ~RendererSprites() {
            Clear();
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
            swap(lhs.spritemaps_, rhs.spritemaps_);
        }

        /// Frees all spritemap memory
        void Clear();

        /// Creates a spritemap and stores it as a render::Texture
        void GlInitializeSpritemap(const data::PrototypeManager& proto,
                                   proto::Sprite::SpriteGroup group,
                                   bool invert_sprites);

        /// Creates a spritemap using sprites in PrototypeManager proto of SpriteGroup group
        /// \param invert_sprites If true, flips each sprite across its X axis
        J_NODISCARD static Spritemap CreateSpritemap(const data::PrototypeManager& proto,
                                                     proto::Sprite::SpriteGroup group,
                                                     bool invert_sprites);

        /// Retrieves spritemap at specified group
        const Spritemap& GetSpritemap(proto::Sprite::SpriteGroup group);
        const Texture* GetTexture(proto::Sprite::SpriteGroup group);

        /// Generates spritemap
        /// Assigns texCoordId to sprites
        /// \remark Color in non specified areas of the spritemap are undefined
        /// \param sprites Collection of pointers towards sprite prototypes
        /// \param invert_sprites If true, flips each sprite across its X axis
        J_NODISCARD static Spritemap GenSpritemap(std::vector<proto::Sprite*> sprites, bool invert_sprites);

    private:
        /// Holds a sprite and its neighbors on the spritemap
        struct GeneratorNode
        {
            explicit GeneratorNode(proto::Sprite* sprite) : sprite(sprite) {}

            proto::Sprite* sprite;

            GeneratorNode* above = nullptr;
            GeneratorNode* right = nullptr;
        };

        struct GeneratorContext
        {
            Texture::SpriteBufferT* spritemapBuffer;
            SpritemapDimensionT spritemapWidth;
            SpriteTexCoords& texCoords;
            bool invertSprites;

            SpriteTexCoordIndexT texCoordIdCounter = 1;
            std::vector<Animation> animations;
        };


        /// Sprite width with border
        static proto::Sprite::SpriteDimension TotalSpriteWidth(const proto::Sprite& sprite) noexcept;
        /// Sprite height with border
        static proto::Sprite::SpriteDimension TotalSpriteHeight(const proto::Sprite& sprite) noexcept;


        static void SortInputSprites(std::vector<proto::Sprite*>& sprites);

        /// Recursively creates linked GeneratorNodes of sprites into node_buffer
        /// Will erase from sprites as each sprite is used
        static void GenerateSpritemapNodes(std::vector<proto::Sprite*>& sprites,
                                           std::vector<GeneratorNode*>& node_buffer,
                                           GeneratorNode& parent_node,
                                           SpritemapDimensionT max_width,
                                           SpritemapDimensionT max_height);


        /// Calculates width of spritemap with adjustments
        /// \param base_node node above parent node
        static SpritemapDimensionT GetSpritemapWidth(GeneratorNode& base_node);


        /// Copies pixel of image at provided pixel_pos to spritemap buffer at offset
        /// \param offset Offset for writing into spritemap
        static void SetSpritemapPixel(GeneratorContext& context,
                                      Position2<SpritemapDimensionT> offset,
                                      const proto::ImageContainer& image,
                                      Position2<unsigned> pixel_pos);

        /// Adds a border around of kSpriteBorder around the image in spritemap
        /// \param offset Offset for writing into spritemap
        static void SetImageBorder(GeneratorContext& context,
                                   const proto::ImageContainer& image,
                                   Position2<SpritemapDimensionT> offset);

        /// \return Number of frames/sets which should have tex coords generated, for 1 animation tick of the game
        static std::pair<SpriteFrameT, SpriteSetT> GetGameTickFrameSet(const proto::Sprite& sprite) noexcept;

        /// Generates animation + ALL the tex coords for the sprite (all frames + sets)
        /// - Assigns tex coords id to sprite
        /// \param sprite Sprite having its animations generated
        static void GenerateAnimationTexCoords(GeneratorContext& context,
                                               Position2<SpritemapDimensionT> offset,
                                               proto::Sprite& sprite);

        /// Recursively processes GeneratorNodes
        /// - outputs sprites into into provided sprite buffer
        /// - Assigns tex coord id to sprites
        /// - sets NON-normalized tex coord
        /// \param offset Offset for writing into spritemap
        static void GenerateSpritemapOutput(GeneratorContext& context,
                                            GeneratorNode& base_node,
                                            Position2<SpritemapDimensionT> offset);

        std::map<unsigned int, Texture*> textures_;
        std::map<unsigned int, Spritemap> spritemaps_;
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_SPRITEMAP_GENERATOR_H
