// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_GUI_IMGUI_MANAGER_H
#define JACTORIO_INCLUDE_RENDERER_GUI_IMGUI_MANAGER_H
#pragma once

#include "game/event/event.h"
#include "game/player/player_data.h"
#include "renderer/rendering/spritemap_generator.h"

namespace jactorio::renderer
{
	class DisplayWindow;


	// If true, ImGui has handled the a input event and thus should not be carried to down the layer
	inline bool input_mouse_captured    = false;
	inline bool input_keyboard_captured = false;


	struct MenuData
	{
		MenuData(const SpriteUvCoordsT& sprite_positions,
		         const unsigned tex_id)
			: spritePositions(sprite_positions),
			  texId(tex_id) {
		}

		const SpriteUvCoordsT& spritePositions;
		unsigned int texId = 0;  // Assigned by openGL
	};

	///
	/// \brief Initializes the spritemap for rendering the character menus <br>
	/// \remark Requires Sprite::sprite_group::gui to be initialized
	void SetupCharacterData(RendererSprites& renderer_sprites);
	J_NODISCARD MenuData GetMenuData();

	void Setup(const DisplayWindow& display_window);

	void ImguiDraw(const DisplayWindow& display_window,
	               game::PlayerData& player_data, const data::PrototypeManager& data_manager, game::EventData& event);

	void ImguiTerminate();
}

#endif //JACTORIO_INCLUDE_RENDERER_GUI_IMGUI_MANAGER_H
