// 
// sprite.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 12/06/2019
// Last modified: 03/16/2020
// 

#define STB_IMAGE_IMPLEMENTATION

#include "data/prototype/sprite.h"

#include <sstream>
#include <stb/stb_image.h>

#include "core/filesystem.h"
#include "data/data_exception.h"

bool jactorio::data::Sprite::is_in_group(const sprite_group group) {
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

jactorio::data::Sprite::Sprite(const std::string& sprite_path, const std::vector<sprite_group> group)
	: group(group), width_(0), height_(0), bytes_per_pixel_(0), sprite_buffer_(nullptr) {
	load_image(sprite_path);
}

jactorio::data::Sprite::~Sprite() {
	delete[] sprite_buffer_;
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
	sprite_buffer_ = new unsigned char[size];
	for (int i = 0; i < size; ++i) {
		sprite_buffer_[i] = other.sprite_buffer_[i];
	}
}

jactorio::data::Sprite& jactorio::data::Sprite::operator=(const Sprite& other) {
	if (this == &other)
		return *this;
	Prototype_base::operator =(other);
	group = other.group;
	width_ = other.width_;
	height_ = other.height_;
	bytes_per_pixel_ = other.bytes_per_pixel_;
	sprite_path_ = other.sprite_path_;
	sprite_buffer_ = other.sprite_buffer_;

	const auto size = static_cast<unsigned long long>(other.width_) * other.height_ * other.bytes_per_pixel_;
	sprite_buffer_ = new unsigned char[size];
	for (int i = 0; i < size; ++i) {
		sprite_buffer_[i] = other.sprite_buffer_[i];
	}

	return *this;
}


jactorio::core::Quad_position jactorio::data::Sprite::get_coords(const uint16_t set, const uint16_t frame) const {
	assert(set < sets);  // Out of range
	assert(frame < frames);
	
	return {
		{
			1.f / frames * frame,
			1.f / sets * set
		},
		{
			1.f / frames * (frame + 1),
			1.f / sets * (set + 1)
		}
	};
}

const unsigned char* jactorio::data::Sprite::get_sprite_data_ptr() const {
	return sprite_buffer_;
}

void jactorio::data::Sprite::set_sprite_data_ptr(unsigned char* sprite_data, const unsigned sprite_width,
                                                 const unsigned sprite_height) {
	width_ = sprite_width;
	height_ = sprite_height;
	sprite_buffer_ = sprite_data;
}

unsigned jactorio::data::Sprite::get_width() const {
	return width_;
}

unsigned jactorio::data::Sprite::get_height() const {
	return height_;
}

jactorio::data::Sprite* jactorio::data::Sprite::load_image(const std::string& image_path) {
	sprite_path_ = core::filesystem::resolve_path("~/data/" + image_path);
	load_image_from_file();

	return this;
}
