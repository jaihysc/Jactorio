// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#ifndef JACTORIO_INCLUDE_GUI_COMPONENTS_H
#define JACTORIO_INCLUDE_GUI_COMPONENTS_H
#pragma once

#include <functional>
#include <type_traits>

#include "core/data_type.h"
#include "gui/component_base.h"
#include "gui/layout.h"

namespace jactorio::game
{
    class ItemStack;
}

namespace jactorio::gui
{
    struct Context;

    class GuiMenu
    {
        static constexpr ImGuiWindowFlags kDefaultFlags =
            0 | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    public:
        GuiMenu() = default;
        ~GuiMenu();

        GuiMenu(const GuiMenu&)  = delete;
        GuiMenu(const GuiMenu&&) = delete;

        GuiMenu& operator=(const GuiMenu&) = delete;
        GuiMenu& operator=(const GuiMenu&&) = delete;


        /// Must be called before using Draw__ methods
        void Begin(const char* name) const;

        J_NODISCARD ImGuiWindowFlags GetFlags() const noexcept {
            return flags_;
        }

        void SetFlags(const ImGuiWindowFlags flags) noexcept {
            flags_ = flags;
        }

        template <typename... TParam>
        void AppendFlags(TParam... flags) noexcept {
            static_assert(std::is_same_v<ImGuiWindowFlags_, std::common_type_t<TParam...>>);
            flags_ = flags_ | (flags | ...);
        }

    private:
        ImGuiWindowFlags flags_ = kDefaultFlags;
    };


    /// Emulates the ImGui title bar, but allows for drawing additional widgets other than text with the callback
    class GuiTitle
    {
        using CallbackT = std::function<void()>;

    public:
        /// \param callback Called after drawing title
        void Begin(
            const std::string& title, const CallbackT& callback = []() {}) const;

        float topPadding    = kGuiStyleFramePaddingY;
        float bottomPadding = kGuiStyleFramePaddingY;
    };


    class GuiItemSlots : public GuiComponentBase
    {
        using BeginCallbackT    = std::function<void(std::size_t slot_index)>;
        using DrawSlotCallbackT = std::function<void()>;

    public:
        explicit GuiItemSlots(const Context& context) : context_(&context) {}

        /// \param slot_count Number of slots to draw
        /// \param callback Use to draw slot
        void Begin(std::size_t slot_count, const BeginCallbackT& callback) const;

        /// \param tex_coord_id Tex coord id of the sprite to be drawn, if 0, a blank slot will be drawn
        /// \param item_count Number to display on the item, 0 to hide
        /// \param callback Called after drawing invisible button which will be clicked on
        void DrawSlot(
            SpriteTexCoordIndexT tex_coord_id, uint16_t item_count, const DrawSlotCallbackT& callback = [] {}) const;

        /// Draws slot without item count
        /// \param tex_coord_id Tex coord id of the sprite to be drawn, if 0, a blank slot will be drawn
        /// \param callback Called after drawing invisible button which will be clicked on
        void DrawSlot(
            SpriteTexCoordIndexT tex_coord_id, const DrawSlotCallbackT& callback = [] {}) const;

        /// \param callback Called after drawing invisible button which will be clicked on
        void DrawSlot(
            const game::ItemStack& item_stack, const DrawSlotCallbackT& callback = [] {}) const;


        /// Slots before wrapping onto new line
        uint8_t slotSpan = 10;
        uint8_t scale    = 1;

        /// If < 0 use default, >= 0 use amount provided of vertical space upon finishing
        float endingVerticalSpace = -1.f;

    private:
        /// Invisible button which detects clicks
        void DrawBackingButton() const;


        const Context* context_;
    };


    /// \remark Will pad description to match length of title if short to avoid cutting title off
    /// \param title Title of the tooltip
    /// \param draw_func Code to run while drawing the tooltip
    void DrawCursorTooltip(bool has_selected_item,
                           const std::string& title,
                           const std::string& description,
                           const std::function<void()>& draw_func);
} // namespace jactorio::gui

#endif // JACTORIO_INCLUDE_GUI_COMPONENTS_H
