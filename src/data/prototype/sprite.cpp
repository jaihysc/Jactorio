// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/06/2019

#define STB_IMAGE_IMPLEMENTATION

#include "data/prototype/sprite.h"

#include <sstream>
#include <utility>
#include <stb/stb_image.h>

#include "core/filesystem.h"
#include "data/data_exception.h"

jactorio::data::Sprite::Sprite(const std::string& sprite_path) {
	LoadImage(sprite_path);
}

jactorio::data::Sprite::Sprite(const std::string& sprite_path, std::vector<SpriteGroup> group)
	: group(std::move(group)) {
	LoadImage(sprite_path);
}

jactorio::data::Sprite::~Sprite() {
	stbi_image_free(spriteBuffer_);
}

jactorio::data::Sprite::Sprite(const Sprite& other)
	: PrototypeBase(other),
	  group(other.group),
	  width_(other.width_),
	  height_(other.height_),
	  bytesPerPixel_(other.bytesPerPixel_),
	  spritePath_(other.spritePath_),
	  spriteBuffer_(other.spriteBuffer_) {

	const auto size = static_cast<unsigned long long>(other.width_) * other.height_ * other.bytesPerPixel_;
	spriteBuffer_   = static_cast<unsigned char*>(malloc(size * sizeof(*spriteBuffer_)));  // stbi uses malloc
	for (unsigned long long i = 0; i < size; ++i) {
		spriteBuffer_[i] = other.spriteBuffer_[i];
	}
}

jactorio::data::Sprite::Sprite(Sprite&& other) noexcept
	: PrototypeBase{std::move(other)},
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

bool jactorio::data::Sprite::IsInGroup(const SpriteGroup group) {
	for (auto& i : this->group) {
		if (i == group)
			return true;
	}

	return false;
}

void jactorio::data::Sprite::DefaultSpriteGroup(const std::vector<SpriteGroup>& new_group) {
	LOG_MESSAGE(debug, "Using default sprite group:");
	for (auto& group : new_group) {
		LOG_MESSAGE_f(debug, "    %d", static_cast<int>(group));
	}

	
	if (group.empty()) {
		group = new_group;
	}
}

void jactorio::data::Sprite::LoadImageFromFile() {
	spriteBuffer_ = stbi_load(
		spritePath_.c_str(),
		&width_,
		&height_,
		&bytesPerPixel_,
		4  // 4 desired channels for RGBA
	);

	if (!spriteBuffer_) {
		LOG_MESSAGE_f(error, "Failed to read sprite at: %s", spritePath_.c_str());

		std::ostringstream sstr;
		sstr << "Failed to read sprite at: " << spritePath_;

		throw DataException(sstr.str());
	}
}


void jactorio::data::Sprite::AdjustSetFrame(RenderableData::SetT& set, RenderableData::FrameT& frame) const {
	set %= sets;
	set += frame / frames;
	frame = frame % frames;
}

jactorio::core::QuadPosition jactorio::data::Sprite::GetCoords(RenderableData::SetT set,
                                                               RenderableData::FrameT frame) const {
	AdjustSetFrame(set, frame);

	assert(set < sets);  // Out of range
	assert(frame < frames);

	return {
		{
			1.f / static_cast<float>(frames) * static_cast<float>(frame),
			1.f / static_cast<float>(sets) * static_cast<float>(set)
		},
		{
			1.f / static_cast<float>(frames) * static_cast<float>(frame + 1),
			1.f / static_cast<float>(sets) * static_cast<float>(set + 1)
		}
	};
}

jactorio::core::QuadPosition jactorio::data::Sprite::GetCoordsTrimmed(RenderableData::SetT set,
                                                                      RenderableData::FrameT frame) const {
	AdjustSetFrame(set, frame);

	assert(set < sets);  // Out of range
	assert(frame < frames);

	const auto width_base  = static_cast<float>(width_) / static_cast<float>(frames);
	const auto height_base = static_cast<float>(height_) / static_cast<float>(sets);

	return {
		{
			(width_base * static_cast<float>(frame) + static_cast<float>(trim))
			/ static_cast<float>(width_),
			(height_base * static_cast<float>(set) + static_cast<float>(trim))
			/ static_cast<float>(height_)
		},
		{
			(width_base * static_cast<float>(frame + 1) - static_cast<float>(trim))
			/ static_cast<float>(width_),
			(height_base * static_cast<float>(set + 1) - static_cast<float>(trim))
			/ static_cast<float>(height_)
		}
	};
}

const unsigned char* jactorio::data::Sprite::GetSpritePtr() const {
	return spriteBuffer_;
}

jactorio::data::Sprite* jactorio::data::Sprite::LoadImage(const std::string& image_path) {
	spritePath_ = core::ResolvePath("data/" + image_path);
	LoadImageFromFile();

	return this;
}

void jactorio::data::Sprite::PostLoadValidate() const {
	J_DATA_ASSERT(frames > 0, "Frames must be at least 1");
	J_DATA_ASSERT(sets > 0, "Sets must be at least 1");
}
