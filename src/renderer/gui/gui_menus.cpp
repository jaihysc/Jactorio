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

constexpr int kInventorySlotWidth        = 36;
constexpr int kInventorySlotPadding      = 3;
constexpr int kInventorySlotImagePadding = 2;

constexpr int kInventoryItemCountXOffset = 0;
constexpr int kInventoryItemCountYOffset = 0;

using namespace jactorio;

///
/// \brief Pad the ingredients: text with trailing whitespace to reach the length of the title
///
/// In order to auto resize to fit the title's text since the title is not accounted
void FitTitle(std::stringstream& description_ss, const uint16_t target_len) {
	while (description_ss.str().size() < target_len)
		description_ss << " ";
}

///
/// \brief Adds additional vertical space
void AddVerticalSpace(float y) {
	ImGui::Dummy({0, y});
	ImGui::NewLine();
}

///
/// \param title Title of the tooltip
/// \param description
/// \param draw_func Code to run while drawing the tooltip
void DrawCursorTooltip(game::PlayerData& player_data, const char* title, const char* description,
                       const std::function<void()>& draw_func) {
	using namespace jactorio;

	ImVec2 cursor_pos(
		static_cast<float>(game::MouseSelection::GetCursorX()),
		static_cast<float>(game::MouseSelection::GetCursorY() + 10.f)
	);
	// If an item is currently selected, move the tooltip down to not overlap
	if (player_data.GetSelectedItem())
		cursor_pos.y += kInventorySlotWidth;

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
/// \brief Auto positions slots based on provided specifications, draws using provided callback <br>
/// Call within a ImGui window
/// \param slot_span Slots before wrapping onto new line
/// \param slot_count Number of slots to draw
/// \param draw_func Draws slot (index, bool button_hovered)
/// \param ending_vertical_space If < 0 use default, >= 0 use amount provided of vertical space upon finishing drawing all slots
void DrawSlots(const uint8_t slot_span, const uint16_t slot_count,
               const uint8_t scale,
               const std::function<void(uint16_t, bool&)>& draw_func,
               const float ending_vertical_space = -1.f) {
	J_GUI_RAII_STYLE_VAR_POP(1);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});

	// If all the slots are drawn without a newline, add one
	bool printed_newline = false;

	uint16_t index = 0;

	bool button_hovered = false;

	while (index < slot_count) {
		const uint16_t x = index % slot_span;
		ImGui::SameLine(J_GUI_STYLE_WINDOW_PADDING_X + x * scale * (kInventorySlotWidth + kInventorySlotPadding));

		ImGui::PushID(index);  // Uniquely identifies the button
		draw_func(index, button_hovered);
		ImGui::PopID();

		if (x == slot_span - 1) {
			printed_newline = true;
			AddVerticalSpace(static_cast<float>(scale) * (kInventorySlotWidth + kInventorySlotPadding));
		}

		++index;
	}

	if (!printed_newline) {
		if (ending_vertical_space < 0) {
			AddVerticalSpace(static_cast<float>(scale) * (kInventorySlotWidth + kInventorySlotPadding));
		}
		else {
			AddVerticalSpace(ending_vertical_space);
		}
	}
}

///
/// \brief Positions item slots based on provided specifications
/// \param scale 
/// \param sprite_iid Internal id of the sprite to be drawn, if 0, a blank slot will be drawn
/// \param item_count Number to display on the item, 0 to hide
/// \param button_hovered Used to ensure hover status is only applied to one item slot out of a set
/// \param button_events Handle events from the button 
void DrawItemSlot(const renderer::MenuData& menu_data,
                  const uint8_t scale,
                  const uint32_t sprite_iid,
                  const uint16_t item_count,
                  bool& button_hovered,
                  const std::function<void()>& button_events = []() {
                  }) {
	const float original_x_offset = ImGui::GetCursorPosX();
	const float original_y_offset = ImGui::GetCursorPosY();

	const float x_offset = original_x_offset - kInventorySlotPadding;
	const float y_offset = original_y_offset - kInventorySlotPadding;

	bool backing_button_hover = false;
	// Backing button, detects clicks
	{
		J_GUI_RAII_STYLE_COLOR_POP(3);
		ImGui::PushStyleColor(ImGuiCol_Button, J_GUI_COL_NONE);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, J_GUI_COL_NONE);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, J_GUI_COL_NONE);

		ImGui::SetCursorPos({x_offset, y_offset});

		ImGui::ImageButton(
			nullptr,
			ImVec2(0, 0),
			ImVec2(-1, -1), ImVec2(-1, -1),
			(kInventorySlotWidth / 2 + kInventorySlotPadding) * scale
		);

		button_events();

		if (!button_hovered) {
			if ((backing_button_hover = ImGui::IsItemHovered()) == true)
				button_hovered = true;
		}
	}

	// Visible button, lower width such that a border is visible between slots
	{
		// Give visible button hover style if back is hovered
		if (backing_button_hover) {
			ImGui::PushStyleColor(ImGuiCol_Button, J_GUI_COL_BUTTON_HOVER);
		}


		// Center in backing button
		ImGui::SetCursorPos({x_offset + kInventorySlotPadding, y_offset + kInventorySlotPadding});

		if (sprite_iid == 0) {
			// Blank button
			ImGui::ImageButton(
				nullptr,
				ImVec2(0, 0),
				ImVec2(-1, -1),
				ImVec2(-1, -1),
				(kInventorySlotWidth / 2 * scale) + ((scale - 1) * kInventorySlotImagePadding)
			);
		}
		else {
			const unsigned int button_size =
				scale * kInventorySlotWidth
				+ (scale - 1) * kInventorySlotPadding  // To align with other scales, account for the padding between slots
				- 2 * kInventorySlotImagePadding;

			const auto& uv = menu_data.spritePositions.at(sprite_iid);
			ImGui::ImageButton(
				reinterpret_cast<void*>(menu_data.texId),
				ImVec2(
					static_cast<float>(button_size),
					static_cast<float>(button_size)
				),
				ImVec2(uv.topLeft.x, uv.topLeft.y),
				ImVec2(uv.bottomRight.x, uv.bottomRight.y),
				kInventorySlotImagePadding
			);
		}


		if (backing_button_hover)
			ImGui::PopStyleColor();

		// Total items count
		if (item_count != 0) {
			ImGui::SetCursorPos({x_offset + kInventoryItemCountXOffset, y_offset + kInventoryItemCountYOffset});
			ImGui::Text("%d", item_count);
		}

		// Set cursor position for next iteration 
		ImGui::SetCursorPos({original_x_offset, original_y_offset});
		ImGui::Dummy({0, 0});
	}
}

// ======================================================================
// Window positioning

///
/// \brief The window size is calculated on the size of the player's inventory
ImVec2 GetWindowSize() {
	// 20 is window padding on both sides, 80 for y is to avoid the scrollbar
	auto window_size = ImVec2(
		2 * J_GUI_STYLE_WINDOW_PADDING_X,
		2 * J_GUI_STYLE_WINDOW_PADDING_Y + 80);

	window_size.x += 10 * (kInventorySlotWidth + kInventorySlotPadding) - kInventorySlotPadding;
	window_size.y += static_cast<unsigned int>(game::PlayerData::kInventorySize / 10) *
		(kInventorySlotWidth + kInventorySlotPadding) - kInventorySlotPadding;

	return window_size;
}

///
/// \brief The next window drawn will be on the left center of the screen
void SetupNextWindowLeft(const ImVec2& window_size) {
	// Uses pixel coordinates, top left is 0, 0, bottom right x, x
	// Character window is left of the center
	const ImVec2 window_center(static_cast<float>(renderer::Renderer::GetWindowWidth()) / 2,
	                           static_cast<float>(renderer::Renderer::GetWindowHeight()) / 2);

	ImGui::SetNextWindowPos(ImVec2(window_center.x - window_size.x,
	                               window_center.y - window_size.y / 2));
}

void SetupNextWindowLeft() {
	SetupNextWindowLeft(GetWindowSize());
}

///
/// The next window drawn will be on the left center of the screen
void SetupNextWindowRight(const ImVec2& window_size) {
	// Uses pixel coordinates, top left is 0, 0, bottom right x, x
	// Character window is left of the center
	const ImVec2 window_center(static_cast<float>(renderer::Renderer::GetWindowWidth()) / 2,
	                           static_cast<float>(renderer::Renderer::GetWindowHeight()) / 2);

	// Recipe menu
	ImGui::SetNextWindowPos(ImVec2(window_center.x,
	                               window_center.y - window_size.y / 2));

	ImGui::SetNextWindowSize(window_size);
}

void SetupNextWindowRight() {
	SetupNextWindowRight(GetWindowSize());
}

// ==========================================================================================
// Player menus (Excluding entity menus)

const ImGuiWindowFlags kMenuFlags = 0 | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

///
/// \brief Draws the player's inventory menu
void PlayerInventoryMenu(game::PlayerData& player_data) {
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize;

	const ImVec2 window_size = GetWindowSize();
	ImGui::SetNextWindowSize(window_size);

	ImGui::Begin("Character", nullptr, window_flags);

	auto menu_data = renderer::GetMenuData();

	DrawSlots(10, game::PlayerData::kInventorySize, 1, [&](auto index, auto& button_hovered) {
		const auto& item = player_data.inventoryPlayer[index];

		// Draw blank slot if item doe snot exist at inventory slot
		auto sprite_id = item.first != nullptr ? item.first->sprite->internalId : 0;

		DrawItemSlot(
			menu_data,
			1, sprite_id,
			item.second,
			button_hovered,
			[&]() {
				if (ImGui::IsItemClicked()) {
					player_data.InventoryClick(
						index, 0, true, player_data.inventoryPlayer);
					player_data.InventorySort();
				}
				else if (ImGui::IsItemClicked(1)) {
					player_data.InventoryClick(
						index, 1, true, player_data.inventoryPlayer);
					player_data.InventorySort();
				}

				// Only draw tooltip + item count if item count is not 0
				if (ImGui::IsItemHovered() && item.second != 0) {
					DrawCursorTooltip(
						player_data,
						item.first->GetLocalizedName().c_str(),
						"sample description",
						[&]() {
							ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_NONE);
							ImGui::TextUnformatted(item.first->GetLocalizedName().c_str());
							ImGui::PopStyleColor();
						}
					);
				}
			});
	});

	ImGui::End();
}

void RecipeMenu(game::PlayerData& player_data,
                const std::function<
	                void(const renderer::MenuData& menu_data,
	                     const data::Recipe& recipe, const data::Item& product,
	                     bool& button_hovered)
                >& item_slot_draw) {

	const auto menu_data = renderer::GetMenuData();

	const ImVec2 window_size = GetWindowSize();
	ImGui::SetNextWindowSize(window_size);

	ImGui::Begin("Recipe", nullptr, kMenuFlags);

	// Menu groups | A group button is twice the size of a slot
	auto groups = data::DataRawGetAllSorted<data::RecipeGroup>(data::DataCategory::recipe_group);

	DrawSlots(5, groups.size(), 2, [&](const uint16_t index, auto& button_hovered) {
		const auto& recipe_group = groups[index];

		// Different color for currently selected recipe group
		if (index == player_data.RecipeGroupGetSelected())
			ImGui::PushStyleColor(ImGuiCol_Button, J_GUI_COL_BUTTON_HOVER);

		DrawItemSlot(menu_data, 2, recipe_group->sprite->internalId, 0, button_hovered, [&]() {
			if (index == player_data.RecipeGroupGetSelected())
				ImGui::PopStyleColor();

			// Recipe group click
			if (ImGui::IsItemClicked())
				player_data.RecipeGroupSelect(index);

			// Item tooltip
			std::stringstream description_ss;
			description_ss << recipe_group->GetLocalizedDescription().c_str();
			FitTitle(description_ss, recipe_group->GetLocalizedName().size());

			if (ImGui::IsItemHovered()) {
				DrawCursorTooltip(
					player_data,
					recipe_group->GetLocalizedName().c_str(),
					description_ss.str().c_str(),
					[&]() {
					}
				);
			}
		});

	});

	// Menu recipes
	const auto& selected_group = groups[player_data.RecipeGroupGetSelected()];

	bool button_hovered = false;
	for (auto& recipe_category : selected_group->recipeCategories) {
		const auto& recipes = recipe_category->recipes;

		DrawSlots(10, recipes.size(), 1, [&](auto index, auto&) {
			const data::Recipe* recipe = recipes.at(index);

			const auto* product =
				data::DataRawGet<data::Item>(data::DataCategory::item, recipe->GetProduct().first);
			assert(product != nullptr);  // Invalid recipe product

			item_slot_draw(menu_data, *recipe, *product, button_hovered);
		});
	}

	ImGui::End();
}

// ======================================================================

void renderer::CharacterMenu(game::PlayerData& player_data, const data::UniqueDataBase*) {
	SetupNextWindowLeft();
	PlayerInventoryMenu(player_data);

	SetupNextWindowRight();
	RecipeMenu(player_data, [&](auto& menu_data, auto& recipe, auto& product, auto& button_hovered) {
		DrawItemSlot(menu_data, 1, product.sprite->internalId, 0, button_hovered, [&]() {
			if (ImGui::IsItemClicked()) {
				if (player_data.RecipeCanCraft(recipe, 1)) {
					player_data.RecipeCraftR(recipe);
					player_data.InventorySort();
				}
			}

			// Draw item tooltip
			if (!ImGui::IsItemHovered() || button_hovered)
				return;

			std::stringstream title_ss;
			// Show the product yield in the title
			title_ss << product.GetLocalizedName().c_str() << " (" << recipe.GetProduct().second << ")";

			std::stringstream description_ss;
			description_ss << "Ingredients:";
			FitTitle(description_ss, title_ss.str().size());

			DrawCursorTooltip(
				player_data,
				title_ss.str().c_str(),
				description_ss.str().c_str(),
				[&]() {
					// ingredients
					for (const auto& ingredient_pair : recipe.ingredients) {
						const auto* item =
							data::DataRawGet<data::Item>(data::DataCategory::item,
							                             ingredient_pair.first);

						DrawItemSlot(menu_data, 1, item->sprite->internalId, 0, button_hovered);

						// Amount of the current ingredient the player has in inventory

						const auto player_item_count = game::GetInvItemCount(
							player_data.inventoryPlayer, game::PlayerData::kInventorySize,
							item);

						ImGui::SameLine(kInventorySlotWidth * 1.5);

						// Does not have ingredient
						if (player_item_count < ingredient_pair.second) {
							const bool can_be_recurse_crafted = player_data.RecipeCanCraft(recipe, 1);
							if (can_be_recurse_crafted) {
								// Ingredient can be crafted recursively
								ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_TEXT_WARNING);
							}
							else {
								// Ingredient cannot be crafted
								ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_TEXT_ERROR);
							}

							ImGui::Text("%d/%d x %s", player_item_count, ingredient_pair.second,
							            item->GetLocalizedName().c_str());
							ImGui::PopStyleColor();
						}
							// Has enough
						else {
							ImGui::Text("%d x %s", ingredient_pair.second,
							            item->GetLocalizedName().c_str());
						}

						AddVerticalSpace(8);
					}
					ImGui::Text("%.1f seconds", recipe.craftingTime);

					// Total raw
					ImGui::Separator();

					ImGui::Text("%s", "Total Raw:");
					AddVerticalSpace(8);

					auto raw_inames = data::Recipe::RecipeGetTotalRaw(product.name);

					DrawSlots(5, raw_inames.size(), 1, [&](const auto slot_index, auto&) {
						const auto* item =
							data::DataRawGet<data::Item>(data::DataCategory::item, raw_inames[slot_index].first);

						const auto item_count_required = raw_inames[slot_index].second;

						// const auto player_item_count = 
						// 	game::GetInvItemCount(player_data.inventoryPlayer, player_data.kInventorySize, item);
						//
						//
						// J_GUI_RAII_STYLE_COLOR_POP(1);
						// if (player_item_count < item_count_required) {
						// 	ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_TEXT_ERROR);
						// }
						// else
						// 	ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_TEXT);

						DrawItemSlot(menu_data, 1, item->sprite->internalId, item_count_required, button_hovered);
					}, 0);

				});
		});
	});
}

void renderer::CursorWindow(game::PlayerData& player_data, const data::UniqueDataBase*) {
	using namespace jactorio;
	// Draw the tooltip of what is currently selected

	const auto menu_data = GetMenuData();

	// Player has an item selected, draw it on the tooltip
	const data::ItemStack* selected_item;
	if ((selected_item = player_data.GetSelectedItem()) != nullptr) {
		ImGui::PushStyleColor(ImGuiCol_Border, J_GUI_COL_NONE);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, J_GUI_COL_NONE);

		// Draw the window at the cursor
		const ImVec2 cursor_pos(
			static_cast<float>(game::MouseSelection::GetCursorX()),
			static_cast<float>(game::MouseSelection::GetCursorY() + 2.f)
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

		const auto& positions = menu_data.spritePositions.at(selected_item->first->sprite->internalId);

		ImGui::SameLine(10.f);
		ImGui::Image(
			reinterpret_cast<void*>(menu_data.texId),
			ImVec2(32, 32),

			ImVec2(positions.topLeft.x, positions.topLeft.y),
			ImVec2(positions.bottomRight.x, positions.bottomRight.y)
		);

		ImGui::SameLine(10.f);
		ImGui::Text("%d", selected_item->second);

		ImGui::End();
		ImGui::PopStyleColor(2);

	}
}

void renderer::CraftingQueue(game::PlayerData& player_data, const data::UniqueDataBase*) {
	auto menu_data = GetMenuData();

	ImGuiWindowFlags flags = 0;
	flags |= ImGuiWindowFlags_NoBackground;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoScrollbar;
	flags |= ImGuiWindowFlags_NoScrollWithMouse;

	const auto& recipe_queue = player_data.GetRecipeQueue();


	const unsigned int y_slots = (recipe_queue.size() + 10 - 1) / 10;  // Always round up for slot count
	auto y_offset              = y_slots * (kInventorySlotWidth + kInventorySlotPadding);

	const unsigned int max_queue_height = Renderer::GetWindowHeight() / 2; // Pixels

	// Clamp to max queue height if greater
	if (y_offset > max_queue_height)
		y_offset = max_queue_height;

	ImGui::SetNextWindowPos(
		ImVec2(0,
		       static_cast<float>(Renderer::GetWindowHeight()) - y_offset
		       - J_GUI_STYLE_WINDOW_PADDING_X));  // Use the x padding to keep it constant on x and y
	ImGui::SetNextWindowSize(
		ImVec2(
			20 + 10 * (kInventorySlotWidth + kInventorySlotPadding) - kInventorySlotPadding,
			static_cast<float>(max_queue_height)));

	// Window
	ImGui::Begin("_crafting_queue", nullptr, flags);

	ImGui::PushStyleColor(ImGuiCol_Button, J_GUI_COL_NONE);
	ImGui::PushStyleColor(ImGuiCol_Border, J_GUI_COL_NONE);

	DrawSlots(10, recipe_queue.size(), 1, [&](auto index, auto& button_hovered) {
		const data::Recipe* recipe = recipe_queue.at(index);

		const auto* item =
			data::DataRawGet<data::Item>(data::DataCategory::item,
			                             recipe->GetProduct().first);
		DrawItemSlot(menu_data, 1,
		             item->sprite->internalId, recipe->GetProduct().second,
		             button_hovered);
	});

	ImGui::PopStyleColor(2);
	ImGui::End();
}

float last_pickup_fraction = 0.f;

void renderer::PickupProgressbar(game::PlayerData& player_data, const data::UniqueDataBase*) {
	constexpr float progress_bar_width  = 260 * 2;
	constexpr float progress_bar_height = 13;

	const float pickup_fraction = player_data.GetPickupPercentage();
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
			static_cast<float>(Renderer::GetWindowWidth()) / 2 - (progress_bar_width / 2),  // Center X
			static_cast<float>(Renderer::GetWindowHeight()) - progress_bar_height));  // TODO account for hotbar when implemented

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
void renderer::ContainerEntity(game::PlayerData& player_data, const data::UniqueDataBase* unique_data) {
	assert(unique_data);
	const auto& container_data = *static_cast<const data::ContainerEntityData*>(unique_data);

	SetupNextWindowLeft();
	PlayerInventoryMenu(player_data);

	SetupNextWindowRight();
	ImGui::Begin("Container", nullptr, kMenuFlags);

	DrawSlots(10, container_data.size, 1, [&](auto i, auto& button_hovered) {
		const auto sprite_id = container_data.inventory[i].first != nullptr
			                       ? container_data.inventory[i].first->sprite->internalId
			                       : 0;

		DrawItemSlot(
			GetMenuData(), 1,
			sprite_id, container_data.inventory[i].second, button_hovered, [&]() {
				if (ImGui::IsItemClicked()) {
					player_data.InventoryClick(i, 0, false, container_data.inventory);
					player_data.InventorySort();
				}
				else if (ImGui::IsItemClicked(1)) {
					player_data.InventoryClick(i, 1, false, container_data.inventory);
					player_data.InventorySort();
				}
			});
	});

	ImGui::End();
}

void renderer::MiningDrill(game::PlayerData& player_data, const data::UniqueDataBase* unique_data) {
	assert(unique_data);
	const auto& drill_data = *static_cast<const data::MiningDrillData*>(unique_data);

	SetupNextWindowLeft();
	PlayerInventoryMenu(player_data);

	SetupNextWindowRight();
	ImGui::Begin("Mining drill", nullptr, kMenuFlags);

	// 1 - (Ticks left / Ticks to mine)
	const long double ticks_left = static_cast<long double>(drill_data.deferralEntry.first) - player_data
	                                                                                          .GetPlayerWorld().GameTick();
	const long double mine_ticks = drill_data.miningTicks;

	if (drill_data.deferralEntry.second == 0)
		ImGui::ProgressBar(0.f);  // Drill has not started
	else 
		ImGui::ProgressBar(1.f - static_cast<float>(ticks_left / mine_ticks));

	ImGui::End();
}
