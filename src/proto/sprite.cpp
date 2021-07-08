// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#define STB_IMAGE_IMPLEMENTATION

#include "proto/sprite.h"

#include <cstdlib>
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

proto::Sprite::Sprite(const std::string& sprite_path, const SpriteGroup group) : group(group) {
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

proto::Sprite* proto::Sprite::Trim(const SpriteTrimT pixels) {
    // Divide first to truncate, as extra pixels on right, bottom are cut when divided per set/frame
    const auto new_width  = (image_.width / frames - 2 * pixels) * frames;
    const auto new_height = (image_.height / sets - 2 * pixels) * sets;

    if (new_width < 0 || new_height < 0) {
        throw ProtoError("Trim too large");
    }

    const auto size        = SafeCast<std::size_t>(new_width) * new_height * image_.bytesPerPixel;
    auto* const new_buffer = static_cast<unsigned char*>(malloc(size * sizeof(*image_.buffer)));

    // Trimming certain sprites produces artifacts if it is done in place
    // As it writes to certain memory locations which have yet been read
    if (new_buffer == nullptr) {
        throw std::bad_alloc();
    }

    /// Pixels per frame
    const auto x_pixels = image_.width / frames - 2 * pixels;
    const auto y_pixels = image_.height / sets - 2 * pixels;

    /// Reads trimmed frame from read_ptr, writes it starting at write_ptr
    auto output_frame = [this, x_pixels, y_pixels, new_width](unsigned char* write_ptr, const unsigned char* read_ptr) {
        for (int y = 0; y < y_pixels; ++y) {
            for (int x = 0; x < x_pixels; ++x) {
                for (int c = 0; c < image_.bytesPerPixel; ++c) {
                    write_ptr[c] = read_ptr[c];
                }
                write_ptr += image_.bytesPerPixel;
                read_ptr += image_.bytesPerPixel;
            }
            // Skip right + left to next start of row for frame
            write_ptr += (new_width - x_pixels) * image_.bytesPerPixel;
            read_ptr += (image_.width - x_pixels) * image_.bytesPerPixel;
        }
    };

    // Saves the trimmed image in place
    auto* write_ptr = new_buffer;
    auto* read_ptr  = image_.buffer;
    read_ptr += pixels * image_.width * image_.bytesPerPixel; // Skip top

    for (int set = 0; set < sets; ++set) {
        for (int frame = 0; frame < frames; ++frame) {
            read_ptr += pixels * image_.bytesPerPixel;
            output_frame(write_ptr, read_ptr);

            // To next frame
            write_ptr += x_pixels * image_.bytesPerPixel;
            read_ptr += x_pixels * image_.bytesPerPixel;
            read_ptr += pixels * image_.bytesPerPixel;
        }
        read_ptr += (image_.width % frames) * image_.bytesPerPixel; // In case image does not divide perfectly

        write_ptr += (y_pixels - 1) * new_width * image_.bytesPerPixel; // To start of next frame at next set
        read_ptr += (y_pixels + 2 * pixels - 1) * image_.width * image_.bytesPerPixel;
    }

    free(image_.buffer);
    image_.buffer = new_buffer;

    image_.width  = new_width;
    image_.height = new_height;

    return this;
}

void proto::Sprite::PostLoadValidate(const data::PrototypeManager& /*proto*/) const {
    J_PROTO_ASSERT(frames > 0, "Frames must be at least 1");
    J_PROTO_ASSERT(sets > 0, "Sets must be at least 1");

    J_PROTO_ASSERT(group != SpriteGroup::none, "Sprite must be in a group");
}
