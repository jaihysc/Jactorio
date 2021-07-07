// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_SPRITE_H
#define JACTORIO_INCLUDE_PROTO_SPRITE_H
#pragma once

#include <string>
#include <vector>

#include "jactorio.h"

#include "core/data_type.h"
#include "proto/framework/framework_base.h"

namespace jactorio::proto
{
    /// Simplifies copy/move constructor for sprite
    struct ImageContainer
    {
        ImageContainer() = default;
        /// Loads image from image_path
        /// \exception ProtoError Failed to load from file
        explicit ImageContainer(const std::string& image_path);
        ~ImageContainer();

        ImageContainer(const ImageContainer& other);
        ImageContainer(ImageContainer&& other) noexcept;

        ImageContainer& operator=(ImageContainer other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        friend void swap(ImageContainer& lhs, ImageContainer& rhs) noexcept {
            using std::swap;
            swap(lhs.width, rhs.width);
            swap(lhs.height, rhs.height);
            swap(lhs.bytesPerPixel, rhs.bytesPerPixel);
            swap(lhs.buffer, rhs.buffer);
        }

        int width = 0, height = 0, bytesPerPixel = 0;
        unsigned char* buffer = nullptr;
    };

    /// Unique data: Renderable_data
    class Sprite final : public FrameworkBase
    {
    public:
        PROTOTYPE_CATEGORY(sprite);

        using SpriteDimension = uint32_t;

        enum class SpriteGroup
        {
            none = 0,
            terrain,
            gui,
            count_
        };

        Sprite() = default;
        explicit Sprite(const std::string& sprite_path);
        Sprite(const std::string& sprite_path, SpriteGroup group);

        /// Loads image from image_path
        /// \remark Do not include ~/data/
        Sprite* Load(const std::string& image_path);

        J_NODISCARD const ImageContainer& GetImage() const noexcept;

        // Sprite properties

        /// Determines which spritemap this sprite is placed on
        PYTHON_PROP_REF_I(SpriteGroup, group, SpriteGroup::none);

        /*
         *     F0 F1 F2 F3 F4
         *    ----------------
         * S0 |  |  |  |  |  |
         *    ----------------
         * S1 |  |  |  |  |  |
         *    ----------------
         * S2 |  |  |  |  |  |
         *    ----------------
         */

        /// If true : X = Set, Y = Frame,
        ///			  false: Y = Set, X = Frame
        PYTHON_PROP_REF_I(bool, invertSetFrame, false);

        /// Animation frames, X axis, indexed by 0 based index, 1 if single
        PYTHON_PROP_REF_I(SpriteFrameT, frames, 1);
        /// Y axis, indexed by 0 based index, 1 if single
        PYTHON_PROP_REF_I(SpriteSetT, sets, 1);

        /// Pixels to remove from the border
        PYTHON_PROP_REF_I(SpriteTrimT, trim, 0);


        void PostLoadValidate(const data::PrototypeManager& proto) const override;

        SpriteTexCoordIndexT texCoordId = 0;

        /// Tells spritemap generator to divide one frame at a set into
        /// subdivide.x frames horizontally. subdivide.y frames vertically
        Dimension subdivide{1, 1};

    private:
        ImageContainer image_;
        /// Full path to sprite
        std::string spritePath_;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_SPRITE_H
