// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#define STB_IMAGE_IMPLEMENTATION

#include "proto/sprite.h"

#include <sstream>
#include <stb/stb_image.h>
#include <type_traits>
#include <utility>

#include "core/convert.h"
#include "data/prototype_manager.h"

using namespace jactorio;

proto::ImageContainer::ImageContainer(const std::string& image_path) {
    buffer = stbi_load(image_path.c_str(),
                       &width,
                       &height,
                       &bytesPerPixel,
                       4 // 4 desired channels for RGBA
    );

    if (buffer == nullptr) {
        LOG_MESSAGE_F(error, "Failed to read sprite at: %s", image_path.c_str());

        std::ostringstream sstr;
        sstr << "Failed to read sprite at: " << image_path;

        throw ProtoError(sstr.str());
    }
}

proto::ImageContainer::~ImageContainer() {
    stbi_image_free(buffer);
}

proto::ImageContainer::ImageContainer(const ImageContainer& other)
    : width{other.width}, height{other.height}, bytesPerPixel{other.bytesPerPixel} {

    const auto size = SafeCast<std::size_t>(other.width) * other.height * other.bytesPerPixel;
    buffer          = static_cast<unsigned char*>(malloc(size * sizeof(*buffer))); // NOLINT: stbi uses malloc

    if (buffer == nullptr) {
        throw std::bad_alloc();
    }

    for (std::size_t i = 0; i < size; ++i) {
        buffer[i] = other.buffer[i];
    }
}

proto::ImageContainer::ImageContainer(ImageContainer&& other) noexcept
    : width{other.width}, height{other.height}, bytesPerPixel{other.bytesPerPixel}, buffer{other.buffer} {
    other.buffer = nullptr;
}

// ======================================================================

proto::Sprite::Sprite(const std::string& sprite_path) {
    Load(sprite_path);
}

proto::Sprite::Sprite(const std::string& sprite_path, std::vector<SpriteGroup> group) : group(std::move(group)) {
    Load(sprite_path);
}

proto::Sprite* proto::Sprite::Load(const std::string& image_path) {
    spritePath_ = std::string(data::PrototypeManager::kDataFolder) + "/" + image_path;
    image_      = ImageContainer(spritePath_);

    return this;
}

const proto::ImageContainer& proto::Sprite::GetImage() const noexcept {
    return image_;
}

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

void proto::Sprite::PostLoadValidate(const data::PrototypeManager& /*proto*/) const {
    J_PROTO_ASSERT(frames > 0, "Frames must be at least 1");
    J_PROTO_ASSERT(sets > 0, "Sets must be at least 1");
}
