#include <gtest/gtest.h>

#include "renderer/rendering/renderer_sprites.h"

TEST(renderer_sprites, gen_spritemap) {
	// Provide series of sprites in array
	// Expect concatenated image and its properties

	// Spritemap will be purely horizontal, all sprites concatenated side by side


	char* arr1 = new char[25] {
		'a', 'a', 'a', 'a', 'a', 
		'b', 'b', 'b', 'b', 'b', 
		'c', 'c', 'c', 'c', 'c', 
		'd', 'd', 'd', 'd', 'd', 
		'e', 'e', 'e', 'e', 'e', 
	};
	
	char* arr2 = new char[35] {
		'f', 'f', 'f', 'f', 'f', 'f', 'f', 
		'g', 'g', 'g', 'g', 'g', 'g', 'g', 
		'h', 'h', 'h', 'h', 'h', 'h', 'h',
		'i', 'i', 'i', 'i', 'i', 'i', 'i', 
		'j', 'j', 'j', 'j', 'j', 'j', 'j', 
	};

	char* arr3 = new char[25] {
		'k', 'k', 'k', 'k', 'k', 
		'l', 'l', 'l', 'l', 'l', 
		'm', 'm', 'm', 'm', 'm', 
		'n', 'n', 'n', 'n', 'n', 
		'o', 'o', 'o', 'o', 'o',
	};

	char* arr4 = new char[25] {
		'p', 'p', 'p', 'p', 'p', 
		'q', 'q', 'q', 'q', 'q', 
		'r', 'r', 'r', 'r', 'r', 
		's', 's', 's', 's', 's', 
		't', 't', 't', 't', 't', 
	};

	char* arr5 = new char[30] {
		'u', 'u', 'u', 'u', 'u', 
		'v', 'v', 'v', 'v', 'v', 
		'w', 'w', 'w', 'w', 'w', 
		'x', 'x', 'x', 'x', 'x', 
		'y', 'y', 'y', 'y', 'y',
		'z', 'z', 'z', 'z', 'z',
	};
	
	const auto sprites = new jactorio::renderer::Renderer_sprites::Sprite[3] {
		{arr1, 5, 5},
		{arr2, 7, 5},
		{arr5, 5, 6},
	};

	auto spritemap = jactorio::renderer::Renderer_sprites::gen_spritemap(sprites, 3);
	delete[] sprites;

	
}