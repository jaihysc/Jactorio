// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/06/2019

#define STB_IMAGE_IMPLEMENTATION

#include "data/prototype/sprite.h"

#include <sstream>
#include <utility>
#include <stb/stb_image.h>

#include "core/filesystem.h"
#include "data/data_exception.h"

bool jactorio::data::Sprite::is_in_group(const spriteGroup group) {
	for (auto& i : this->group) {
		if (i == group)
			return true;
	}

	return false;
}

void jactorio::data::Sprite::load_image_from_file() {
	sprite_buffer_ = stbi_load(
		sprite_path_.c_str(),
		&width_,
		&height_,
		&bytes_per_pixel_,
		4  // 4 desired channels for RGBA
	);

	if (!sprite_buffer_) {
		LOG_MESSAGE_f(error, "Failed to read sprite at: %s", sprite_path_.c_str());

		std::ostringstream sstr;
		sstr << "Failed to read sprite at: " << sprite_path_;

		throw Data_exception(sstr.str());
	}
}

jactorio::data::Sprite::Sprite()
	: width_(0), height_(0), bytes_per_pixel_(0), sprite_buffer_(nullptr) {
}

jactorio::data::Sprite::Sprite(const std::string& sprite_path)
	: width_(0), height_(0), bytes_per_pixel_(0), sprite_buffer_(nullptr) {
	load_image(sprite_path);
}

jactorio::data::Sprite::Sprite(const std::string& sprite_path, std::vector<spriteGroup> group)
	: group(std::move(group)), width_(0), height_(0), bytes_per_pixel_(0), sprite_buffer_(nullptr) {
	load_image(sprite_path);
}

jactorio::data::Sprite::~Sprite() {
	stbi_image_free(sprite_buffer_);
}

jactorio::data::Sprite::Sprite(const Sprite& other)
	: Prototype_base(other),
	  group(other.group),
	  width_(other.width_),
	  height_(other.height_),
	  bytes_per_pixel_(other.bytes_per_pixel_),
	  sprite_path_(other.sprite_path_),
	  sprite_buffer_(other.sprite_buffer_) {

	const auto size = static_cast<unsigned long long>(other.width_) * other.height_ * other.bytes_per_pixel_;
	sprite_buffer_ = static_cast<unsigned char*>(malloc(size * sizeof(*sprite_buffer_)));  // stbi uses malloc
	for (unsigned long long i = 0; i < size; ++i) {
		sprite_buffer_[i] = other.sprite_buffer_[i];
	}
}

jactorio::data::Sprite::Sprite(Sprite&& other) noexcept
	: Prototype_base{std::move(other)},
	  group{std::move(other.group)},
	  frames{other.frames},
	  sets{other.sets},
	  trim{other.trim},
	  width_{other.width_},
	  height_{other.height_},
	  bytes_per_pixel_{other.bytes_per_pixel_},
	  sprite_path_{std::move(other.sprite_path_)},
	  sprite_buffer_{other.sprite_buffer_} {
	other.sprite_buffer_ = nullptr;
}

jactorio::core::Quad_position jactorio::data::Sprite::get_coords(uint16_t mset, const uint16_t frame) const {
	mset %= sets;
	assert(mset < sets);  // Out of range
	assert(frame < frames);

	return {
		{
			1.f / static_cast<float>(frames) * static_cast<float>(frame),
			1.f / static_cast<float>(sets) * static_cast<float>(mset)
		},
		{
			1.f / static_cast<float>(frames) * static_cast<float>(frame + 1),
			1.f / static_cast<float>(sets) * static_cast<float>(mset + 1)
		}
	};
}

jactorio::core::Quad_position jactorio::data::Sprite::
get_coords_trimmed(uint16_t mset, const uint16_t frame) const {
	mset %= sets;
	assert(mset < sets);  // Out of range
	assert(frame < frames);

	const auto width_base = static_cast<float>(width_) / static_cast<float>(frames);
	const auto height_base = static_cast<float>(height_) / static_cast<float>(sets);

	return {
		{
			(width_base * static_cast<float>(frame) + static_cast<float>(trim))
			/ static_cast<float>(width_),
			(height_base * static_cast<float>(mset) + static_cast<float>(trim))
			/ static_cast<float>(height_)
		},
		{
			(width_base * static_cast<float>(frame + 1) - static_cast<float>(trim))
			/ static_cast<float>(width_),
			(height_base * static_cast<float>(mset + 1) - static_cast<float>(trim))
			/ static_cast<float>(height_)
		}
	};
}

const unsigned char* jactorio::data::Sprite::get_sprite_data_ptr() const {
	return sprite_buffer_;
}

jactorio::data::Sprite* jactorio::data::Sprite::load_image(const std::string& image_path) {
	sprite_path_ = core::resolve_path("~/data/" + image_path);
	load_image_from_file();

	return this;
}

void jactorio::data::Sprite::post_load_validate() const {
	J_DATA_ASSERT(frames > 0, "Frames must be at least 1");
	J_DATA_ASSERT(sets > 0, "Sets must be at least 1");
}
