#include "renderer/gui/gui_menus.h"

#include <functional>
#include <imgui/imgui.h>
#include <sstream>

#include "data/data_manager.h"
#include "game/input/mouse_selection.h"
#include "game/player/player_manager.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/item/recipe_group.h"
#include "renderer/rendering/renderer.h"
#include "game/logic/inventory_controller.h"
#include "renderer/gui/gui_colors.h"

constexpr float inventory_slot_width = 32.f;
constexpr float inventory_slot_padding = 6.f;

/**
 * In order to auto resize to fit the title's text since the title is not accounted
 * Pad the ingredients: text with trailing whitespace to reach the length of the title
 */
void fit_title(std::stringstream& description_ss, const uint16_t target_len) {
	while (description_ss.str().size() < target_len)
		description_ss << " ";
}

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
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, J_GUI_COL_TOOLTIP_TITLE_BG);
	ImGui::PushStyleColor(ImGuiCol_TitleBg, J_GUI_COL_TOOLTIP_TITLE_BG);
	ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_TOOLTIP_TITLE_TEXT);

	ImGui::Begin(title, nullptr, flags);
	ImGui::PopStyleColor();  // Pop the black text
	ImGui::Text("%s", description);

	draw_func();
	ImGui::End();

	ImGui::PopStyleColor(2);

	// This window is always in front
	ImGui::SetWindowFocus(title);
}

/**
 * Auto positions slots based on provided specifications, draws using provided callback <br>
 * Call within a ImGui window
 * @param slot_span Slots before wrapping onto new line
 * @param slot_count Number of slots to draw
 * @param slot_scale Multiplier to scale each slot by
 * @param draw_func Draws slot (index)
 */
void draw_slots(const uint8_t slot_span, const uint16_t slot_count, const uint8_t slot_scale,
                const std::function<void(uint16_t)>& draw_func) {
	// If all the slots are drawn without a newline, add one
	bool printed_newline = false;

	uint16_t index = 0;
	while (index < slot_count) {
		const uint16_t x = index % slot_span;
		ImGui::SameLine(10.f + x * (inventory_slot_width + inventory_slot_padding) * slot_scale);
		ImGui::PushID(index);  // Uniquely identifies the button

		// Do user defined stuff per slot here
		draw_func(index);

		ImGui::PopID();
		// Newlines corresponds to scale
		if (x == slot_span - 1) {
			printed_newline = true;
			for (int i = 0; i < slot_scale; ++i) {
				ImGui::NewLine();
			}
		}

		++index;
	}

	if (!printed_newline)
		for (int i = 0; i < slot_scale; ++i) {
			ImGui::NewLine();
		}
}

/**
 * Draws a inventory slot
 * @param menu_data
 * @param l_offset How many slots to offset from the left
 * @param sprite_iid Internal id of the sprite to be drawn
 * @param item_count Number to display on the item, 0 to hide
 * @param button_event_func Register events with the button click
 */
void draw_slot(const jactorio::renderer::imgui_manager::Character_menu_data& menu_data,
               const uint16_t l_offset,
               const uint32_t sprite_iid,
               const uint16_t item_count,
               const std::function<void()>& button_event_func = [](){}) {
	using namespace jactorio;
	
	// TODO A better, accurrate calculate of the offset
	ImGui::SameLine(10.f + l_offset * (inventory_slot_width + inventory_slot_padding));

	const auto& uv = menu_data.sprite_positions.at(sprite_iid);
	ImGui::ImageButton(
		reinterpret_cast<void*>(menu_data.tex_id),
		ImVec2(inventory_slot_width, inventory_slot_width),

		ImVec2(uv.top_left.x, uv.top_left.y),
		ImVec2(uv.bottom_right.x, uv.bottom_right.y),
		2
	);

	button_event_func();

	// Total raw count
	if (item_count != 0) {
		ImGui::SameLine(10.f + l_offset * (inventory_slot_width + inventory_slot_padding));
		ImGui::Text("%d", item_count);
	}
}

void jactorio::renderer::gui::character_menu(const ImGuiWindowFlags window_flags,
                                             const imgui_manager::Character_menu_data& menu_data) {
	namespace player_manager = game::player_manager;

	data::item_stack* inventory = player_manager::inventory_player;

	const auto window_size = 
		ImVec2(20 + 10 * (inventory_slot_width + inventory_slot_padding),
		       player_manager::inventory_size / 10 * (inventory_slot_width + inventory_slot_padding) + 80);

	// Uses pixel coordinates, top left is 0, 0, bottom right x, x
	// Character window is left of the center
	const ImVec2 window_center(Renderer::get_window_width() / 2, Renderer::get_window_height() / 2);
	
	ImGui::SetNextWindowPos(ImVec2(window_center.x - window_size.x, window_center.y - window_size.y / 2));
	ImGui::Begin("Character", nullptr, window_size, -1, window_flags);

	draw_slots(10, player_manager::inventory_size, 1, [&](auto index) {
		const auto& item = inventory[index];

		// Item exists at inventory slot?
		if (item.first != nullptr) {
			draw_slot(menu_data, index % 10, item.first->sprite->internal_id, item.second, [index, item]() {
				if (ImGui::IsItemClicked()) {
					player_manager::inventory_click(index, 0);
					player_manager::inventory_sort();
				}
				else if (ImGui::IsItemClicked(1)) {
					player_manager::inventory_click(index, 1);
					player_manager::inventory_sort();
				}

				// Only draw tooltip + item count if item count is not 0
				if (ImGui::IsItemHovered() && item.second != 0) {
					draw_cursor_tooltip(
						item.first->get_localized_name().c_str(),
						"sample description",
						[&]() {
							ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_NONE);
							ImGui::TextUnformatted(item.first->get_localized_name().c_str());
							ImGui::PopStyleColor();
						}
					);
				}
			});
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
				player_manager::inventory_click(index, 0);
				player_manager::inventory_sort();
			}
			else if (ImGui::IsItemClicked(1)) {
				player_manager::inventory_click(index, 1);
				player_manager::inventory_sort();
			}
		}
	});

	ImGui::End();

	// Recipe menu
	ImGui::SetNextWindowPos(ImVec2(window_center.x, window_center.y - window_size.y / 2));
	ImGui::Begin("Recipe", nullptr, window_size, -1, window_flags);

	// Menu groups | A group button is twice the size of a slot
	auto groups = data::data_manager::data_raw_get_all_sorted<data::Recipe_group>(data::data_category::recipe_group);
	draw_slots(5, groups.size(), 2, [&](const uint16_t index) {
		const auto& recipe_group = groups[index];

		const auto& positions = menu_data.sprite_positions.at(recipe_group->sprite->internal_id);

		// Different color for currently selected recipe group
		if (index == player_manager::recipe_group_get_selected())
			ImGui::PushStyleColor(ImGuiCol_Button, J_GUI_COL_BUTTON_HOVER);
		
		ImGui::ImageButton(
			reinterpret_cast<void*>(menu_data.tex_id),
			ImVec2(inventory_slot_width * 2, inventory_slot_width * 2),

			ImVec2(positions.top_left.x, positions.top_left.y),
			ImVec2(positions.bottom_right.x, positions.bottom_right.y),
			2
		);
		
		if (index == player_manager::recipe_group_get_selected())
			ImGui::PopStyleColor();

		// Recipe group click
		if (ImGui::IsItemClicked())
			player_manager::recipe_group_select(index);

		// Item tooltip
		std::stringstream description_ss;
		description_ss << recipe_group->get_localized_description().c_str();
		fit_title(description_ss, recipe_group->get_localized_name().size());

		if (ImGui::IsItemHovered()) {
			draw_cursor_tooltip(
				recipe_group->get_localized_name().c_str(),
				description_ss.str().c_str(),
				[&]() {
				}
			);
		}
	});

	// Menu recipes
	const auto& selected_group = groups[player_manager::recipe_group_get_selected()];
	for (auto& recipe_category : selected_group->recipe_categories) {
		const auto& recipes = recipe_category->recipes;
		
		draw_slots(10, recipes.size(), 1, [&](auto index) {
			data::Recipe* recipe = recipes.at(index);

			
			const auto product = 
				data::data_manager::data_raw_get<data::Item>(data::data_category::item, recipe->get_product().first);
			assert(product != nullptr);  // Invalid recipe product
			
			const auto& uv = menu_data.sprite_positions.at(product->sprite->internal_id);
			ImGui::ImageButton(
				reinterpret_cast<void*>(menu_data.tex_id),
				ImVec2(inventory_slot_width, inventory_slot_width),

				ImVec2(uv.top_left.x, uv.top_left.y),
				ImVec2(uv.bottom_right.x, uv.bottom_right.y),
				2
			);

			// Click event
			if (ImGui::IsItemClicked()) {
				LOG_MESSAGE_f(debug, "Recipe click at index %d in category", index);
				if (player_manager::recipe_can_craft(recipe, 1)) {
					player_manager::recipe_craft_r(recipe);
					player_manager::inventory_sort();
				}
			}

			// Draw item tooltip
			if (!ImGui::IsItemHovered())
				return;

			std::stringstream title_ss;
			// Show the product yield in the title
			title_ss << product->get_localized_name().c_str() << " (" << recipe->get_product().second << ")";

			std::stringstream description_ss;
			description_ss << "Ingredients:";
			fit_title(description_ss, title_ss.str().size());

			draw_cursor_tooltip(
				title_ss.str().c_str(),
				description_ss.str().c_str(),
				[&]() {
					// Draw ingredients
					for (const auto& ingredient_pair : recipe->ingredients) {
						ImGui::NewLine();
						const auto* item =
							data::data_manager::data_raw_get<data::Item>(data::data_category::item,
							                                             ingredient_pair.first);
						
						draw_slot(menu_data, 0, item->sprite->internal_id, 0);

						// Amount of the current ingredient the player has in inventory
						const auto player_item_count = game::inventory_c::get_inv_item_count(
							player_manager::inventory_player, player_manager::inventory_size, 
							item);
						
						// Draw ingredient amount required
						ImGui::SameLine();
						// Does not have ingredient
						if (player_item_count < ingredient_pair.second) {
							const bool can_be_recurse_crafted = player_manager::recipe_can_craft(recipe, 1);
							if (can_be_recurse_crafted) {
								// Ingredient can be crafted recursively
								ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_TEXT_WARNING);
							}
							else {
								// Ingredient cannot be crafted
								ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_TEXT_ERROR);
							}

							ImGui::Text("%d/%d x %s", player_item_count, ingredient_pair.second,
							            item->get_localized_name().c_str());
							ImGui::PopStyleColor();
						}
							// Has enough
						else {
							ImGui::Text("%d x %s", ingredient_pair.second, 
							            item->get_localized_name().c_str());
						}
					}
					ImGui::Text("%.1f seconds", recipe->crafting_time);
				
					// Total raw
					ImGui::Separator();
					ImGui::Text("%s", "Total\nRaw");
					auto raw_inames = data::Recipe::recipe_get_total_raw(product->name);
					draw_slots(5, raw_inames.size(), 1, [&](const auto index) {
						const auto* item =
							data::data_manager::data_raw_get<data::Item>(data::data_category::item,
							                                             raw_inames[index].first);
						draw_slot(menu_data, index + 1,
						          item->sprite->internal_id, raw_inames[index].second);
					});
				});
		});

	}

	ImGui::End();

}

void jactorio::renderer::gui::cursor_window(const imgui_manager::Character_menu_data& menu_data) {
	using namespace jactorio;
	// Draw the tooltip of what is currently selected

	// Player has an item selected, draw it on the tooltip
	const data::item_stack* selected_item;
	if ((selected_item = game::player_manager::get_selected_item()) != nullptr) {
		ImGui::PushStyleColor(ImGuiCol_Border, J_GUI_COL_NONE);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, J_GUI_COL_NONE);

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

		const auto& positions = menu_data.sprite_positions.at(selected_item->first->sprite->internal_id);

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

void jactorio::renderer::gui::crafting_queue(const imgui_manager::Character_menu_data& menu_data) {
	ImGuiWindowFlags flags = 0;
	flags |= ImGuiWindowFlags_NoBackground;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoResize;

	// TODO window size and positioning calculation right
	ImGui::SetNextWindowPos(
		ImVec2(0, Renderer::get_window_height() - (inventory_slot_width + inventory_slot_padding)));

	const ImVec2 window_size(11 * (inventory_slot_padding + inventory_slot_width),
	                         inventory_slot_width + 2 * inventory_slot_padding);
	
	ImGui::Begin("queue", nullptr, window_size, -1, flags);
	ImGui::PushStyleColor(ImGuiCol_Button, J_GUI_COL_NONE);
	ImGui::PushStyleColor(ImGuiCol_Border, J_GUI_COL_NONE);

	auto& recipe_queue = game::player_manager::get_recipe_queue();
	draw_slots(10, recipe_queue.size(), 1, [&](auto index) {
		data::Recipe* recipe = recipe_queue.at(index);
		
		const auto* item =
			data::data_manager::data_raw_get<data::Item>(data::data_category::item,
			                                             recipe->get_product().first);
		draw_slot(menu_data, index % 10, item->sprite->internal_id, 1);
	});
	
	ImGui::PopStyleColor(2);
	ImGui::End();
}
