// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_GUI_GUI_RENDERER_H
#define JACTORIO_INCLUDE_RENDER_GUI_GUI_RENDERER_H
#pragma once

#include <type_traits>

#include "jactorio.h"

#include "core/data_type.h"
#include "render/gui/component_base.h"

namespace jactorio
{
    namespace game
    {
        class PlayerData;
        class LogicData;
    } // namespace game

    namespace data
    {
        class FrameworkBase;
        struct UniqueDataBase;
        class PrototypeManager;
    } // namespace data
} // namespace jactorio


namespace jactorio::render
{
    struct MenuData;

    class GuiRenderer
    {
    public:
        GuiRenderer(GameWorlds& worlds,
                    game::LogicData& logic,
                    game::PlayerData& player,
                    const data::PrototypeManager& proto_manager,
                    MenuData& menu_data)
            : worlds(worlds), logic(logic), player(player), protoManager(proto_manager), menuData(menu_data) {}

        ///
        /// For OnRShowGui to init prototype and uniqueData
        /// \param g_rendr
        /// \param prototype
        /// \param unique_data
        GuiRenderer(GuiRenderer g_rendr, const data::FrameworkBase* prototype, data::UniqueDataBase* unique_data)
            : worlds(g_rendr.worlds),
              logic(g_rendr.logic),
              player(g_rendr.player),
              protoManager(g_rendr.protoManager),
              menuData(g_rendr.menuData),
              prototype(prototype),
              uniqueData(unique_data) {}

        template <typename T>
        J_NODISCARD T MakeComponent() const {
            static_assert(std::is_base_of_v<GuiComponentBase, T>);
            return T(this);
        }

        template <typename T>
        J_NODISCARD T MakeComponent() {
            static_assert(std::is_base_of_v<GuiComponentBase, T>);
            return T(this);
        }

        GameWorlds& worlds;
        game::LogicData& logic;
        game::PlayerData& player;

        const data::PrototypeManager& protoManager;

        MenuData& menuData;

        const data::FrameworkBase* prototype = nullptr;
        data::UniqueDataBase* uniqueData     = nullptr;
    };

} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_GUI_GUI_RENDERER_H
