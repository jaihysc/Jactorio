// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/01/2020

#include "renderer/gui/gui_menus.h"

#include <functional>
#include <sstream>

#include "core/utility.h"

#include "data/prototype/entity/assembly_machine.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/item/recipe_group.h"

#include "game/input/mouse_selection.h"
#include "game/logic/inventory_controller.h"
#include "game/player/player_data.h"

#include "renderer/gui/gui_colors.h"
#include "renderer/gui/gui_layout.h"
#include "renderer/gui/imgui_manager.h"
#include "renderer/rendering/renderer.h"

using namespace jactorio;

///
/// \brief Implements ImGui::IsItemClicked() for left and right mouse buttons
template <bool HalfSelectOnLeft = false, bool HalfSelectOnRight = true>
void ImplementInventoryIsItemClicked(game::PlayerData& player_data,
                                     const data::DataManager& data_manager,
                                     data::Item::Inventory& inv, const size_t index) {
	if (ImGui::IsItemClicked()) {
		player_data.HandleInventoryActions(data_manager, inv, index, HalfSelectOnLeft);
	}
	else if (ImGui::IsItemClicked(1)) {
		player_data.HandleInventoryActions(data_manager, inv, index, HalfSelectOnRight);
	}

}

// ==========================================================================================
// Player menus (Excluding entity menus)

const ImGuiWindowFlags kMenuFlags = 0 | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

///
/// \brief Draws the player's inventory menu
void PlayerInventoryMenu(game::PlayerData& player_data, const data::DataManager& data_manager) {
	const ImVec2 window_size = renderer::GetWindowSize();
	ImGui::SetNextWindowSize(window_size);

	renderer::ImGuard guard{};
	guard.Begin("_character", nullptr, kMenuFlags);

	renderer::DrawTitleBar("Character");

	auto menu_data = renderer::GetMenuData();

	renderer::RemoveItemSlotTopPadding();
	renderer::DrawSlots(10, player_data.inventoryPlayer.size(), 1, [&](auto index, auto& button_hovered) {
		const auto& item = player_data.inventoryPlayer[index];

		// Draw blank slot if item doe snot exist at inventory slot
		auto sprite_id = item.first != nullptr ? item.first->sprite->internalId : 0;

		DrawItemSlot(
			menu_data,
			1, sprite_id,
			item.second,
			button_hovered,
			[&]() {
				ImplementInventoryIsItemClicked(player_data, data_manager, player_data.inventoryPlayer, index);

				// Only draw tooltip + item count if item count is not 0
				if (ImGui::IsItemHovered() && item.second != 0) {
					renderer::DrawCursorTooltip(
						player_data, data_manager,
						item.first->GetLocalizedName().c_str(),
						"sample description",
						[&]() {
							renderer::ImGuard tooltip_guard{};

							tooltip_guard.PushStyleColor(ImGuiCol_Text, J_GUI_COL_NONE);
							ImGui::TextUnformatted(item.first->GetLocalizedName().c_str());
						}
					);
				}
			});
	});
}

void RecipeMenu(game::PlayerData& player_data, const data::DataManager& data_manager, const std::string& title,
                const std::function<
	                void(const data::Recipe& recipe, const data::Item& product,
	                     bool& button_hovered)
                >& item_slot_draw) {

	const auto menu_data = renderer::GetMenuData();

	const ImVec2 window_size = renderer::GetWindowSize();
	ImGui::SetNextWindowSize(window_size);

	renderer::ImGuard guard{};
	guard.Begin("_recipe", nullptr, kMenuFlags);

	// Title with search bar
	renderer::DrawTitleBar(title, [&]() {
		ImGui::SameLine();
		// Shift above to center title text in middle of search bar
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - J_GUI_STYLE_TITLEBAR_PADDING_Y / 2);

		renderer::ImGuard title_guard{};
		title_guard.PushStyleVar(ImGuiStyleVar_FramePadding,
		                         {J_GUI_STYLE_WINDOW_PADDING_X, J_GUI_STYLE_TITLEBAR_PADDING_Y / 2});

		// Search text
		// Make temporary buffer, copy std::string contents into, pass to imgui input, copy result back into std::string
		const size_t search_buf_size = 100;

		char buf[search_buf_size + 1];

		const auto end = std::min(player_data.recipeSearchText.size(), search_buf_size);
		for (size_t i = 0; i < end; ++i) {
			buf[i] = player_data.recipeSearchText[i];
		}
		buf[end] = '\0';

		ImGui::InputText("", buf, search_buf_size);
		player_data.recipeSearchText = buf;

		// Continue title bar calculations from where the label text was
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - J_GUI_STYLE_TITLEBAR_PADDING_Y / 2);
	});

	// Menu groups | A group button is twice the size of a slot
	auto groups = data_manager.DataRawGetAllSorted<data::RecipeGroup>(data::DataCategory::recipe_group);

	renderer::RemoveItemSlotTopPadding();
	renderer::DrawSlots(5, groups.size(), 2, [&](const uint16_t index, auto& button_hovered) {
		const auto& recipe_group = groups[index];

		// Only draw item categories matching searched product name 
		for (auto& recipe_category : recipe_group->recipeCategories) {
			for (auto& recipe : recipe_category->recipes) {
				const auto& product_name =
					data_manager.DataRawGet<data::Item>(data::DataCategory::item, recipe->product.first)->GetLocalizedName();

				if (core::StrToLower(product_name).find(core::StrToLower(player_data.recipeSearchText)) != std::string::npos) {
					goto loop_exit;
				}
			}
		}
		return;

	loop_exit:
		// Different color for currently selected recipe group
		renderer::ImGuard recipe_group_guard{};
		if (index == player_data.RecipeGroupGetSelected())
			recipe_group_guard.PushStyleColor(ImGuiCol_Button, J_GUI_COL_BUTTON_HOVER);

		DrawItemSlot(menu_data, 2, recipe_group->sprite->internalId, 0, button_hovered, [&]() {
			// Recipe group click
			if (ImGui::IsItemClicked())
				player_data.RecipeGroupSelect(index);

			// Item tooltip
			std::stringstream description_ss;
			description_ss << recipe_group->GetLocalizedDescription().c_str();
			renderer::FitTitle(description_ss, recipe_group->GetLocalizedName().size());

			if (ImGui::IsItemHovered()) {
				renderer::DrawCursorTooltip(
					player_data, data_manager,
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
	for (const auto& recipe_category : selected_group->recipeCategories) {
		const auto& recipes = recipe_category->recipes;

		renderer::DrawSlots(10, recipes.size(), 1, [&](auto index, auto&) {
			const data::Recipe* recipe = recipes.at(index);

			const auto* product =
				data_manager.DataRawGet<data::Item>(data::DataCategory::item, recipe->product.first);
			assert(product != nullptr);  // Invalid recipe product

			// Do not draw item slot if it does not match search text
			if (core::StrToLower(product->GetLocalizedName())
				.find(core::StrToLower(player_data.recipeSearchText)) == std::string::npos)
				return;

			DrawItemSlot(menu_data, 1, product->sprite->internalId, 0, button_hovered, [&]() {
				item_slot_draw(*recipe, *product, button_hovered);
			});
		});
	}
}

///
/// \brief Draws preview tooltip for a recipe
/// \tparam IsPlayerCrafting Shows items possessed by the player and opportunities for intermediate crafting
template <bool IsPlayerCrafting>
void RecipeHoverTooltip(game::PlayerData& player_data, const data::DataManager& data_manager,
                        const data::Recipe& recipe, const data::Item& product) {
	auto menu_data = renderer::GetMenuData();

	std::stringstream title_ss;
	// Show the product yield in the title
	title_ss << product.GetLocalizedName().c_str() << " (" << recipe.product.second << ")";

	std::stringstream description_ss;
	description_ss << "Ingredients:";
	renderer::FitTitle(description_ss, title_ss.str().size());

	bool hovered = false;  // Not needed since the tooltip is always drawn under the cursor
	renderer::DrawCursorTooltip(
		player_data, data_manager,
		title_ss.str().c_str(),
		description_ss.str().c_str(),
		[&]() {
			// Ingredients
			for (const auto& ingredient_pair : recipe.ingredients) {
				const auto* item = data_manager.DataRawGet<data::Item>(data::DataCategory::item,
				                                                       ingredient_pair.first);

				DrawItemSlot(menu_data, 1, item->sprite->internalId, 0, hovered);

				// Amount of the current ingredient the player has in inventory
				const auto player_item_count = game::GetInvItemCount(player_data.inventoryPlayer, item);

				ImGui::SameLine(renderer::kInventorySlotWidth * 1.5);

				// Does not have ingredient
				if (IsPlayerCrafting && player_item_count < ingredient_pair.second) {
					const bool can_be_recurse_crafted = player_data.RecipeCanCraft(data_manager, recipe, 1);

					renderer::ImGuard guard{};
					if (can_be_recurse_crafted)
						guard.PushStyleColor(ImGuiCol_Text, J_GUI_COL_TEXT_WARNING);
					else
						guard.PushStyleColor(ImGuiCol_Text, J_GUI_COL_TEXT_ERROR);

					ImGui::Text("%d/%d x %s", player_item_count, ingredient_pair.second,
					            item->GetLocalizedName().c_str());
				}
					// Has enough
				else {
					ImGui::Text("%d x %s", ingredient_pair.second,
					            item->GetLocalizedName().c_str());
				}
			}
			ImGui::Text("%.1f seconds", recipe.craftingTime);

			// Total raw items
			ImGui::Separator();

			ImGui::Text("%s", "Total Raw:");

			auto raw_inames = data::Recipe::RecipeGetTotalRaw(data_manager, product.name);

			renderer::DrawSlots(5, raw_inames.size(), 1, [&](const auto slot_index, auto&) {
				const auto* item =
					data_manager.DataRawGet<data::Item>(data::DataCategory::item, raw_inames[slot_index].first);

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

				DrawItemSlot(menu_data, 1, item->sprite->internalId, item_count_required, hovered);
			}, 0);

		});
}

// ======================================================================

void renderer::CharacterMenu(game::PlayerData& player_data, const data::DataManager& data_manager,
                             const data::PrototypeBase*, data::UniqueDataBase*) {
	SetupNextWindowLeft();
	PlayerInventoryMenu(player_data, data_manager);

	SetupNextWindowRight();
	RecipeMenu(player_data, data_manager, "Recipe",
	           [&](auto& recipe, auto& product, auto& button_hovered) {
		           if (ImGui::IsItemClicked()) {
			           if (player_data.RecipeCanCraft(data_manager, recipe, 1)) {
				           player_data.RecipeCraftR(data_manager, recipe);
				           player_data.InventorySort(player_data.inventoryPlayer);
			           }
		           }

		           if (ImGui::IsItemHovered() && !button_hovered)
			           RecipeHoverTooltip<true>(player_data, data_manager, recipe, product);
	           });
}

void renderer::CursorWindow(game::PlayerData& player_data, const data::DataManager&,
                            const data::PrototypeBase*, data::UniqueDataBase*) {
	using namespace jactorio;
	// Draw the tooltip of what is currently selected

	const auto menu_data = GetMenuData();

	// Player has an item selected, draw it on the tooltip
	const data::Item::Stack* selected_item;
	if ((selected_item = player_data.GetSelectedItem()) != nullptr) {
		ImGuard guard{};
		guard.PushStyleColor(ImGuiCol_Border, J_GUI_COL_NONE);
		guard.PushStyleColor(ImGuiCol_PopupBg, J_GUI_COL_NONE);

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

		// ======================================================================

		ImGui::SetNextWindowFocus();
		guard.Begin("_selected_item", nullptr, flags);

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
	}
}

void renderer::CraftingQueue(game::PlayerData& player_data, const data::DataManager& data_manager,
                             const data::PrototypeBase*, data::UniqueDataBase*) {
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
	ImGuard guard{};
	guard.Begin("_crafting_queue", nullptr, flags);

	guard.PushStyleColor(ImGuiCol_Button, J_GUI_COL_NONE);
	guard.PushStyleColor(ImGuiCol_Border, J_GUI_COL_NONE);

	DrawSlots(10, recipe_queue.size(), 1, [&](auto index, auto& button_hovered) {
		const data::Recipe* recipe = recipe_queue.at(index);

		const auto* item =
			data_manager.DataRawGet<data::Item>(data::DataCategory::item,
			                                    recipe->product.first);
		DrawItemSlot(menu_data, 1,
		             item->sprite->internalId, recipe->product.second,
		             button_hovered);
	});
}

float last_pickup_fraction = 0.f;

void renderer::PickupProgressbar(game::PlayerData& player_data, const data::DataManager&,
                                 const data::PrototypeBase*, data::UniqueDataBase*) {
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
	ImGuard guard{};
	guard.Begin("_entity_pickup_status", nullptr, flags);

	guard.PushStyleColor(ImGuiCol_Text, J_GUI_COL_NONE);
	guard.PushStyleColor(ImGuiCol_FrameBg, J_GUI_COL_PROGRESS_BG);
	guard.PushStyleColor(ImGuiCol_PlotHistogram, J_GUI_COL_PROGRESS);

	ImGui::ProgressBar(pickup_fraction, ImVec2(progress_bar_width, progress_bar_height));
}

// ==========================================================================================
// Entity menus
void renderer::ContainerEntity(game::PlayerData& player_data, const data::DataManager& data_manager,
                               const data::PrototypeBase* prototype, data::UniqueDataBase* unique_data) {
	assert(prototype);
	assert(unique_data);
	auto& container_data = *static_cast<data::ContainerEntityData*>(unique_data);

	SetupNextWindowLeft();
	PlayerInventoryMenu(player_data, data_manager);

	SetupNextWindowRight();

	ImGuard guard{};
	guard.Begin("_container", nullptr, kMenuFlags);

	DrawTitleBar(prototype->GetLocalizedName());

	DrawSlots(10, container_data.inventory.size(), 1, [&](auto index, auto& button_hovered) {
		const auto sprite_id = container_data.inventory[index].first != nullptr
			                       ? container_data.inventory[index].first->sprite->internalId
			                       : 0;

		DrawItemSlot(
			GetMenuData(), 1,
			sprite_id, container_data.inventory[index].second, button_hovered, [&]() {
				ImplementInventoryIsItemClicked(player_data, data_manager, container_data.inventory, index);
			});
	});
}

void renderer::MiningDrill(game::PlayerData& player_data, const data::DataManager& data_manager,
                           const data::PrototypeBase* prototype, data::UniqueDataBase* unique_data) {
	assert(prototype);
	assert(unique_data);
	const auto& drill_data = *static_cast<const data::MiningDrillData*>(unique_data);

	SetupNextWindowLeft();
	PlayerInventoryMenu(player_data, data_manager);

	SetupNextWindowRight();

	ImGuard guard{};
	guard.Begin("_mining_drill", nullptr, kMenuFlags);

	DrawTitleBar(prototype->GetLocalizedName());

	// 1 - (Ticks left / Ticks to mine)
	const long double ticks_left = static_cast<long double>(drill_data.deferralEntry.first) -
		player_data.GetPlayerWorld().GameTick();
	const long double mine_ticks = drill_data.miningTicks;

	if (drill_data.deferralEntry.second == 0)
		ImGui::ProgressBar(0.f);  // Drill has not started
	else
		ImGui::ProgressBar(1.f - static_cast<float>(ticks_left / mine_ticks));
}

void renderer::AssemblyMachine(game::PlayerData& player_data, const data::DataManager& data_manager,
                               const data::PrototypeBase* prototype, data::UniqueDataBase* unique_data) {
	assert(prototype);
	assert(unique_data);

	auto& world_data          = player_data.GetPlayerWorld();
	const auto& machine_proto = *static_cast<const data::AssemblyMachine*>(prototype);
	auto& machine_data        = *static_cast<data::AssemblyMachineData*>(unique_data);

	auto menu_data = GetMenuData();

	// Will be modifying AssemblyMachineData::recipe
	std::lock_guard<std::mutex> world_data_guard{world_data.worldDataMutex};

	if (machine_data.HasRecipe()) {
		const auto window_size = GetWindowSize();

		SetupNextWindowLeft(window_size);
		PlayerInventoryMenu(player_data, data_manager);

		SetupNextWindowRight();

		ImGuard guard{};
		guard.Begin("_assembly_machine", nullptr, kMenuFlags);

		DrawTitleBar(prototype->GetLocalizedName());


		// TODO ingredients \n recipe \n progressbar
		// TODO tooltip hints

		bool button_hovered = false;

		// Ingredients 
		RemoveItemSlotTopPadding();
		DrawSlots(10, machine_data.ingredientInv.size() + 1, 1, [&](auto index, bool&) {
			// Recipe change button
			if (index == machine_data.ingredientInv.size()) {
				auto* reset_icon = data_manager.DataRawGet<data::Item>(data::DataCategory::item, data::Item::kResetIname);
				assert(reset_icon != nullptr);

				DrawItemSlot(menu_data, 1, reset_icon->sprite->internalId, 0, button_hovered, [&]() {
					if (ImGui::IsItemClicked()) {
						machine_data.ChangeRecipe(world_data, machine_proto, nullptr);
					}
				});
				return;
			}

			// Item which is required
			auto* ingredient_item =
				data_manager.DataRawGet<data::Item>(data::DataCategory::item,
				                                    machine_data.GetRecipe()->ingredients[index].first);
			assert(ingredient_item != nullptr);

			// Amount of item possessed

			DrawItemSlot(menu_data, 1,
			             ingredient_item->sprite->internalId, machine_data.ingredientInv[index].second,
			             button_hovered, [&]() {
				             // TODO, filter to only allowed items
				             ImplementInventoryIsItemClicked(player_data, data_manager, machine_data.ingredientInv, index);
			             });
		});

		// User may have clicked the reset button, and a recipe no longer exists
		if (!machine_data.HasRecipe())
			return;

		// Progress
		const auto original_cursor_y = ImGui::GetCursorPosY();
		ImGui::SetCursorPosY(original_cursor_y + J_GUI_STYLE_TITLEBAR_PADDING_Y / 2);
		ImGui::ProgressBar(0.f, {window_size.x - 2 * kInventorySlotWidth, 0});

		ImGui::SameLine();
		ImGui::SetCursorPosY(original_cursor_y);

		// Product
		DrawSlots(10, 1, 1, [&](auto, bool&) {
			auto* product_item =
				data_manager.DataRawGet<data::Item>(data::DataCategory::item, machine_data.GetRecipe()->product.first);

			assert(product_item != nullptr);
			DrawItemSlot(menu_data, 1,
			             product_item->sprite->internalId, machine_data.productInv[0].second,
			             button_hovered, [&]() {
				             ImplementInventoryIsItemClicked(player_data, data_manager, machine_data.productInv, 0);
			             });
		});
	}
	else {
		// Only draw recipe menu if no recipe is selected for assembling
		SetupNextWindowCenter();
		RecipeMenu(player_data, data_manager, prototype->GetLocalizedName(),
		           [&](auto& recipe, auto& product, auto& button_hovered) {

			           if (ImGui::IsItemClicked()) {
				           machine_data.ChangeRecipe(world_data, machine_proto, &recipe);
			           }

			           if (ImGui::IsItemHovered() && !button_hovered)
				           RecipeHoverTooltip<false>(player_data, data_manager, recipe, product);
		           });
	}

}
