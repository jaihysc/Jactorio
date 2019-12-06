#define STB_IMAGE_IMPLEMENTATION

#include "data/prototype/sprite.h"

#include <stb/stb_image.h>

#include "core/filesystem.h"
#include "core/logger.h"

void jactorio::data::Sprite::load_image_from_file() {
	// openGL reads images vertically flipped, bottom left is (0,0)
	stbi_set_flip_vertically_on_load(true);
	sprite_buffer_ = stbi_load(
		sprite_path_.c_str(),
		&width_,
		&height_,
		&bytes_per_pixel_,
		4  // 4 desired channels for RGBA
	);

	if (!sprite_buffer_) {
		LOG_MESSAGE_f(error, "Failed to read texture %s", sprite_path_.c_str());
	}
}

jactorio::data::Sprite::Sprite()
	: width_(0), height_(0), bytes_per_pixel_(0), sprite_buffer_(nullptr) {
}

jactorio::data::Sprite::Sprite(const std::string& sprite_path)
	: width_(0), height_(0), bytes_per_pixel_(0), sprite_buffer_(nullptr) {
	load_image(sprite_path);
}

jactorio::data::Sprite::~Sprite() {
	delete[] sprite_buffer_;
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

void jactorio::data::Sprite::load_image(const std::string& image_path) {
	sprite_path_ = core::filesystem::resolve_path("~/data/" + image_path);
	load_image_from_file();
}
