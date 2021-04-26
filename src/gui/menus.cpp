// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "gui/menus.h"

#include <functional>
#include <sstream>

#include "core/utility.h"
#include "game/input/mouse_selection.h"
#include "game/logic/logic.h"
#include "game/logistic/inventory.h"
#include "game/player/player.h"
#include "gui/components.h"
#include "gui/context.h"
#include "gui/menu_data.h"
#include "proto/assembly_machine.h"
#include "proto/container_entity.h"
#include "proto/mining_drill.h"
#include "proto/recipe.h"
#include "proto/recipe_category.h"
#include "proto/recipe_group.h"
#include "proto/sprite.h"
#include "render/renderer.h"

using namespace jactorio;

/// Implements ImGui::IsItemClicked() for left and right mouse buttons
/// \param on_click Called after inventory actions were handled
template <bool HalfSelectOnLeft = false, bool HalfSelectOnRight = true>
void HandleInvClicked(
    game::Player& player,
    const data::PrototypeManager& proto,
    game::Inventory& inv,
    const size_t index,
    const std::function<void()>& on_click = []() {}) {
    if (ImGui::IsItemClicked()) {
        player.inventory.HandleInventoryActions(proto, inv, index, HalfSelectOnLeft);
        on_click();
    }
    else if (ImGui::IsItemClicked(1)) {
        player.inventory.HandleInventoryActions(proto, inv, index, HalfSelectOnRight);
        on_click();
    }
}

template <bool HalfSelectOnLeft = false, bool HalfSelectOnRight = true>
void HandleInvClicked(
    const gui::Context& context,
    game::Inventory& inv,
    const size_t index,
    const std::function<void()>& on_click = []() {}) {

    HandleInvClicked<HalfSelectOnLeft, HalfSelectOnRight>(context.player, context.proto, inv, index, on_click);
}

float GetProgressBarFraction(const GameTickT game_tick,
                             const game::DeferralTimer::DeferralEntry& entry,
                             const float total_ticks) {
    if (!entry.Valid())
        return 0.f;

    const auto ticks_left = SafeCast<long double>(entry.dueTick) - game_tick;
    return 1.f - LossyCast<float>(ticks_left / total_ticks);
}

// ==========================================================================================
// Player menus (Excluding entity menus)

/// Draws the player's inventory menu
void PlayerInventoryMenu(const gui::Context& context) {
    const gui::GuiMenu menu;
    menu.Begin("_character");

    const gui::GuiTitle title;
    title.Begin("Character");


    auto& player_inv = context.player.inventory.inventory;

    gui::GuiItemSlots item_slots(context);
    item_slots.Begin(player_inv.Size(), [&](auto index) {
        const auto& stack = player_inv[index];

        item_slots.DrawSlot(stack, [&]() {
            HandleInvClicked(context, player_inv, index);

            if (ImGui::IsItemHovered() && stack.count != 0) {
                gui::DrawCursorTooltip(context.player.inventory.GetSelectedItem() != nullptr,
                                       stack.item->GetLocalizedName(),
                                       stack.item->GetLocalizedDescription(),
                                       [&]() {
                                           gui::ImGuard tooltip_guard;

                                           tooltip_guard.PushStyleColor(ImGuiCol_Text, gui::kGuiColNone);
                                           ImGui::TextUnformatted(stack.item->GetLocalizedName().c_str());
                                       });
            }
        });
    });
}

void RecipeMenu(const gui::Context context,
                const std::string& title,
                const std::function<void(const proto::Recipe& recipe)>& item_slot_draw) {
    auto& player      = context.player;
    const auto& proto = context.proto;

    const gui::GuiMenu menu;
    menu.Begin("_recipe");

    const gui::GuiTitle gui_title;
    ;
    gui_title.Begin(title, [&]() {
        constexpr ImU32 search_bar_padding = 4;

        const auto original_cursor_y = ImGui::GetCursorPosY();

        // Vertically center title text with search bar
        ImGui::SameLine();

        const auto title_bar_height  = original_cursor_y + gui::kGuiStyleFramePaddingY;
        const auto search_bar_height = gui::GetFontHeight() + search_bar_padding * 2;
        ImGui::SetCursorPosY(title_bar_height / 2 - search_bar_height / 2);

        {
            gui::ImGuard guard;
            guard.PushStyleVar(ImGuiStyleVar_FramePadding, {gui::kGuiStyleWindowPaddingX, search_bar_padding});

            auto& search_text = player.crafting.recipeSearchText;
            search_text.resize(100);
            ImGui::InputText("", search_text.data(), search_text.size());
        }

        ImGui::SetCursorPosY(original_cursor_y);
    });


    auto matches_search_str = [](const std::string& item_name, const std::string& str) {
        if (str.empty())
            return true;

        const auto compare_item_name = StrToLower(item_name);
        const auto compare_str       = StrToLower(StrTrimWhitespace(str));

        return compare_item_name.find(compare_str) != std::string::npos;
    };

    // Menu groups
    auto groups = proto.GetAllSorted<proto::RecipeGroup>();

    gui::GuiItemSlots group_slots(context);
    group_slots.slotSpan = 5;
    group_slots.scale    = 2;
    group_slots.Begin(groups.size(), [&](const uint16_t index) {
        const auto& recipe_group = groups[index];

        // Only draw item categories matching searched product name
        for (auto& recipe_category : recipe_group->recipeCategories) {
            for (auto& recipe : recipe_category->recipes) {
                const auto& product_name = proto.Get<proto::Item>(recipe->product.first)->GetLocalizedName();

                if (matches_search_str(product_name, player.crafting.recipeSearchText))
                    goto loop_exit;
            }
        }
        return;

    loop_exit:
        // Different color for currently selected recipe group
        gui::ImGuard recipe_group_guard;
        if (index == player.crafting.RecipeGroupGetSelected())
            recipe_group_guard.PushStyleColor(ImGuiCol_Button, gui::kGuiColButtonHover);

        group_slots.DrawSlot(recipe_group->sprite->internalId, [&]() {
            if (ImGui::IsItemClicked())
                player.crafting.RecipeGroupSelect(index);

            // Item tooltip
            if (ImGui::IsItemHovered()) {
                gui::DrawCursorTooltip(player.inventory.GetSelectedItem() != nullptr,
                                       recipe_group->GetLocalizedName(),
                                       recipe_group->GetLocalizedDescription(),
                                       [&]() {});
            }
        });
    });

    // Menu recipes
    const auto& selected_group = groups[player.crafting.RecipeGroupGetSelected()];

    for (const auto& recipe_category : selected_group->recipeCategories) {
        const auto& recipes = recipe_category->recipes;

        gui::GuiItemSlots recipe_row(context);
        recipe_row.Begin(recipes.size(), [&](auto index) {
            const auto* recipe = recipes.at(index);

            const auto* product = proto.Get<proto::Item>(recipe->product.first);
            assert(product != nullptr); // Invalid recipe product

            if (!matches_search_str(product->GetLocalizedName(), player.crafting.recipeSearchText))
                return;

            recipe_row.DrawSlot(product->sprite->internalId, [&]() { item_slot_draw(*recipe); });
        });
    }
}

/// Draws preview tooltip for a recipe
/// \tparam IsPlayerCrafting Shows items possessed by the player and opportunities for intermediate crafting
template <bool IsPlayerCrafting>
void RecipeHoverTooltip(const gui::Context& context, const proto::Recipe& recipe) {
    auto& player      = context.player;
    const auto& proto = context.proto;

    auto* product_item = proto.Get<proto::Item>(recipe.product.first);
    assert(product_item);

    // Show the product yield in the title
    std::stringstream title_ss;
    title_ss << product_item->GetLocalizedName() << " (" << recipe.product.second << ")";


    gui::ImGuard guard;
    guard.PushStyleColor(ImGuiCol_Button, gui::kGuiColNone);

    gui::DrawCursorTooltip(player.inventory.GetSelectedItem() != nullptr, title_ss.str(), "Ingredients:", [&]() {
        // Ingredients
        for (const auto& [ingredient_name, amount_required] : recipe.ingredients) {
            const auto* item = proto.Get<proto::Item>(ingredient_name);

            gui::GuiItemSlots ingredient_row(context);
            ingredient_row.DrawSlot(item->sprite->internalId);

            ImGui::SameLine(gui::kInventorySlotWidth * 1.5);

            const auto player_item_count = player.inventory.inventory.Count(*item);
            // Does not have ingredient
            if (IsPlayerCrafting && player_item_count < amount_required) {
                const bool can_be_recurse_crafted = player.crafting.RecipeCanCraft(proto, recipe, 1);

                gui::ImGuard text_guard;
                if (can_be_recurse_crafted)
                    text_guard.PushStyleColor(ImGuiCol_Text, gui::kGuiColTextWarning);
                else
                    text_guard.PushStyleColor(ImGuiCol_Text, gui::kGuiColTextError);

                ImGui::Text("%d/%d x %s", player_item_count, amount_required, item->GetLocalizedName().c_str());
            }
            // Has enough
            else {
                ImGui::Text("%d x %s", amount_required, item->GetLocalizedName().c_str());
            }
        }
        ImGui::Text("%.1f seconds", recipe.craftingTime);

        // Total raw items
        ImGui::Separator();

        ImGui::Text("%s", "Total Raw:");

        auto raw_inames = proto::Recipe::RecipeGetTotalRaw(proto, product_item->name);

        gui::GuiItemSlots raw_item_slots(context);
        raw_item_slots.slotSpan            = 5;
        raw_item_slots.endingVerticalSpace = 0;
        raw_item_slots.Begin(raw_inames.size(), [&](const auto slot_index) {
            const auto* item = proto.Get<proto::Item>(raw_inames[slot_index].first);

            const auto item_count_required = raw_inames[slot_index].second;

            // const auto player_item_count =
            // 	game::GetInvItemCount(player.inventory.inventoryPlayer, player.inventory.kInventorySize,
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


void gui::MainMenu(const Context& /*context*/,
                   const proto::FrameworkBase* /*prototype*/,
                   proto::UniqueDataBase* /*unique_data*/) {
    assert(false); // To draw main menu, test for if menu is visible and call custom main menu draw function in
                   // render/main_menu.h
}

void gui::CharacterMenu(const Context& context,
                        const proto::FrameworkBase* /*prototype*/,
                        proto::UniqueDataBase* /*unique_data*/) {
    SetupNextWindowLeft();
    PlayerInventoryMenu(context);

    SetupNextWindowRight();
    RecipeMenu(context, "Recipe", [&](auto& recipe) {
        auto& player      = context.player;
        const auto& proto = context.proto;

        if (ImGui::IsItemClicked()) {
            if (player.crafting.RecipeCanCraft(proto, recipe, 1)) {
                player.crafting.RecipeCraftR(proto, recipe);
                player.inventory.inventory.Sort();
            }
        }

        if (ImGui::IsItemHovered())
            RecipeHoverTooltip<true>(context, recipe);
    });
}

void gui::CursorWindow(const Context& context,
                       const proto::FrameworkBase* /*prototype*/,
                       proto::UniqueDataBase* /*unique_data*/) {
    // Draw the tooltip of what is currently selected

    // Player has an item selected, draw it on the tooltip
    const auto* selected_stack = context.player.inventory.GetSelectedItem();

    if (selected_stack != nullptr) {
        ImGuard guard;
        guard.PushStyleColor(ImGuiCol_Border, kGuiColNone);
        guard.PushStyleColor(ImGuiCol_PopupBg, kGuiColNone);

        // Slightly off center so that user can still click
        ImGui::SetNextWindowPos({LossyCast<float>(game::MouseSelection::GetCursor().x),
                                 LossyCast<float>(game::MouseSelection::GetCursor().y) + 2.f});
        ImGui::SetNextWindowFocus();

        GuiMenu menu;
        menu.AppendFlags(ImGuiWindowFlags_NoBackground);
        menu.Begin("_selected_item");

        const auto& positions = context.menuData.spritePositions.at(selected_stack->item->sprite->internalId);

        ImGui::SameLine(10.f);
        ImGui::Image(reinterpret_cast<void*>(context.menuData.texId),
                     {kInventorySlotWidth, kInventorySlotWidth},
                     {positions.topLeft.x, positions.topLeft.y},
                     {positions.bottomRight.x, positions.bottomRight.y});

        ImGui::SameLine(10.f);
        ImGui::Text("%d", selected_stack->count);
    }
}

void gui::CraftingQueue(const Context& context,
                        const proto::FrameworkBase* /*prototype*/,
                        proto::UniqueDataBase* /*unique_data*/) {
    constexpr auto slot_span = 10;


    auto get_window_height = [slot_span](const std::size_t queued_items) {
        const auto y_slots = (queued_items + slot_span - 1) / slot_span; // Always round up for slot count

        auto window_height = y_slots * (kInventorySlotWidth + kInventorySlotPadding);
        window_height += kGuiStyleWindowPaddingY;

        const auto max_window_height = render::Renderer::GetWindowHeight() / 2; // Pixels
        if (window_height > max_window_height)
            window_height = max_window_height;

        return window_height;
    };


    const auto& recipe_queue = context.player.crafting.GetRecipeQueue();
    const auto window_height = get_window_height(recipe_queue.size());

    ImGui::SetNextWindowPos({0, SafeCast<float>(render::Renderer::GetWindowHeight() - window_height)});
    ImGui::SetNextWindowSize({GetTotalItemSlotWidth(slot_span) + GetTotalWindowPaddingX() + kGuiStyleScrollBarSize,
                              SafeCast<float>(window_height)});

    GuiMenu menu;
    menu.AppendFlags(ImGuiWindowFlags_NoMove, ImGuiWindowFlags_NoBackground, ImGuiWindowFlags_NoScrollWithMouse);
    menu.Begin("_crafting_queue");


    ImGuard guard;
    guard.PushStyleColor(ImGuiCol_Button, kGuiColNone);
    guard.PushStyleColor(ImGuiCol_Border, kGuiColNone);

    GuiItemSlots queued_item_row(context);
    queued_item_row.slotSpan = slot_span;
    queued_item_row.Begin(recipe_queue.size(), [&](auto index) {
        const auto* recipe = recipe_queue.at(index).Get();
        assert(recipe != nullptr);

        const auto* item = context.proto.Get<proto::Item>(recipe->product.first);
        queued_item_row.DrawSlot(item->sprite->internalId, recipe->product.second);
    });
}

float last_pickup_fraction = 0.f;

void gui::PickupProgressbar(const Context& context,
                            const proto::FrameworkBase* /*prototype*/,
                            proto::UniqueDataBase* /*unique_data*/) {
    constexpr float progress_bar_width  = 260 * 2;
    constexpr float progress_bar_height = 13;

    const float pickup_fraction = context.player.placement.GetPickupPercentage();
    // Do not draw progress bar if 0 or has not moved since last tick
    if (pickup_fraction == 0 || last_pickup_fraction == pickup_fraction)
        return;
    last_pickup_fraction = pickup_fraction;


    ImGui::SetNextWindowSize({progress_bar_width, progress_bar_height});
    ImGui::SetNextWindowPos(
        {SafeCast<float>(render::Renderer::GetWindowWidth()) / 2 - (progress_bar_width / 2), // Center X
         SafeCast<float>(render::Renderer::GetWindowHeight()) - progress_bar_height});

    // Window
    GuiMenu menu;
    menu.AppendFlags(ImGuiWindowFlags_NoBackground, ImGuiWindowFlags_NoMove);
    menu.Begin("_entity_pickup_status");

    ImGuard guard;
    guard.PushStyleColor(ImGuiCol_Text, kGuiColNone);
    guard.PushStyleColor(ImGuiCol_FrameBg, kGuiColProgressBg);
    guard.PushStyleColor(ImGuiCol_PlotHistogram, kGuiColProgress);

    ImGui::ProgressBar(pickup_fraction, {progress_bar_width, progress_bar_height});
}

// ==========================================================================================
// Entity menus
void gui::ContainerEntity(const Context& context,
                          const proto::FrameworkBase* prototype,
                          proto::UniqueDataBase* unique_data) {
    assert(prototype != nullptr);
    assert(unique_data != nullptr);

    auto& container_data = *SafeCast<proto::ContainerEntityData*>(unique_data);


    SetupNextWindowLeft();
    PlayerInventoryMenu(context);

    SetupNextWindowRight();
    const GuiMenu menu;
    menu.Begin("_e_container");

    const GuiTitle title;
    title.Begin(prototype->GetLocalizedName());

    GuiItemSlots inv_slots(context);
    inv_slots.Begin(container_data.inventory.Size(), [&](auto index) {
        inv_slots.DrawSlot(container_data.inventory[index],
                           [&]() { HandleInvClicked(context, container_data.inventory, index); });
    });
}

void gui::MiningDrill(const Context& context,
                      const proto::FrameworkBase* prototype,
                      proto::UniqueDataBase* unique_data) {
    assert(prototype != nullptr);
    assert(unique_data != nullptr);

    const auto& drill_data = *SafeCast<const proto::MiningDrillData*>(unique_data);


    SetupNextWindowLeft();
    PlayerInventoryMenu(context);

    SetupNextWindowRight();

    const GuiMenu menu;
    menu.Begin("_e_mining_drill");

    const GuiTitle title;
    title.Begin(prototype->GetLocalizedName());

    ImGui::ProgressBar(
        GetProgressBarFraction(context.logic.GameTick(), drill_data.deferralEntry, drill_data.miningTicks));
}

void gui::AssemblyMachine(const Context& context,
                          const proto::FrameworkBase* prototype,
                          proto::UniqueDataBase* unique_data) {
    assert(prototype != nullptr);
    assert(unique_data != nullptr);

    auto& logic       = context.logic;
    const auto& proto = context.proto;

    const auto& machine_proto = *SafeCast<const proto::AssemblyMachine*>(prototype);
    auto& machine_data        = *SafeCast<proto::AssemblyMachineData*>(unique_data);

    if (machine_data.HasRecipe()) {
        const auto window_size = GetWindowSize();

        SetupNextWindowLeft(window_size);
        PlayerInventoryMenu(context);

        SetupNextWindowRight();

        const GuiMenu menu;
        menu.Begin("_e_assembly_machine");

        const GuiTitle title;
        title.Begin(prototype->GetLocalizedName());


        // Ingredients
        GuiItemSlots ingredient_slots(context);
        ingredient_slots.Begin(machine_data.ingredientInv.Size() + 1, [&](auto index) {
            // Recipe change button
            if (index == machine_data.ingredientInv.Size()) {
                auto* reset_icon = proto.Get<proto::Item>(proto::Item::kResetIname);
                assert(reset_icon != nullptr);

                ingredient_slots.DrawSlot(reset_icon->sprite->internalId, [&]() {
                    if (ImGui::IsItemClicked()) {
                        machine_data.ChangeRecipe(logic, proto, nullptr);
                    }
                });
                return;
            }

            // Item which is required
            auto* ingredient_item = proto.Get<proto::Item>(machine_data.GetRecipe()->ingredients[index].first);
            assert(ingredient_item != nullptr);

            // Amount of item possessed

            ingredient_slots.DrawSlot(
                ingredient_item->sprite->internalId, machine_data.ingredientInv[index].count, [&]() {
                    HandleInvClicked(context, machine_data.ingredientInv, index, [&]() {
                        machine_proto.TryBeginCrafting(logic, machine_data);
                    });

                    if (ImGui::IsItemHovered()) {
                        auto* recipe =
                            proto::Recipe::GetItemRecipe(proto, machine_data.GetRecipe()->ingredients[index].first);

                        if (recipe)
                            RecipeHoverTooltip<false>(context, *recipe);
                    }
                });
        });

        // User may have clicked the reset button, and a recipe no longer exists
        if (!machine_data.HasRecipe())
            return;


        // Progress
        const auto progress = GetProgressBarFraction(
            logic.GameTick(),
            machine_data.deferralEntry,
            SafeCast<float>(machine_data.GetRecipe()->GetCraftingTime(machine_proto.assemblySpeed)));

        ImGui::ProgressBar(
            progress,
            {window_size.x - GetTotalWindowPaddingX() - GetTotalItemSlotWidth(1) - GetTotalWindowItemSpacingX(1), 0});

        ImGui::SameLine();

        // Product
        GuiItemSlots product_slots(context);
        product_slots.Begin(1, [&](auto /*slot_index*/) {
            auto* product_item = proto.Get<proto::Item>(machine_data.GetRecipe()->product.first);

            assert(product_item != nullptr);
            product_slots.DrawSlot(product_item->sprite->internalId, machine_data.productInv[0].count, [&]() {
                HandleInvClicked(context, machine_data.productInv, 0, [&]() {
                    machine_proto.TryBeginCrafting(logic, machine_data);
                });

                if (ImGui::IsItemHovered()) {
                    const auto* recipe = proto::Recipe::GetItemRecipe(proto, machine_data.GetRecipe()->product.first);
                    assert(recipe != nullptr);

                    RecipeHoverTooltip<false>(context, *recipe);
                }
            });
        });
    }
    else {
        // Only draw recipe menu if no recipe is selected for assembling
        SetupNextWindowCenter();
        RecipeMenu(context, prototype->GetLocalizedName(), [&](auto& recipe) {
            if (ImGui::IsItemClicked()) {
                machine_data.ChangeRecipe(logic, proto, &recipe);
            }

            if (ImGui::IsItemHovered())
                RecipeHoverTooltip<false>(context, recipe);
        });
    }
}
