#include "renderer/gui/gui_menus.h"

#include <functional>
#include <imgui/imgui.h>

#include "data/data_manager.h"
#include "game/input/mouse_selection.h"
#include "game/player/player_manager.h"
#include "data/prototype/entity/entity.h"

constexpr float inventory_slot_width = 32.f;
constexpr float inventory_slot_padding = 6.f;

/**
 * @param title Title of the tooltip
 * @param description
 * @param draw_func Code to run while drawing the tooltip
 */
void draw_cursor_tooltip(const char* title, const char* description, const std::function<void()>& draw_func) {
	using namespace jactorio;

	ImVec2 cursor_pos(
		game::mouse_selection::get_position_x(),
		game::mouse_selection::get_position_y() + 10.f
	);
	// If an item is currently selected, move the tooltip down to not overlap
	if (game::player_manager::get_selected_item())
		cursor_pos.y += inventory_slot_width;

	ImGui::SetNextWindowPos(cursor_pos);


	ImGuiWindowFlags flags = 0;
	flags |= ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoInputs;
	flags |= ImGuiWindowFlags_NoScrollbar;
	flags |= ImGuiWindowFlags_AlwaysAutoResize;

	// Draw tooltip
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, IM_COL32(224, 202, 169, 255));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, IM_COL32(224, 202, 169, 255));
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(49, 48, 49, 255));
	
	ImGui::Begin(title, nullptr, flags);
	ImGui::PopStyleColor();  // Pop the black text
	ImGui::Text("%s", description);

	draw_func();
	ImGui::End();
	
	ImGui::PopStyleColor(2);
}

void jactorio::renderer::gui::character_menu(const ImGuiWindowFlags window_flags,
                                             const imgui_manager::Character_menu_data& menu_data) {
	namespace player_manager = game::player_manager;

	// ImGui::SetNextWindowPosCenter();

	data::item_stack* inventory = player_manager::player_inventory;

	ImGui::Begin("Character", nullptr,
	             ImVec2(20 + 10 * (inventory_slot_width + inventory_slot_padding),
	                    player_manager::inventory_size / 10 * (inventory_slot_width +
		                    inventory_slot_padding) + 80),
	             -1, window_flags);

	int index = 0;
	do {
		const int x = index % 10;
		ImGui::SameLine(10.f + x * (inventory_slot_width + inventory_slot_padding));

		ImGui::PushID(index);  // Uniquely identifies the button

		const auto& item = inventory[index];

		// Item exists at inventory slot?
		if (item.first != nullptr) {
			const auto& positions = menu_data.sprite_positions.at(item.first->sprite->internal_id);

			ImGui::ImageButton(
				reinterpret_cast<void*>(menu_data.tex_id),
				ImVec2(inventory_slot_width, inventory_slot_width),

				ImVec2(positions.top_left.x, positions.top_left.y),
				ImVec2(positions.bottom_right.x, positions.bottom_right.y),
				2
			);

			// Click event
			if (ImGui::IsItemClicked()) {
				player_manager::set_clicked_inventory(index, 0);
			}
			else if (ImGui::IsItemClicked(1)) {
				player_manager::set_clicked_inventory(index, 1);
			}

			// Only draw tooltip + item count if item count is not 0
			if (item.second != 0) {
				// Item tooltip
				if (ImGui::IsItemHovered()) {
					const auto entity_ptr = static_cast<data::Entity*>(item.first->entity_prototype);
					
					draw_cursor_tooltip(
						entity_ptr->localized_name.c_str(),
						"sample description",
						[&]() {
							ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 0));
							ImGui::TextUnformatted(entity_ptr->localized_name.c_str());
							ImGui::PopStyleColor();
						}
					);
				}

				// Stack size
				ImGui::SameLine(10.f + x * (inventory_slot_width + inventory_slot_padding));
				ImGui::Text("%d", item.second);
			}

		}
		else {
			// Empty button
			ImGui::ImageButton(
				nullptr,
				ImVec2(0, 0),
				ImVec2(-1, -1),
				ImVec2(-1, -1),
				inventory_slot_width / 2 + 2 // 32 / 2 + 2
			);
			// Click event
			if (ImGui::IsItemClicked()) {
				player_manager::set_clicked_inventory(index, 0);
			}
			else if (ImGui::IsItemClicked(1)) {
				player_manager::set_clicked_inventory(index, 1);
			}
		}

		ImGui::PopID();

		if (x == 9)
			ImGui::NewLine();
	} while (++index < player_manager::inventory_size);

	ImGui::End();

	recipe_menu(window_flags, menu_data);
	
}

void jactorio::renderer::gui::recipe_menu(const ImGuiWindowFlags window_flags,
                                          const imgui_manager::Character_menu_data& menu_data) {
	namespace player_manager = game::player_manager;

	ImGui::Begin("Recipe", nullptr,
	             ImVec2(20 + 10 * (inventory_slot_width + inventory_slot_padding),
	                    player_manager::inventory_size / 10 * (inventory_slot_width +
		                    inventory_slot_padding) + 80),
	             -1, window_flags);

	// Menu categories | A category button is twice the size of a slot
	int index = 0;
	do {
		const int x = index % 5;
		ImGui::SameLine(10.f + x * (inventory_slot_width * 2 + inventory_slot_padding));

		ImGui::PushID(index);  // Uniquely identifies the button

		// const auto& item = inventory[index];
		// const auto& positions = menu_data.sprite_positions[item.first->sprite->internal_id];

		ImGui::ImageButton(
			reinterpret_cast<void*>(menu_data.tex_id),
			ImVec2(inventory_slot_width * 2, inventory_slot_width * 2),

			ImVec2(0, 0),
			ImVec2(1, 1),
			2
		);

		// Click event
		if (ImGui::IsItemClicked()) {
			player_manager::set_clicked_inventory(index, 0);
		}

		// Item tooltip
		if (ImGui::IsItemHovered()) {
			draw_cursor_tooltip(
				"Category",
				"it's a category!",
				[&]() {
				}
			);
		}
		ImGui::PopID();

		if (x == 4) { // 5 category buttons per line
			ImGui::NewLine();
			ImGui::NewLine();
		}
	} while (++index < 5);
	
	// Menu items
	index = 0;
	do {
		const int x = index % 10;
		ImGui::SameLine(10.f + x * (inventory_slot_width + inventory_slot_padding));

		ImGui::PushID(index);  // Uniquely identifies the button

		// const auto& item = inventory[index];
		// const auto& positions = menu_data.sprite_positions[item.first->sprite->internal_id];

		ImGui::ImageButton(
			reinterpret_cast<void*>(menu_data.tex_id),
			ImVec2(inventory_slot_width, inventory_slot_width),

			ImVec2(0, 0),
			ImVec2(1, 1),
			2
		);

		// Click event
		if (ImGui::IsItemClicked()) {
			player_manager::set_clicked_inventory(index, 0);
		}

		// Item tooltip
		if (ImGui::IsItemHovered()) {
			draw_cursor_tooltip(
				"???",
				"sample description",
				[&]() {
				}
			);
		}
		ImGui::PopID();

		if (x == 9)
			ImGui::NewLine();
	} while (++index < player_manager::inventory_size);

	ImGui::End();
}

void jactorio::renderer::gui::cursor_window(ImGuiWindowFlags window_flags,
                                            imgui_manager::Character_menu_data menu_data) {
	using namespace jactorio;
	// Draw the tooltip of what is currently selected

	// Player has an item selected, draw it on the tooltip
	const data::item_stack* selected_item;
	if ((selected_item = game::player_manager::get_selected_item()) != nullptr) {
		ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(0, 0, 0, 0));

		// Draw the window at the cursor
		const ImVec2 cursor_pos(
			game::mouse_selection::get_position_x(),
			game::mouse_selection::get_position_y() + 2.f
		);
		ImGui::SetNextWindowPos(cursor_pos);

		ImGuiWindowFlags flags = 0;
		flags |= ImGuiWindowFlags_NoBackground;
		flags |= ImGuiWindowFlags_NoTitleBar;
		flags |= ImGuiWindowFlags_NoCollapse;
		flags |= ImGuiWindowFlags_NoMove;
		flags |= ImGuiWindowFlags_NoResize;

		ImGui::SetNextWindowFocus();
		ImGui::Begin("Selected-item", nullptr, flags);

		const auto& positions = menu_data.sprite_positions[selected_item->first->sprite->internal_id];

		ImGui::SameLine(10.f);
		ImGui::Image(
			reinterpret_cast<void*>(menu_data.tex_id),
			ImVec2(32, 32),

			ImVec2(positions.top_left.x, positions.top_left.y),
			ImVec2(positions.bottom_right.x, positions.bottom_right.y)
		);

		ImGui::SameLine(10.f);
		ImGui::Text("%d", selected_item->second);

		ImGui::End();
		ImGui::PopStyleColor(2);

	}
}
