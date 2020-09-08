// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "renderer/gui/gui_menus.h"

#include <functional>
#include <sstream>

#include "data/prototype/assembly_machine.h"
#include "data/prototype/container_entity.h"
#include "data/prototype/mining_drill.h"
#include "data/prototype/recipe_group.h"

#include "game/input/mouse_selection.h"
#include "game/logic/inventory_controller.h"
#include "game/player/player_data.h"

#include "renderer/gui/gui_colors.h"
#include "renderer/gui/gui_layout.h"

using namespace jactorio;

///
/// Implements ImGui::IsItemClicked() for left and right mouse buttons
template <bool HalfSelectOnLeft = false, bool HalfSelectOnRight = true>
void ImplementInventoryIsItemClicked(
    game::PlayerData& player_data,
    const data::PrototypeManager& data_manager,
    data::Item::Inventory& inv,
    const size_t index,
    const std::function<void()>& on_click = []() {}) {
    if (ImGui::IsItemClicked()) {
        player_data.inventory.HandleInventoryActions(data_manager, inv, index, HalfSelectOnLeft);
        on_click();
    }
    else if (ImGui::IsItemClicked(1)) {
        player_data.inventory.HandleInventoryActions(data_manager, inv, index, HalfSelectOnRight);
        on_click();
    }
}

float GetProgressBarFraction(const GameTickT game_tick,
                             const game::DeferralTimer::DeferralEntry& entry,
                             const float total_ticks) {
    if (!entry.Valid())
        return 0.f;

    const auto ticks_left = core::SafeCast<long double>(entry.dueTick) - game_tick;
    return 1.f - core::LossyCast<float>(ticks_left / total_ticks);
}

// ==========================================================================================
// Player menus (Excluding entity menus)

const ImGuiWindowFlags kMenuFlags =
    0 | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

///
/// Draws the player's inventory menu
void PlayerInventoryMenu(game::PlayerData& player_data, const data::PrototypeManager& data_manager) {
    const ImVec2 window_size = renderer::GetWindowSize();
    ImGui::SetNextWindowSize(window_size);

    renderer::ImGuard guard{};
    guard.Begin("_character", nullptr, kMenuFlags);

    renderer::DrawTitleBar("Character");

    auto menu_data = renderer::GetMenuData();

    renderer::RemoveItemSlotTopPadding();
    renderer::DrawSlots(10, player_data.inventory.inventory.size(), 1, [&](auto index, auto& button_hovered) {
        const auto& stack = player_data.inventory.inventory[index];

        // Draw blank slot if item doe snot exist at inventory slot
        auto sprite_id = stack.item != nullptr ? stack.item->sprite->internalId : 0;

        DrawItemSlot(menu_data, 1, sprite_id, stack.count, button_hovered, [&]() {
            ImplementInventoryIsItemClicked(player_data, data_manager, player_data.inventory.inventory, index);

            // Only draw tooltip + item count if item count is not 0
            if (ImGui::IsItemHovered() && stack.count != 0) {
                renderer::DrawCursorTooltip(
                    player_data, data_manager, stack.item->GetLocalizedName().c_str(), "sample description", [&]() {
                        renderer::ImGuard tooltip_guard{};

                        tooltip_guard.PushStyleColor(ImGuiCol_Text, renderer::kGuiColNone);
                        ImGui::TextUnformatted(stack.item->GetLocalizedName().c_str());
                    });
            }
        });
    });
}

void RecipeMenu(game::PlayerData& player_data,
                const data::PrototypeManager& data_manager,
                const std::string& title,
                const std::function<void(const data::Recipe& recipe, bool& button_hovered)>& item_slot_draw) {

    const auto menu_data = renderer::GetMenuData();

    const ImVec2 window_size = renderer::GetWindowSize();
    ImGui::SetNextWindowSize(window_size);

    renderer::ImGuard guard{};
    guard.Begin("_recipe", nullptr, kMenuFlags);

    // Title with search bar
    renderer::DrawTitleBar(title, [&]() {
        ImGui::SameLine();
        // Shift above to center title text in middle of search bar
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - core::SafeCast<float>(renderer::kGuiStyleTitlebarPaddingY) / 2);

        renderer::ImGuard title_guard{};
        title_guard.PushStyleVar(
            ImGuiStyleVar_FramePadding,
            {renderer::kGuiStyleWindowPaddingX, core::SafeCast<float>(renderer::kGuiStyleTitlebarPaddingY) / 2});

        // Search text
        // Make temporary buffer, copy std::string contents into, pass to imgui input, copy result back into std::string
        const size_t search_buf_size = 100;

        char buf[search_buf_size + 1];

        const auto end = std::min(player_data.crafting.recipeSearchText.size(), search_buf_size);
        for (size_t i = 0; i < end; ++i) {
            buf[i] = player_data.crafting.recipeSearchText[i];
        }
        buf[end] = '\0';

        ImGui::InputText("", buf, search_buf_size);
        player_data.crafting.recipeSearchText = buf;

        // Continue title bar calculations from where the label text was
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - core::SafeCast<float>(renderer::kGuiStyleTitlebarPaddingY) / 2);
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
                    data_manager.DataRawGet<data::Item>(recipe->product.first)->GetLocalizedName();

                if (core::StrToLower(product_name).find(core::StrToLower(player_data.crafting.recipeSearchText)) !=
                    std::string::npos) {
                    goto loop_exit;
                }
            }
        }
        return;

    loop_exit:
        // Different color for currently selected recipe group
        renderer::ImGuard recipe_group_guard{};
        if (index == player_data.crafting.RecipeGroupGetSelected())
            recipe_group_guard.PushStyleColor(ImGuiCol_Button, renderer::kGuiColButtonHover);

        DrawItemSlot(menu_data, 2, recipe_group->sprite->internalId, 0, button_hovered, [&]() {
            // Recipe group click
            if (ImGui::IsItemClicked())
                player_data.crafting.RecipeGroupSelect(index);

            // Item tooltip
            std::stringstream description_ss;
            description_ss << recipe_group->GetLocalizedDescription().c_str();
            renderer::FitTitle(description_ss, recipe_group->GetLocalizedName().size());

            if (ImGui::IsItemHovered()) {
                renderer::DrawCursorTooltip(player_data,
                                            data_manager,
                                            recipe_group->GetLocalizedName().c_str(),
                                            description_ss.str().c_str(),
                                            [&]() {});
            }
        });
    });

    // Menu recipes
    const auto& selected_group = groups[player_data.crafting.RecipeGroupGetSelected()];

    bool button_hovered = false;
    for (const auto& recipe_category : selected_group->recipeCategories) {
        const auto& recipes = recipe_category->recipes;

        renderer::DrawSlots(10, recipes.size(), 1, [&](auto index, auto& /*slot_hovered*/) {
            const data::Recipe* recipe = recipes.at(index);

            const auto* product = data_manager.DataRawGet<data::Item>(recipe->product.first);
            assert(product != nullptr); // Invalid recipe product

            // Do not draw item slot if it does not match search text
            if (core::StrToLower(product->GetLocalizedName())
                    .find(core::StrToLower(player_data.crafting.recipeSearchText)) == std::string::npos)
                return;

            DrawItemSlot(menu_data, 1, product->sprite->internalId, 0, button_hovered, [&]() {
                item_slot_draw(*recipe, button_hovered);
            });
        });
    }
}

///
/// Draws preview tooltip for a recipe
/// \tparam IsPlayerCrafting Shows items possessed by the player and opportunities for intermediate crafting
template <bool IsPlayerCrafting>
void RecipeHoverTooltip(game::PlayerData& player_data,
                        const data::PrototypeManager& data_manager,
                        const data::Recipe& recipe) {
    auto menu_data = renderer::GetMenuData();

    auto* product_item = data_manager.DataRawGet<data::Item>(recipe.product.first);
    assert(product_item);

    std::stringstream title_ss;
    // Show the product yield in the title
    title_ss << product_item->GetLocalizedName().c_str() << " (" << recipe.product.second << ")";

    std::stringstream description_ss;
    description_ss << "Ingredients:";
    renderer::FitTitle(description_ss, title_ss.str().size());

    bool hovered = false; // Not needed since the tooltip is always drawn under the cursor
    renderer::DrawCursorTooltip(player_data, data_manager, title_ss.str().c_str(), description_ss.str().c_str(), [&]() {
        // Ingredients
        for (const auto& ingredient_pair : recipe.ingredients) {
            const auto* item = data_manager.DataRawGet<data::Item>(ingredient_pair.first);

            DrawItemSlot(menu_data, 1, item->sprite->internalId, 0, hovered);

            // Amount of the current ingredient the player has in inventory
            const auto player_item_count = game::GetInvItemCount(player_data.inventory.inventory, item);

            ImGui::SameLine(renderer::kInventorySlotWidth * 1.5);

            // Does not have ingredient
            if (IsPlayerCrafting && player_item_count < ingredient_pair.second) {
                const bool can_be_recurse_crafted = player_data.crafting.RecipeCanCraft(data_manager, recipe, 1);

                renderer::ImGuard guard{};
                if (can_be_recurse_crafted)
                    guard.PushStyleColor(ImGuiCol_Text, renderer::kGuiColTextWarning);
                else
                    guard.PushStyleColor(ImGuiCol_Text, renderer::kGuiColTextError);

                ImGui::Text("%d/%d x %s", player_item_count, ingredient_pair.second, item->GetLocalizedName().c_str());
            }
            // Has enough
            else {
                ImGui::Text("%d x %s", ingredient_pair.second, item->GetLocalizedName().c_str());
            }
        }
        ImGui::Text("%.1f seconds", recipe.craftingTime);

        // Total raw items
        ImGui::Separator();

        ImGui::Text("%s", "Total Raw:");

        auto raw_inames = data::Recipe::RecipeGetTotalRaw(data_manager, product_item->name);

        renderer::DrawSlots(
            5,
            raw_inames.size(),
            1,
            [&](const auto slot_index, auto& /*slot_hover*/) {
                const auto* item = data_manager.DataRawGet<data::Item>(raw_inames[slot_index].first);

                const auto item_count_required = raw_inames[slot_index].second;

                // const auto player_item_count =
                // 	game::GetInvItemCount(player_data.inventory.inventoryPlayer, player_data.inventory.kInventorySize,
                // item);
                //
                //
                // J_GUI_RAII_STYLE_COLOR_POP(1);
                // if (player_item_count < item_count_required) {
                // 	ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_TEXT_ERROR);
                // }
                // else
                // 	ImGui::PushStyleColor(ImGuiCol_Text, J_GUI_COL_TEXT);

                DrawItemSlot(menu_data, 1, item->sprite->internalId, item_count_required, hovered);
            },
            0);
    });
}

// ======================================================================

void renderer::CharacterMenu(const MenuFunctionParams& params) {
    auto& player_data        = params.player;
    const auto& data_manager = params.protoManager;

    SetupNextWindowLeft();
    PlayerInventoryMenu(player_data, data_manager);

    SetupNextWindowRight();
    RecipeMenu(player_data, data_manager, "Recipe", [&](auto& recipe, auto& button_hovered) {
        if (ImGui::IsItemClicked()) {
            if (player_data.crafting.RecipeCanCraft(data_manager, recipe, 1)) {
                player_data.crafting.RecipeCraftR(data_manager, recipe);
                player_data.inventory.InventorySort(player_data.inventory.inventory);
            }
        }

        if (ImGui::IsItemHovered() && !button_hovered)
            RecipeHoverTooltip<true>(player_data, data_manager, recipe);
    });
}

void renderer::CursorWindow(const MenuFunctionParams& params) {
    // Draw the tooltip of what is currently selected

    auto& player_data    = params.player;
    const auto menu_data = GetMenuData();

    // Player has an item selected, draw it on the tooltip
    const auto* selected_stack = player_data.inventory.GetSelectedItem();

    if (selected_stack != nullptr) {
        ImGuard guard{};
        guard.PushStyleColor(ImGuiCol_Border, kGuiColNone);
        guard.PushStyleColor(ImGuiCol_PopupBg, kGuiColNone);

        // Draw the window at the cursor
        const ImVec2 cursor_pos(core::LossyCast<float>(game::MouseSelection::GetCursorX()),
                                core::LossyCast<float>(game::MouseSelection::GetCursorY()) + 2.f);
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

        const auto& positions = menu_data.spritePositions.at(selected_stack->item->sprite->internalId);

        ImGui::SameLine(10.f);
        ImGui::Image(reinterpret_cast<void*>(menu_data.texId),
                     ImVec2(32, 32),

                     ImVec2(positions.topLeft.x, positions.topLeft.y),
                     ImVec2(positions.bottomRight.x, positions.bottomRight.y));

        ImGui::SameLine(10.f);
        ImGui::Text("%d", selected_stack->count);
    }
}

void renderer::CraftingQueue(const MenuFunctionParams& params) {
    auto& player_data        = params.player;
    const auto& data_manager = params.protoManager;
    auto menu_data           = GetMenuData();

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoBackground;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoScrollbar;
    flags |= ImGuiWindowFlags_NoScrollWithMouse;

    const auto& recipe_queue = player_data.crafting.GetRecipeQueue();


    const auto y_slots = (recipe_queue.size() + 10 - 1) / 10; // Always round up for slot count
    auto y_offset      = y_slots * (kInventorySlotWidth + kInventorySlotPadding);

    const unsigned int max_queue_height = Renderer::GetWindowHeight() / 2; // Pixels

    // Clamp to max queue height if greater
    if (y_offset > max_queue_height)
        y_offset = max_queue_height;

    ImGui::SetNextWindowPos(ImVec2(0,
                                   core::SafeCast<float>(Renderer::GetWindowHeight()) - y_offset -
                                       kGuiStyleWindowPaddingX)); // Use the x padding to keep it constant on x and y
    ImGui::SetNextWindowSize(ImVec2(20 + 10 * (kInventorySlotWidth + kInventorySlotPadding) - kInventorySlotPadding,
                                    core::SafeCast<float>(max_queue_height)));

    // Window
    ImGuard guard{};
    guard.Begin("_crafting_queue", nullptr, flags);

    guard.PushStyleColor(ImGuiCol_Button, kGuiColNone);
    guard.PushStyleColor(ImGuiCol_Border, kGuiColNone);

    DrawSlots(10, recipe_queue.size(), 1, [&](auto index, auto& button_hovered) {
        const data::Recipe* recipe = recipe_queue.at(index).Get();
        assert(recipe != nullptr);

        const auto* item = data_manager.DataRawGet<data::Item>(recipe->product.first);
        DrawItemSlot(menu_data, 1, item->sprite->internalId, recipe->product.second, button_hovered);
    });
}

float last_pickup_fraction = 0.f;

void renderer::PickupProgressbar(const MenuFunctionParams& params) {
    auto& player_data = params.player;

    constexpr float progress_bar_width  = 260 * 2;
    constexpr float progress_bar_height = 13;

    const float pickup_fraction = player_data.placement.GetPickupPercentage();
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
        ImVec2(core::SafeCast<float>(Renderer::GetWindowWidth()) / 2 - (progress_bar_width / 2), // Center X
               core::SafeCast<float>(Renderer::GetWindowHeight()) -
                   progress_bar_height)); // TODO account for hotbar when implemented

    // Window
    ImGuard guard{};
    guard.Begin("_entity_pickup_status", nullptr, flags);

    guard.PushStyleColor(ImGuiCol_Text, kGuiColNone);
    guard.PushStyleColor(ImGuiCol_FrameBg, kGuiColProgressBg);
    guard.PushStyleColor(ImGuiCol_PlotHistogram, kGuiColProgress);

    ImGui::ProgressBar(pickup_fraction, ImVec2(progress_bar_width, progress_bar_height));
}

// ==========================================================================================
// Entity menus
void renderer::ContainerEntity(const MenuFunctionParams& params) {
    auto& player_data        = params.player;
    const auto& data_manager = params.protoManager;
    const auto* prototype    = params.prototype;
    auto* unique_data        = params.uniqueData;

    assert(prototype != nullptr);
    assert(unique_data != nullptr);
    auto& container_data = *static_cast<data::ContainerEntityData*>(unique_data);

    SetupNextWindowLeft();
    PlayerInventoryMenu(player_data, data_manager);

    SetupNextWindowRight();

    ImGuard guard{};
    guard.Begin("_container", nullptr, kMenuFlags);

    DrawTitleBar(prototype->GetLocalizedName());

    DrawSlots(10, container_data.inventory.size(), 1, [&](auto index, auto& button_hovered) {
        const auto sprite_id = container_data.inventory[index].item != nullptr
            ? container_data.inventory[index].item->sprite->internalId
            : 0;

        DrawItemSlot(GetMenuData(), 1, sprite_id, container_data.inventory[index].count, button_hovered, [&]() {
            ImplementInventoryIsItemClicked(player_data, data_manager, container_data.inventory, index);
        });
    });
}

void renderer::MiningDrill(const MenuFunctionParams& params) {
    auto& logic_data         = params.logic;
    auto& player_data        = params.player;
    const auto& data_manager = params.protoManager;
    const auto* prototype    = params.prototype;
    auto* unique_data        = params.uniqueData;

    assert(prototype != nullptr);
    assert(unique_data != nullptr);
    const auto& drill_data = *static_cast<const data::MiningDrillData*>(unique_data);

    SetupNextWindowLeft();
    PlayerInventoryMenu(player_data, data_manager);

    SetupNextWindowRight();

    ImGuard guard{};
    guard.Begin("_mining_drill", nullptr, kMenuFlags);

    DrawTitleBar(prototype->GetLocalizedName());

    ImGui::ProgressBar(GetProgressBarFraction(logic_data.GameTick(), drill_data.deferralEntry, drill_data.miningTicks));
}

void renderer::AssemblyMachine(const MenuFunctionParams& params) {
    auto& logic_data         = params.logic;
    auto& player_data        = params.player;
    const auto& data_manager = params.protoManager;
    const auto* prototype    = params.prototype;
    auto* unique_data        = params.uniqueData;

    assert(prototype != nullptr);
    assert(unique_data != nullptr);


    const auto& machine_proto = *static_cast<const data::AssemblyMachine*>(prototype);
    auto& machine_data        = *static_cast<data::AssemblyMachineData*>(unique_data);

    auto menu_data = GetMenuData();

    if (machine_data.HasRecipe()) {
        const auto window_size = GetWindowSize();

        SetupNextWindowLeft(window_size);
        PlayerInventoryMenu(player_data, data_manager);

        SetupNextWindowRight();

        ImGuard guard{};
        guard.Begin("_assembly_machine", nullptr, kMenuFlags);

        DrawTitleBar(prototype->GetLocalizedName());


        bool button_hovered = false;

        // Ingredients
        RemoveItemSlotTopPadding();
        DrawSlots(10, machine_data.ingredientInv.size() + 1, 1, [&](auto index, bool& /*slot_hover*/) {
            // Recipe change button
            if (index == machine_data.ingredientInv.size()) {
                auto* reset_icon = data_manager.DataRawGet<data::Item>(data::Item::kResetIname);
                assert(reset_icon != nullptr);

                DrawItemSlot(menu_data, 1, reset_icon->sprite->internalId, 0, button_hovered, [&]() {
                    if (ImGui::IsItemClicked()) {
                        machine_data.ChangeRecipe(logic_data, data_manager, nullptr);
                    }
                });
                return;
            }

            // Item which is required
            auto* ingredient_item =
                data_manager.DataRawGet<data::Item>(machine_data.GetRecipe()->ingredients[index].first);
            assert(ingredient_item != nullptr);

            // Amount of item possessed

            DrawItemSlot(menu_data,
                         1,
                         ingredient_item->sprite->internalId,
                         machine_data.ingredientInv[index].count,
                         button_hovered,
                         [&]() {
                             ImplementInventoryIsItemClicked(
                                 player_data, data_manager, machine_data.ingredientInv, index, [&]() {
                                     machine_proto.TryBeginCrafting(logic_data, machine_data);
                                 });

                             if (ImGui::IsItemHovered() && !button_hovered) {
                                 auto* recipe = data::Recipe::GetItemRecipe(
                                     data_manager, machine_data.GetRecipe()->ingredients[index].first);

                                 if (recipe)
                                     RecipeHoverTooltip<false>(player_data, data_manager, *recipe);
                             }
                         });
        });

        // User may have clicked the reset button, and a recipe no longer exists
        if (!machine_data.HasRecipe())
            return;

        // Progress
        const auto original_cursor_y = ImGui::GetCursorPosY();
        ImGui::SetCursorPosY(original_cursor_y + core::SafeCast<float>(kGuiStyleTitlebarPaddingY) / 2);

        const auto progress = GetProgressBarFraction(
            logic_data.GameTick(),
            machine_data.deferralEntry,
            core::SafeCast<float>(machine_data.GetRecipe()->GetCraftingTime(machine_proto.assemblySpeed)));

        ImGui::ProgressBar(progress, {window_size.x - 2 * kInventorySlotWidth, 0});

        ImGui::SameLine();
        ImGui::SetCursorPosY(original_cursor_y);

        // Product
        DrawSlots(10, 1, 1, [&](auto /*slot_index*/, bool& /*slot_hover*/) {
            auto* product_item = data_manager.DataRawGet<data::Item>(machine_data.GetRecipe()->product.first);

            assert(product_item != nullptr);
            DrawItemSlot(menu_data,
                         1,
                         product_item->sprite->internalId,
                         machine_data.productInv[0].count,
                         button_hovered,
                         [&]() {
                             ImplementInventoryIsItemClicked(
                                 player_data, data_manager, machine_data.productInv, 0, [&]() {
                                     machine_proto.TryBeginCrafting(logic_data, machine_data);
                                 });

                             if (ImGui::IsItemHovered() && !button_hovered) {
                                 const auto* recipe =
                                     data::Recipe::GetItemRecipe(data_manager, machine_data.GetRecipe()->product.first);
                                 assert(recipe != nullptr);
                                 RecipeHoverTooltip<false>(player_data, data_manager, *recipe);
                             }
                         });
        });
    }
    else {
        // Only draw recipe menu if no recipe is selected for assembling
        SetupNextWindowCenter();
        RecipeMenu(player_data, data_manager, prototype->GetLocalizedName(), [&](auto& recipe, auto& button_hovered) {
            if (ImGui::IsItemClicked()) {
                machine_data.ChangeRecipe(logic_data, data_manager, &recipe);
            }

            if (ImGui::IsItemHovered() && !button_hovered)
                RecipeHoverTooltip<false>(player_data, data_manager, recipe);
        });
    }
}
