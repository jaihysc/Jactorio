// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/gui/gui_menus.h"

#include <functional>
#include <sstream>

#include "data/prototype/assembly_machine.h"
#include "data/prototype/container_entity.h"
#include "data/prototype/mining_drill.h"
#include "data/prototype/recipe_group.h"

#include "game/input/mouse_selection.h"
#include "game/logic/inventory_controller.h"
#include "game/player/player_data.h"

#include "render/gui/components.h"
#include "render/gui/gui_colors.h"

using namespace jactorio;

///
/// Implements ImGui::IsItemClicked() for left and right mouse buttons
/// \param callback Called after inventory actions were handled
template <bool HalfSelectOnLeft = false, bool HalfSelectOnRight = true>
void HandleInvClicked(
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

template <bool HalfSelectOnLeft = false, bool HalfSelectOnRight = true>
void HandleInvClicked(
    const render::GuiRenderer& g_rendr,
    data::Item::Inventory& inv,
    const size_t index,
    const std::function<void()>& on_click = []() {}) {

    HandleInvClicked<HalfSelectOnLeft, HalfSelectOnRight>(g_rendr.player, g_rendr.protoManager, inv, index, on_click);
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
void PlayerInventoryMenu(const render::GuiRenderer& g_rendr) {
    render::GuiMenu menu;
    menu.Begin("_character", nullptr, kMenuFlags); // TODO handle this

    menu.DrawTitleBar("Character");


    auto& player_inv = g_rendr.player.inventory.inventory;

    auto item_slots = g_rendr.MakeComponent<render::GuiSlotRenderer>();
    item_slots.Begin(player_inv.size(), [&](auto index) {
        const auto& stack = player_inv[index];

        item_slots.DrawSlot(stack, [&]() {
            HandleInvClicked(g_rendr, player_inv, index);

            if (ImGui::IsItemHovered() && stack.count != 0) {
                render::DrawCursorTooltip(g_rendr.player.inventory.GetSelectedItem() != nullptr,
                                          stack.item->GetLocalizedName().c_str(),
                                          "sample description",
                                          [&]() {
                                              render::ImGuard tooltip_guard;

                                              tooltip_guard.PushStyleColor(ImGuiCol_Text, render::kGuiColNone);
                                              ImGui::TextUnformatted(stack.item->GetLocalizedName().c_str());
                                          });
            }
        });
    });
}

void RecipeMenu(const render::GuiRenderer g_rendr,
                const std::string& title,
                const std::function<void(const data::Recipe& recipe, bool& button_hovered)>& item_slot_draw) {
    auto& player_data         = g_rendr.player;
    const auto& proto_manager = g_rendr.protoManager;

    render::GuiMenu menu;
    menu.Begin("_recipe", nullptr, kMenuFlags);

    // Title with search bar
    menu.DrawTitleBar(title, [&]() {
        // Vertically center title text with search bar
        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - core::SafeCast<float>(render::kGuiStyleTitlebarPaddingY) / 2);

        render::ImGuard title_guard;
        title_guard.PushStyleVar(
            ImGuiStyleVar_FramePadding,
            {render::kGuiStyleWindowPaddingX, core::SafeCast<float>(render::kGuiStyleTitlebarPaddingY) / 2});


        auto& search_text = player_data.crafting.recipeSearchText;
        search_text.resize(100);
        ImGui::InputText("", search_text.data(), search_text.size());

        // Continue title bar calculations from where the label text was
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - core::SafeCast<float>(render::kGuiStyleTitlebarPaddingY) / 2);
    });


    auto matches_search_str = [](const std::string& item_name, const std::string& str) {
        if (str.empty())
            return true;

        auto compare_item_name = core::StrToLower(item_name);
        auto compare_str       = core::StrToLower(core::StrTrimWhitespace(str));

        return compare_item_name.find(compare_str) != std::string::npos;
    };

    // Menu groups
    auto groups = proto_manager.DataRawGetAllSorted<data::RecipeGroup>(data::DataCategory::recipe_group);

    auto group_slots     = g_rendr.MakeComponent<render::GuiSlotRenderer>();
    group_slots.slotSpan = 5;
    group_slots.scale    = 2;
    group_slots.Begin(groups.size(), [&](const uint16_t index) {
        const auto& recipe_group = groups[index];

        // Only draw item categories matching searched product name
        for (auto& recipe_category : recipe_group->recipeCategories) {
            for (auto& recipe : recipe_category->recipes) {
                const auto& product_name =
                    proto_manager.DataRawGet<data::Item>(recipe->product.first)->GetLocalizedName();

                if (matches_search_str(product_name, player_data.crafting.recipeSearchText))
                    goto loop_exit;
            }
        }
        return;

    loop_exit:
        // Different color for currently selected recipe group
        render::ImGuard recipe_group_guard;
        if (index == player_data.crafting.RecipeGroupGetSelected())
            recipe_group_guard.PushStyleColor(ImGuiCol_Button, render::kGuiColButtonHover);

        group_slots.DrawSlot(recipe_group->sprite->internalId, [&]() {
            if (ImGui::IsItemClicked())
                player_data.crafting.RecipeGroupSelect(index);

            // TODO abstract this somewhere
            // Item tooltip
            std::stringstream description_ss;
            description_ss << recipe_group->GetLocalizedDescription().c_str();
            render::FitTitle(description_ss, recipe_group->GetLocalizedName().size());

            if (ImGui::IsItemHovered()) {
                render::DrawCursorTooltip(player_data.inventory.GetSelectedItem() != nullptr,
                                          recipe_group->GetLocalizedName().c_str(),
                                          description_ss.str().c_str(),
                                          [&]() {});
            }
        });
    });

    // Menu recipes
    const auto& selected_group = groups[player_data.crafting.RecipeGroupGetSelected()];

    bool button_hovered = false; // todo remove this
    for (const auto& recipe_category : selected_group->recipeCategories) {
        const auto& recipes = recipe_category->recipes;

        auto recipe_row = g_rendr.MakeComponent<render::GuiSlotRenderer>();
        recipe_row.Begin(recipes.size(), [&](auto index) {
            const auto* recipe = recipes.at(index);

            const auto* product = proto_manager.DataRawGet<data::Item>(recipe->product.first);
            assert(product != nullptr); // Invalid recipe product

            if (!matches_search_str(product->GetLocalizedName(), player_data.crafting.recipeSearchText))
                return;

            recipe_row.DrawSlot(product->sprite->internalId, [&]() { item_slot_draw(*recipe, button_hovered); });
        });
    }
}

///
/// Draws preview tooltip for a recipe
/// \tparam IsPlayerCrafting Shows items possessed by the player and opportunities for intermediate crafting
template <bool IsPlayerCrafting>
void RecipeHoverTooltip(const render::GuiRenderer& g_rendr, const data::Recipe& recipe) {
    auto& player_data         = g_rendr.player;
    const auto& proto_manager = g_rendr.protoManager;

    auto* product_item = proto_manager.DataRawGet<data::Item>(recipe.product.first);
    assert(product_item);

    std::stringstream title_ss;
    // Show the product yield in the title
    title_ss << product_item->GetLocalizedName().c_str() << " (" << recipe.product.second << ")";

    std::stringstream description_ss;
    description_ss << "Ingredients:";
    render::FitTitle(description_ss, title_ss.str().size());


    render::ImGuard guard;
    guard.PushStyleColor(ImGuiCol_Button, render::kGuiColNone);

    render::DrawCursorTooltip(
        player_data.inventory.GetSelectedItem() != nullptr,
        title_ss.str().c_str(),
        description_ss.str().c_str(),
        [&]() {
            // Ingredients
            for (const auto& ingredient_pair : recipe.ingredients) {
                const auto* item = proto_manager.DataRawGet<data::Item>(ingredient_pair.first);

                auto ingredient_row = g_rendr.MakeComponent<render::GuiSlotRenderer>();
                ingredient_row.DrawSlot(item->sprite->internalId);

                ImGui::SameLine(render::kInventorySlotWidth * 1.5);

                const auto player_item_count = game::GetInvItemCount(player_data.inventory.inventory, item);
                // Does not have ingredient
                if (IsPlayerCrafting && player_item_count < ingredient_pair.second) {
                    const bool can_be_recurse_crafted = player_data.crafting.RecipeCanCraft(proto_manager, recipe, 1);

                    render::ImGuard text_guard;
                    if (can_be_recurse_crafted)
                        text_guard.PushStyleColor(ImGuiCol_Text, render::kGuiColTextWarning);
                    else
                        text_guard.PushStyleColor(ImGuiCol_Text, render::kGuiColTextError);

                    ImGui::Text(
                        "%d/%d x %s", player_item_count, ingredient_pair.second, item->GetLocalizedName().c_str());
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

            auto raw_inames = data::Recipe::RecipeGetTotalRaw(proto_manager, product_item->name);

            auto raw_item_slots                = g_rendr.MakeComponent<render::GuiSlotRenderer>();
            raw_item_slots.slotSpan            = 5;
            raw_item_slots.endingVerticalSpace = 0;
            raw_item_slots.Begin(raw_inames.size(), [&](const auto slot_index) {
                const auto* item = proto_manager.DataRawGet<data::Item>(raw_inames[slot_index].first);

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

                raw_item_slots.DrawSlot(item->sprite->internalId, item_count_required);
            });
        });
}

// ======================================================================

void render::CharacterMenu(const GuiRenderer& g_rendr) {
    SetupNextWindowLeft();
    PlayerInventoryMenu(g_rendr);

    SetupNextWindowRight();
    RecipeMenu(g_rendr, "Recipe", [&](auto& recipe, auto& button_hovered) {
        auto& player              = g_rendr.player;
        const auto& proto_manager = g_rendr.protoManager;

        if (ImGui::IsItemClicked()) {
            if (player.crafting.RecipeCanCraft(proto_manager, recipe, 1)) {
                player.crafting.RecipeCraftR(proto_manager, recipe);
                player.inventory.InventorySort(player.inventory.inventory);
            }
        }

        if (ImGui::IsItemHovered() && !button_hovered)
            RecipeHoverTooltip<true>(g_rendr, recipe);
    });
}

void render::CursorWindow(const GuiRenderer& g_rendr) {
    // Draw the tooltip of what is currently selected

    // Player has an item selected, draw it on the tooltip
    const auto* selected_stack = g_rendr.player.inventory.GetSelectedItem();

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

        render::GuiMenu menu;
        ImGui::SetNextWindowFocus();
        menu.Begin("_selected_item", nullptr, flags);

        const auto& positions = g_rendr.menuData.spritePositions.at(selected_stack->item->sprite->internalId);

        ImGui::SameLine(10.f);
        ImGui::Image(reinterpret_cast<void*>(g_rendr.menuData.texId),
                     ImVec2(32, 32),

                     ImVec2(positions.topLeft.x, positions.topLeft.y),
                     ImVec2(positions.bottomRight.x, positions.bottomRight.y));

        ImGui::SameLine(10.f);
        ImGui::Text("%d", selected_stack->count);
    }
}

void render::CraftingQueue(const GuiRenderer& g_rendr) {
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoBackground;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoScrollbar;
    flags |= ImGuiWindowFlags_NoScrollWithMouse;

    const auto& recipe_queue = g_rendr.player.crafting.GetRecipeQueue();


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
    render::GuiMenu menu;
    menu.Begin("_crafting_queue", nullptr, flags);

    ImGuard guard;
    guard.PushStyleColor(ImGuiCol_Button, kGuiColNone);
    guard.PushStyleColor(ImGuiCol_Border, kGuiColNone);

    auto queued_item_row = g_rendr.MakeComponent<render::GuiSlotRenderer>();
    queued_item_row.Begin(recipe_queue.size(), [&](auto index) {
        const auto* recipe = recipe_queue.at(index).Get();
        assert(recipe != nullptr);

        const auto* item = g_rendr.protoManager.DataRawGet<data::Item>(recipe->product.first);
        queued_item_row.DrawSlot(item->sprite->internalId, recipe->product.second);
    });
}

float last_pickup_fraction = 0.f;

void render::PickupProgressbar(const GuiRenderer& g_rendr) {
    constexpr float progress_bar_width  = 260 * 2;
    constexpr float progress_bar_height = 13;

    const float pickup_fraction = g_rendr.player.placement.GetPickupPercentage();
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
    GuiMenu menu;
    menu.Begin("_entity_pickup_status", nullptr, flags);

    ImGuard guard;
    guard.PushStyleColor(ImGuiCol_Text, kGuiColNone);
    guard.PushStyleColor(ImGuiCol_FrameBg, kGuiColProgressBg);
    guard.PushStyleColor(ImGuiCol_PlotHistogram, kGuiColProgress);

    ImGui::ProgressBar(pickup_fraction, ImVec2(progress_bar_width, progress_bar_height));
}

// ==========================================================================================
// Entity menus
void render::ContainerEntity(const GuiRenderer& g_rendr) {
    SetupNextWindowLeft();
    PlayerInventoryMenu(g_rendr);

    SetupNextWindowRight();

    GuiMenu menu;
    menu.Begin("_container", nullptr, kMenuFlags);


    const auto* prototype = g_rendr.prototype;
    assert(prototype != nullptr);
    menu.DrawTitleBar(prototype->GetLocalizedName());

    auto* unique_data = g_rendr.uniqueData;
    assert(unique_data != nullptr);
    auto& container_data = *static_cast<data::ContainerEntityData*>(unique_data);

    auto inv_slots = g_rendr.MakeComponent<render::GuiSlotRenderer>();
    inv_slots.Begin(container_data.inventory.size(), [&](auto index) {
        const auto sprite_id = container_data.inventory[index].item != nullptr
            ? container_data.inventory[index].item->sprite->internalId
            : 0;

        inv_slots.DrawSlot(sprite_id, container_data.inventory[index].count, [&]() {
            HandleInvClicked(g_rendr, container_data.inventory, index);
        });
    });
}

void render::MiningDrill(const GuiRenderer& g_rendr) {
    auto* unique_data = g_rendr.uniqueData;
    assert(unique_data != nullptr);
    const auto& drill_data = *static_cast<const data::MiningDrillData*>(unique_data);

    SetupNextWindowLeft();
    PlayerInventoryMenu(g_rendr);

    SetupNextWindowRight();

    GuiMenu menu;
    menu.Begin("_mining_drill", nullptr, kMenuFlags);

    const auto* prototype = g_rendr.prototype;
    assert(prototype != nullptr);
    menu.DrawTitleBar(prototype->GetLocalizedName());

    ImGui::ProgressBar(
        GetProgressBarFraction(g_rendr.logic.GameTick(), drill_data.deferralEntry, drill_data.miningTicks));
}

void render::AssemblyMachine(const GuiRenderer& g_rendr) {
    auto& logic               = g_rendr.logic;
    const auto& proto_manager = g_rendr.protoManager;

    const auto* prototype = g_rendr.prototype;
    assert(prototype != nullptr);

    auto* unique_data = g_rendr.uniqueData;
    assert(unique_data != nullptr);


    const auto& machine_proto = *static_cast<const data::AssemblyMachine*>(prototype);
    auto& machine_data        = *static_cast<data::AssemblyMachineData*>(unique_data);

    if (machine_data.HasRecipe()) {
        const auto window_size = GetWindowSize();

        SetupNextWindowLeft(window_size);
        PlayerInventoryMenu(g_rendr);

        SetupNextWindowRight();

        GuiMenu menu;
        menu.Begin("_assembly_machine", nullptr, kMenuFlags);

        menu.DrawTitleBar(prototype->GetLocalizedName());


        bool button_hovered = false;

        // Ingredients
        auto ingredient_slots = g_rendr.MakeComponent<render::GuiSlotRenderer>();
        ingredient_slots.Begin(machine_data.ingredientInv.size() + 1, [&](auto index) {
            // Recipe change button
            if (index == machine_data.ingredientInv.size()) {
                auto* reset_icon = proto_manager.DataRawGet<data::Item>(data::Item::kResetIname);
                assert(reset_icon != nullptr);

                ingredient_slots.DrawSlot(reset_icon->sprite->internalId, [&]() {
                    if (ImGui::IsItemClicked()) {
                        machine_data.ChangeRecipe(logic, proto_manager, nullptr);
                    }
                });
                return;
            }

            // Item which is required
            auto* ingredient_item =
                proto_manager.DataRawGet<data::Item>(machine_data.GetRecipe()->ingredients[index].first);
            assert(ingredient_item != nullptr);

            // Amount of item possessed

            ingredient_slots.DrawSlot(
                ingredient_item->sprite->internalId, machine_data.ingredientInv[index].count, [&]() {
                    HandleInvClicked(g_rendr, machine_data.ingredientInv, index, [&]() {
                        machine_proto.TryBeginCrafting(logic, machine_data);
                    });

                    if (ImGui::IsItemHovered() && !button_hovered) {
                        auto* recipe = data::Recipe::GetItemRecipe(proto_manager,
                                                                   machine_data.GetRecipe()->ingredients[index].first);

                        if (recipe)
                            RecipeHoverTooltip<false>(g_rendr, *recipe);
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
            logic.GameTick(),
            machine_data.deferralEntry,
            core::SafeCast<float>(machine_data.GetRecipe()->GetCraftingTime(machine_proto.assemblySpeed)));

        ImGui::ProgressBar(progress, {window_size.x - 2 * kInventorySlotWidth, 0});

        ImGui::SameLine();
        ImGui::SetCursorPosY(original_cursor_y);


        // Product
        auto product_slots = g_rendr.MakeComponent<render::GuiSlotRenderer>();
        product_slots.Begin(1, [&](auto /*slot_index*/) {
            auto* product_item = proto_manager.DataRawGet<data::Item>(machine_data.GetRecipe()->product.first);

            assert(product_item != nullptr);
            product_slots.DrawSlot(product_item->sprite->internalId, machine_data.productInv[0].count, [&]() {
                HandleInvClicked(g_rendr, machine_data.productInv, 0, [&]() {
                    machine_proto.TryBeginCrafting(logic, machine_data);
                });

                if (ImGui::IsItemHovered() && !button_hovered) {
                    const auto* recipe =
                        data::Recipe::GetItemRecipe(proto_manager, machine_data.GetRecipe()->product.first);
                    assert(recipe != nullptr);

                    RecipeHoverTooltip<false>(g_rendr, *recipe);
                }
            });
        });
    }
    else {
        // Only draw recipe menu if no recipe is selected for assembling
        SetupNextWindowCenter();
        RecipeMenu(g_rendr, prototype->GetLocalizedName(), [&](auto& recipe, auto& button_hovered) {
            if (ImGui::IsItemClicked()) {
                machine_data.ChangeRecipe(logic, proto_manager, &recipe);
            }

            if (ImGui::IsItemHovered() && !button_hovered)
                RecipeHoverTooltip<false>(g_rendr, recipe);
        });
    }
}
