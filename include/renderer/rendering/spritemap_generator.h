// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERING_SPRITEMAP_GENERATOR_H
#define JACTORIO_INCLUDE_RENDERER_RENDERING_SPRITEMAP_GENERATOR_H
#pragma once

#include <unordered_map>

#include "core/data_type.h"
#include "data/prototype/sprite.h"
#include "renderer/opengl/texture.h"

// Generates spritemaps on initialization with tile sprites
// - Concatenate sprite into spritemap
// - Location of a sprite within spritemap
namespace jactorio::renderer::renderer_sprites
{
	struct Spritemap_data
	{
		// For the loaded sprite
		unsigned char* sprite_buffer = nullptr;
		unsigned int width = 0;
		unsigned int height = 0;

		// Image positions retrieved via the path originally given to create the spritemap
		// 0 - 1 positions of the sprite within the spritemap
		// Upper left is 0, 0 - bottom right is 1, 1
		// std::string is internal name of prototype
		std::unordered_map<unsigned int, core::Quad_position> sprite_positions;
	};

	/**
	 * Frees all spritemap memory
	 */
	void clear_spritemaps();

	/**
	 * Creates a spritemap and stores it as a renderer::Texture
	 */
	void create_spritemap(data::Sprite::spriteGroup group, bool invert_sprites);

	/**
	 * Retrieves spritemap at specified group
	 */
	const Spritemap_data& get_spritemap(data::Sprite::spriteGroup group);
	const Texture* get_texture(data::Sprite::spriteGroup group);

	/*!
	 * Generated spritemap will be purely horizontal, all images concatenated side by side <br>
	 * Color in non specified areas of the spritemap are undefined <br>
	 * !!Spritemap pointer must be manually deleted
	 * @param sprites Pointer array to pointers towards sprite prototypes
	 * @param count Count of pointer array
	 * @param invert_sprites Whether or not to vertically invert the sprites on the spritemap. Commonly done for OpenGL
	 */
	Spritemap_data gen_spritemap(data::Sprite** sprites, unsigned short count, bool invert_sprites);
}

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_SPRITEMAP_GENERATOR_H
