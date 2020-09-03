// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#define STB_IMAGE_IMPLEMENTATION

#include "data/prototype/sprite.h"

#include <sstream>
#include <type_traits>
#include <utility>
#include <stb/stb_image.h>

#include "core/filesystem.h"
#include "core/math.h"
#include "data/data_exception.h"

using namespace jactorio;

data::Sprite::Sprite(const std::string& sprite_path) {
	LoadImage(sprite_path);
}

data::Sprite::Sprite(const std::string& sprite_path, std::vector<SpriteGroup> group)
	: group(std::move(group)) {
	LoadImage(sprite_path);
}

data::Sprite::~Sprite() {
	stbi_image_free(spriteBuffer_);
}

data::Sprite::Sprite(const Sprite& other)
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
	spriteBuffer_   = static_cast<unsigned char*>(malloc(size * sizeof(*spriteBuffer_)));  // stbi uses malloc
	for (std::size_t i = 0; i < size; ++i) {
		spriteBuffer_[i] = other.spriteBuffer_[i];
	}
}

data::Sprite::Sprite(Sprite&& other) noexcept
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

bool data::Sprite::IsInGroup(const SpriteGroup group) const {
	for (const auto& i : this->group) {
		if (i == group)
			return true;
	}

	return false;
}

void data::Sprite::DefaultSpriteGroup(const std::vector<SpriteGroup>& new_group) {
	LOG_MESSAGE(debug, "Using default sprite group:");
	for (const auto& group : new_group) {
		LOG_MESSAGE_F(debug, "    %d", static_cast<int>(group));
	}


	if (group.empty()) {
		group = new_group;
	}
}

void data::Sprite::LoadImageFromFile() {
	spriteBuffer_ = stbi_load(
		spritePath_.c_str(),
		&width_,
		&height_,
		&bytesPerPixel_,
		4  // 4 desired channels for RGBA
	);

	if (!spriteBuffer_) {
		LOG_MESSAGE_F(error, "Failed to read sprite at: %s", spritePath_.c_str());

		std::ostringstream sstr;
		sstr << "Failed to read sprite at: " << spritePath_;

		throw DataException(sstr.str());
	}
}


UvPositionT data::Sprite::GetCoords(SetT set, FrameT frame) const {
	float width_base  = core::SafeCast<float>(width_) / core::SafeCast<float>(frames);
	float height_base = core::SafeCast<float>(height_) / core::SafeCast<float>(sets);

	// If inverted:
	// Set   = X axis
	// Frame = Y axis
	if (invertSetFrame) {
		width_base  = core::SafeCast<float>(width_) / core::SafeCast<float>(sets);
		height_base = core::SafeCast<float>(height_) / core::SafeCast<float>(frames);

		AdjustSetFrame<false>(frame, set);

		assert(set < frames);  // Out of range
		assert(frame < sets);
	}
	else {
		AdjustSetFrame<true>(set, frame);

		assert(set < sets);  // Out of range
		assert(frame < frames);
	}

	return {
		{
			(width_base * core::SafeCast<float>(frame) + core::SafeCast<float>(trim))
			/ core::SafeCast<float>(width_),
			(height_base * core::SafeCast<float>(set) + core::SafeCast<float>(trim))
			/ core::SafeCast<float>(height_)
		},
		{
			(width_base * core::SafeCast<float>(frame + 1) - core::SafeCast<float>(trim))
			/ core::SafeCast<float>(width_),
			(height_base * core::SafeCast<float>(set + 1) - core::SafeCast<float>(trim))
			/ core::SafeCast<float>(height_)
		}
	};
}

const unsigned char* data::Sprite::GetSpritePtr() const {
	return spriteBuffer_;
}

data::Sprite* data::Sprite::LoadImage(const std::string& image_path) {
	spritePath_ = core::ResolvePath("data/" + image_path);
	LoadImageFromFile();

	return this;
}

void data::Sprite::PostLoadValidate(const PrototypeManager&) const {
	J_DATA_ASSERT(frames > 0, "Frames must be at least 1");
	J_DATA_ASSERT(sets > 0, "Sets must be at least 1");
}
