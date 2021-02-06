// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#define STB_IMAGE_IMPLEMENTATION

#include "proto/sprite.h"

#include <sstream>
#include <stb/stb_image.h>
#include <type_traits>
#include <utility>

#include "core/convert.h"
#include "core/coordinate_tuple.h"
#include "data/prototype_manager.h"

using namespace jactorio;

proto::Sprite::Sprite(const std::string& sprite_path) {
    LoadImage(sprite_path);
}

proto::Sprite::Sprite(const std::string& sprite_path, std::vector<SpriteGroup> group) : group(std::move(group)) {
    LoadImage(sprite_path);
}

proto::Sprite::~Sprite() {
    stbi_image_free(spriteBuffer_);
}

proto::Sprite::Sprite(const Sprite& other)
    : FrameworkBase{other},
      group{other.group},
      invertSetFrame{other.invertSetFrame},
      frames{other.frames},
      sets{other.sets},
      trim{other.trim},
      width_{other.width_},
      height_{other.height_},
      bytesPerPixel_{other.bytesPerPixel_},
      spritePath_{other.spritePath_},
      spriteBuffer_{other.spriteBuffer_} {

    const auto size = core::SafeCast<std::size_t>(other.width_) * other.height_ * other.bytesPerPixel_;
    spriteBuffer_   = static_cast<unsigned char*>(malloc(size * sizeof(*spriteBuffer_))); // NOLINT: stbi uses malloc
    for (std::size_t i = 0; i < size; ++i) {
        spriteBuffer_[i] = other.spriteBuffer_[i];
    }
}

proto::Sprite::Sprite(Sprite&& other) noexcept
    : FrameworkBase{std::move(other)},
      group{std::move(other.group)},
      frames{other.frames},
      sets{other.sets},
      trim{other.trim},
      width_{other.width_},
      height_{other.height_},
      bytesPerPixel_{other.bytesPerPixel_},
      spritePath_{std::move(other.spritePath_)},
      spriteBuffer_{other.spriteBuffer_} {
    other.spriteBuffer_ = nullptr;
}

// ======================================================================

bool proto::Sprite::IsInGroup(const SpriteGroup group) const {
    for (const auto& i : this->group) {
        if (i == group)
            return true;
    }

    return false;
}

void proto::Sprite::DefaultSpriteGroup(const std::vector<SpriteGroup>& new_group) {
    LOG_MESSAGE(debug, "Using default sprite group:");
    for (const auto& group : new_group) {
        LOG_MESSAGE_F(debug, "    %d", static_cast<int>(group));
    }


    if (group.empty()) {
        group = new_group;
    }
}

void proto::Sprite::LoadImageFromFile() {
    spriteBuffer_ = stbi_load(spritePath_.c_str(),
                              &width_,
                              &height_,
                              &bytesPerPixel_,
                              4 // 4 desired channels for RGBA
    );

    if (spriteBuffer_ == nullptr) {
        LOG_MESSAGE_F(error, "Failed to read sprite at: %s", spritePath_.c_str());

        std::ostringstream sstr;
        sstr << "Failed to read sprite at: " << spritePath_;

        throw ProtoError(sstr.str());
    }
}


UvPositionT proto::Sprite::GetCoords(SpriteSetT set, SpriteFrameT frame) const {
    float width_base  = core::SafeCast<float>(width_) / core::SafeCast<float>(frames);
    float height_base = core::SafeCast<float>(height_) / core::SafeCast<float>(sets);

    // If inverted:
    // Set   = X axis
    // Frame = Y axis
    if (invertSetFrame) {
        width_base  = core::SafeCast<float>(width_) / core::SafeCast<float>(sets);
        height_base = core::SafeCast<float>(height_) / core::SafeCast<float>(frames);

        AdjustSetFrame<false>(frame, set);

        assert(set < frames); // Out of range
        assert(frame < sets);
    }
    else {
        AdjustSetFrame<true>(set, frame);

        assert(set < sets); // Out of range
        assert(frame < frames);
    }

    return {
        {(width_base * core::SafeCast<float>(frame) + core::SafeCast<float>(trim)) / core::SafeCast<float>(width_),
         (height_base * core::SafeCast<float>(set) + core::SafeCast<float>(trim)) / core::SafeCast<float>(height_)},
        {(width_base * core::SafeCast<float>(frame + 1) - core::SafeCast<float>(trim)) / core::SafeCast<float>(width_),
         (height_base * core::SafeCast<float>(set + 1) - core::SafeCast<float>(trim)) /
             core::SafeCast<float>(height_)}};
}

const unsigned char* proto::Sprite::GetSpritePtr() const {
    return spriteBuffer_;
}

proto::Sprite* proto::Sprite::LoadImage(const std::string& image_path) {
    spritePath_ = std::string(data::PrototypeManager::kDataFolder) + "/" + image_path;
    LoadImageFromFile();

    return this;
}

void proto::Sprite::PostLoadValidate(const data::PrototypeManager& /*proto*/) const {
    J_PROTO_ASSERT(frames > 0, "Frames must be at least 1");
    J_PROTO_ASSERT(sets > 0, "Sets must be at least 1");
}
