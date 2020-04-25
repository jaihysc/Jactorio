// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/01/2020

#include "renderer/gui/gui_menus.h"

#include <functional>
#include <sstream>

#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/item/recipe_group.h"

#include "game/input/mouse_selection.h"
#include "game/logic/inventory_controller.h"
#include "game/player/player_data.h"

#include "renderer/gui/gui_colors.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/rendering/renderer.h"

constexpr unsigned int inventory_slot_width = 36;
constexpr unsigned int inventory_slot_padding = 3;

/**
 * In order to auto resize to fit the title's text since the title is not accounted
 * Pad the ingredients: text with trailing whitespace to reach the length of the title
 */
void fit_title(std::stringstream& description_ss, const uint16_t target_len) {
	while (description_ss.str().size() < target_len)
		description_ss << " ";
}

///
/// \param title Title of the tooltip
/// \param description
/// \param draw_func Code to run while drawing the tooltip
void draw_cursor_tooltip(jactorio::game::Player_data& player_data, const char* title, const char* description,
                         const std::function<void()>& draw_func) {
	using namespace jactorio;

	ImVec2 cursor_pos(
		static_cast<float>(game::Mouse_selection::get_cursor_x()),
		static_cast<float>(game::Mouse_selection::get_cursor_y() + 10.f)
	);
	// If an item is currently selected, move the tooltip down to not overlap
	if (player_data.get_selected_item())
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

///
/// Auto positions slots based on provided specifications, draws using provided callback <br>
/// Call within a ImGui window
/// \param slot_span Slots before wrapping onto new line
/// \param slot_count Number of slots to draw
/// \param draw_func Draws slot (index)
void draw_slots(const uint8_t slot_span, const uint16_t slot_count,
                const std::function<void(uint16_t)>& draw_func) {
	// If all the slots are drawn without a newline, add one
	bool printed_newline = false;

	uint16_t index = 0;
	while (index < slot_count) {
		const uint16_t x = index % slot_span;
		ImGui::SameLine(J_GUI_STYLE_WINDOW_PADDING_X + x * (inventory_slot_width + inventory_slot_padding));
		ImGui::PushID(index);  // Uniquely identifies the button

		// Do user defined stuff per slot here
		draw_func(index);

		ImGui::PopID();
		// Newlines corresponds to scale
		if (x == slot_span - 1) {
			printed_newline = true;
			ImGui::NewLine();
		}

		++index;
	}

	if (!printed_newline)
		ImGui::NewLine();
}

/**
 * Draws a inventory slot
/// \param menu_data
/// \param scale
/// \param l_offset How many slots to offset from the left
/// \param sprite_iid Internal id of the sprite to be drawn
/// \param item_count Number to display on the item, 0 to hide
/// \param button_event_func Register events with the button click
 */
void draw_slot(const jactorio::renderer::imgui_manager::Menu_data& menu_data,
               const uint8_t scale,
               const uint16_t l_offset,
               const uint32_t sprite_iid,
               const uint16_t item_count,
               const std::function<void()>& button_event_func = []() {
               }) {
	using namespace jactorio;

	// Padding around the image in a slot
	// Imgui padding is additive around the slot, therefore size must be subtracted to maintain the same dimensions
	constexpr unsigned int image_padding = 2;


	const unsigned int button_size =
		scale * inventory_slot_width
		+ (scale - 1) * inventory_slot_padding  // To align with other scales, account for the padding between slots
		- 2 * image_padding;


	ImGui::SameLine(J_GUI_STYLE_WINDOW_PADDING_X
		+ l_offset * (scale * (inventory_slot_width + inventory_slot_padding)));

	const auto& uv = menu_data.sprite_positions.at(sprite_iid);
	ImGui::ImageButton(
		reinterpret_cast<void*>(menu_data.tex_id),
		ImVec2(
			static_cast<float>(button_size),
			// I do not know why this happens, but buttons are off by 1 pixel for each scale level 
			static_cast<float>(button_size + (scale - 1))),

		ImVec2(uv.top_left.x, uv.top_left.y),
		ImVec2(uv.bottom_right.x, uv.bottom_right.y),
		image_padding
	);

	button_event_func();

	// Total raw count
	if (item_count != 0) {
		ImGui::SameLine(J_GUI_STYLE_WINDOW_PADDING_X
			+ l_offset * (inventory_slot_width + inventory_slot_padding));
		ImGui::Text("%d", item_count);
	}
}

/**
 * Draws empty inventory slot
 */
void draw_empty_slot() {
	ImGui::ImageButton(
		nullptr,
		ImVec2(0, 0),
		ImVec2(-1, -1),
		ImVec2(-1, -1),
		inventory_slot_width / 2 // 32 / 2
	);
}

// ======================================================================
// Window positioning

///
/// \brief The window size is calculated on the size of the player's inventory
ImVec2 get_window_size(jactorio::game::Player_data& /*player_data*/) {
	// 20 is window padding on both sides, 80 for y is to avoid the scrollbar
	auto window_size = ImVec2(
		2 * J_GUI_STYLE_WINDOW_PADDING_X,
		2 * J_GUI_STYLE_WINDOW_PADDING_Y + 80);

	window_size.x += 10 * (inventory_slot_width + inventory_slot_padding) - inventory_slot_padding;
	window_size.y += static_cast<unsigned int>(jactorio::game::Player_data::inventory_size / 10) *
		(inventory_slot_width + inventory_slot_padding) - inventory_slot_padding;

	return window_size;
}

///
/// The next window drawn will be on the left center of the screen
void setup_next_window_left(const ImVec2& window_size) {
	// Uses pixel coordinates, top left is 0, 0, bottom right x, x
	// Character window is left of the center
	const ImVec2 window_center(static_cast<float>(jactorio::renderer::Renderer::get_window_width()) / 2,
	                           static_cast<float>(jactorio::renderer::Renderer::get_window_height()) / 2);

	ImGui::SetNextWindowPos(ImVec2(window_center.x - window_size.x,
	                               window_center.y - window_size.y / 2));
}

///
/// The next window drawn will be on the left center of the screen
void setup_next_window_right(const ImVec2& window_size) {
	// Uses pixel coordinates, top left is 0, 0, bottom right x, x
	// Character window is left of the center
	const ImVec2 window_center(static_cast<float>(jactorio::renderer::Renderer::get_window_width()) / 2,
	                           static_cast<float>(jactorio::renderer::Renderer::get_window_height()) / 2);

	// Recipe menu
	ImGui::SetNextWindowPos(ImVec2(window_center.x,
	                               window_center.y - window_size.y / 2));

	ImGui::SetNextWindowSize(window_size);
}

///
/// \brief Draws the player's inventory menu
void player_inventory_menu(jactorio::game::Player_data& player_data) {
	const ImVec2 window_size = get_window_size(player_data);
	setup_next_window_left(window_size);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize;

	ImGui::SetNextWindowSize(window_size);
	ImGui::Begin("Character", nullptr, window_flags);

	auto menu_data = jactorio::renderer::imgui_manager::get_menu_data();
	draw_slots(10, jactorio::game::Player_data::inventory_size, [&](auto index) {
		const auto& item = player_data.inventory_player[index];

		// Item exists at inventory slot?
		if (item.first != nullptr) {
			draw_slot(menu_data, 1, index % 10, item.first->sprite->internal_id, item.second,
			          [&]() {
				          if (ImGui::IsItemClicked()) {
					          player_data.inventory_click(
						          index, 0, true, player_data.inventory_player);
					          player_data.inventory_sort();
				          }
				          else if (ImGui::IsItemClicked(1)) {
					          player_data.inventory_click(
						          index, 1, true, player_data.inventory_player);
					          player_data.inventory_sort();
				          }

				          // Only draw tooltip + item count if item count is not 0
				          if (ImGui::IsItemHovered() && item.second != 0) {
					          draw_cursor_tooltip(
						          player_data,
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
			draw_empty_slot();
			// Click event
			if (ImGui::IsItemClicked()) {
				player_data.inventory_click(
					index, 0, true, player_data.inventory_player);
				player_data.inventory_sort();
			}
			else if (ImGui::IsItemClicked(1)) {
				player_data.inventory_click(
					index, 1, true, player_data.inventory_player);
				player_data.inventory_sort();
			}
		}
	});

	ImGui::End();
}

// ==========================================================================================
// Player menus (Excluding entity menus)

const ImGuiWindowFlags menu_flags = 0 | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;


void jactorio::renderer::gui::character_menu(game::Player_data& player_data, const data::Unique_data_base*) {
	player_inventory_menu(player_data);


	auto menu_data = imgui_manager::get_menu_data();

	const ImVec2 window_size = get_window_size(player_data);
	setup_next_window_right(window_size);

	ImGui::SetNextWindowSize(window_size);
	ImGui::Begin("Recipe", nullptr, menu_flags);

	// Menu groups | A group button is twice the size of a slot
	auto groups = data::data_raw_get_all_sorted<data::Recipe_group>(data::dataCategory::recipe_group);
	draw_slots(5, groups.size(), [&](const uint16_t index) {
		const auto& recipe_group = groups[index];

		// Different color for currently selected recipe group
		if (index == player_data.recipe_group_get_selected())
			ImGui::PushStyleColor(ImGuiCol_Button, J_GUI_COL_BUTTON_HOVER);

		draw_slot(menu_data, 2, index % 10, recipe_group->sprite->internal_id, 0);

		if (index == player_data.recipe_group_get_selected())
			ImGui::PopStyleColor();

		// Recipe group click
		if (ImGui::IsItemClicked())
			player_data.recipe_group_select(index);

		// Item tooltip
		std::stringstream description_ss;
		description_ss << recipe_group->get_localized_description().c_str();
		fit_title(description_ss, recipe_group->get_localized_name().size());

		if (ImGui::IsItemHovered()) {
			draw_cursor_tooltip(
				player_data,
				recipe_group->get_localized_name().c_str(),
				description_ss.str().c_str(),
				[&]() {
				}
			);
		}
	});

	// Menu recipes
	const auto& selected_group = groups[player_data.recipe_group_get_selected()];
	for (auto& recipe_category : selected_group->recipe_categories) {
		const auto& recipes = recipe_category->recipes;

		draw_slots(10, recipes.size(), [&](auto index) {
			data::Recipe* recipe = recipes.at(index);

			const auto* product =
				data::data_raw_get<data::Item>(data::dataCategory::item, recipe->get_product().first);
			assert(product != nullptr);  // Invalid recipe product

			draw_slot(menu_data, 1, index % 10, product->sprite->internal_id, 0);

			// Click event
			if (ImGui::IsItemClicked()) {
				LOG_MESSAGE_f(debug, "Recipe click at index %d in category", index);
				if (player_data.recipe_can_craft(recipe, 1)) {
					player_data.recipe_craft_r(recipe);
					player_data.inventory_sort();
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
				player_data,
				title_ss.str().c_str(),
				description_ss.str().c_str(),
				[&]() {
					// Draw ingredients
					for (const auto& ingredient_pair : recipe->ingredients) {
						ImGui::NewLine();
						const auto* item =
							data::data_raw_get<data::Item>(data::dataCategory::item,
							                                             ingredient_pair.first);

						draw_slot(menu_data, 1, 0, item->sprite->internal_id, 0);

						// Amount of the current ingredient the player has in inventory
						const auto player_item_count = game::get_inv_item_count(
							player_data.inventory_player, player_data.inventory_size,
							item);

						// Draw ingredient amount required
						ImGui::SameLine();
						// Does not have ingredient
						if (player_item_count < ingredient_pair.second) {
							const bool can_be_recurse_crafted = player_data.recipe_can_craft(recipe, 1);
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
					draw_slots(5, raw_inames.size(), [&](const auto slot_index) {
						const auto* item =
							data::data_raw_get<data::Item>(data::dataCategory::item,
							                                             raw_inames[slot_index].first);
						draw_slot(menu_data, 1, slot_index + 1,
						          item->sprite->internal_id, raw_inames[slot_index].second);
					});
				});
		});

	}

	ImGui::End();

}

void jactorio::renderer::gui::cursor_window(game::Player_data& player_data, const data::Unique_data_base*) {
	using namespace jactorio;
	// Draw the tooltip of what is currently selected

	const auto menu_data = imgui_manager::get_menu_data();

	// Player has an item selected, draw it on the tooltip
	const data::item_stack* selected_item;
	if ((selected_item = player_data.get_selected_item()) != nullptr) {
		ImGui::PushStyleColor(ImGuiCol_Border, J_GUI_COL_NONE);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, J_GUI_COL_NONE);

		// Draw the window at the cursor
		const ImVec2 cursor_pos(
			static_cast<float>(game::Mouse_selection::get_cursor_x()),
			static_cast<float>(game::Mouse_selection::get_cursor_y() + 2.f)
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

void jactorio::renderer::gui::crafting_queue(game::Player_data& player_data, const data::Unique_data_base*) {
	auto menu_data = imgui_manager::get_menu_data();

	ImGuiWindowFlags flags = 0;
	flags |= ImGuiWindowFlags_NoBackground;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoScrollbar;
	flags |= ImGuiWindowFlags_NoScrollWithMouse;

	const auto& recipe_queue = player_data.get_recipe_queue();


	const unsigned int y_slots = (recipe_queue.size() + 10 - 1) / 10;  // Always round up for slot count
	auto y_offset = y_slots * (inventory_slot_width + inventory_slot_padding);

	const unsigned int max_queue_height = Renderer::get_window_height() / 2; // Pixels

	// Clamp to max queue height if greater
	if (y_offset > max_queue_height)
		y_offset = max_queue_height;

	ImGui::SetNextWindowPos(
		ImVec2(0,
		       static_cast<float>(Renderer::get_window_height()) - y_offset
		       - J_GUI_STYLE_WINDOW_PADDING_X));  // Use the x padding to keep it constant on x and y
	ImGui::SetNextWindowSize(
		ImVec2(
			20 + 10 * (inventory_slot_width + inventory_slot_padding) - inventory_slot_padding,
			static_cast<float>(max_queue_height)));

	// Window
	ImGui::Begin("_crafting_queue", nullptr, flags);

	ImGui::PushStyleColor(ImGuiCol_Button, J_GUI_COL_NONE);
	ImGui::PushStyleColor(ImGuiCol_Border, J_GUI_COL_NONE);

	draw_slots(10, recipe_queue.size(), [&](auto index) {
		data::Recipe* recipe;

		// Because of concurrency, the deque may have resized by the same it is indexed
		try {
			recipe = recipe_queue.at(index);
		}
		catch (std::out_of_range&) {
			return;  // Returning from lambda
		}

		const auto* item =
			data::data_raw_get<data::Item>(data::dataCategory::item,
			                                             recipe->get_product().first);
		draw_slot(menu_data, 1, index % 10,
		          item->sprite->internal_id, recipe->get_product().second);
	});

	ImGui::PopStyleColor(2);
	ImGui::End();
}

float last_pickup_fraction = 0.f;

void jactorio::renderer::gui::pickup_progressbar(game::Player_data& player_data, const data::Unique_data_base*) {
	constexpr float progress_bar_width = 260 * 2;
	constexpr float progress_bar_height = 13;

	const float pickup_fraction = player_data.get_pickup_percentage();
	// Do not draw progress bar if 0 or has not moved since last tick
	if (pickup_fraction == 0 || last_pickup_fraction == pickup_fraction)
		return;
	last_pickup_fraction = pickup_fraction;


	ImGuiWindowFlags flags = 0;
	flags |= ImGuiWindowFlags_NoBackground;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoResize;

	ImGui::SetNextWindowSize(ImVec2(progress_bar_width, progress_bar_height));
	ImGui::SetNextWindowPos(
		ImVec2(
			static_cast<float>(Renderer::get_window_width()) / 2 - (progress_bar_width / 2),  // Center X
			static_cast<float>(Renderer::get_window_height()) - progress_bar_height));  // TODO account for hotbar when implemented

	// Window
	ImGui::Begin("_entity_pickup_status", nullptr, flags);

	ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_NONE);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, J_GUI_COL_PROGRESS_BG);
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, J_GUI_COL_PROGRESS);

	ImGui::ProgressBar(pickup_fraction, ImVec2(progress_bar_width, progress_bar_height));

	ImGui::PopStyleColor(3);

	ImGui::End();
}

// ==========================================================================================
// Entity menus
void jactorio::renderer::gui::container_entity(game::Player_data& player_data, const data::Unique_data_base* unique_data) {
	assert(unique_data);
	const auto& container_data = *static_cast<const data::Container_entity_data*>(unique_data);

	player_inventory_menu(player_data);

	const auto window_size = get_window_size(player_data);
	setup_next_window_right(window_size);
	ImGui::Begin("Container", nullptr, menu_flags);

	draw_slots(10, container_data.size, [&](auto i) {
		if (container_data.inventory[i].first == nullptr) {
			draw_empty_slot();
			if (ImGui::IsItemClicked()) {
				player_data.inventory_click(i, 0, false, container_data.inventory);
				player_data.inventory_sort();
			}
			else if (ImGui::IsItemClicked(1)) {
				player_data.inventory_click(i, 1, false, container_data.inventory);
				player_data.inventory_sort();
			}
		}
		else
			draw_slot(
				imgui_manager::get_menu_data(), 1, i % 10,
				container_data.inventory[i].first->sprite->internal_id, container_data.inventory[i].second, [&]() {
					if (ImGui::IsItemClicked()) {
						player_data.inventory_click(i, 0, false, container_data.inventory);
						player_data.inventory_sort();
					}
					else if (ImGui::IsItemClicked(1)) {
						player_data.inventory_click(i, 1, false, container_data.inventory);
						player_data.inventory_sort();
					}
				});
	});

	ImGui::End();
}

void jactorio::renderer::gui::mining_drill(game::Player_data& player_data, const data::Unique_data_base* unique_data) {
	assert(unique_data);
	const auto& drill_data = *static_cast<const data::Mining_drill_data*>(unique_data);

	player_inventory_menu(player_data);

	const auto window_size = get_window_size(player_data);
	setup_next_window_right(window_size);

	ImGui::Begin("Mining drill", nullptr, menu_flags);

	// 1 - (Ticks left / Ticks to mine)
	const long double ticks_left = drill_data.deferral_entry.first - player_data.get_player_world().game_tick();
	const long double mine_ticks = drill_data.mining_ticks;

	ImGui::ProgressBar(1.f - static_cast<float>(ticks_left / mine_ticks));

	ImGui::End();
}
